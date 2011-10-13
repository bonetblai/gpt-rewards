//  StandardPOMDP.h -- Standard POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _StandardPOMDP_INCLUDE_
#define _StandardPOMDP_INCLUDE_

#include "Exception.h"
#include "POMDP.h"
#include "Problem.h"
#include "SB.h"
//#include "StandardBelief.h"
#include "QBelief.h"
#include "BeliefCache.h"
#include "Quantization.h"
#include "StandardModel.h"
#include "Serialization.h"
#include "Utils.h"

#include <iostream>
#include <float.h>
#include <map>
#include <list>

class Sondik;

class StandardPOMDP : public POMDP {
  protected:
    Quantization *quantization_;
    mutable QResult *qresult_;
    mutable double *nextobs_;
    mutable BeliefCache cache_;

  public:
    StandardPOMDP(const StandardModel *model = 0, double qlevels = 10, double qbase = 0.95)
      : POMDP(model),
        quantization_(new Quantization(qlevels, qbase, model ? model->numStates_ : 0)),
        qresult_(new QResult(numActions_)),
        nextobs_(new double[numObs_]) {
        beliefHash_ = new QBeliefHash;
        beliefHash_->setQuantization(quantization_);
    }
    virtual ~StandardPOMDP() {
        delete beliefHash_;
        delete[] nextobs_;
        delete qresult_;
        delete quantization_;
        cache_.deallocate(numActions_, numObs_);
    }

    double QValue(const Belief &belief, int action, BeliefCache::Entry *cache_entry, const BeliefHash *hash) const {
        double qvalue = DBL_MAX;
        if( applicable(belief,action) ) {
            qvalue = 0;
            const double *nextobs = 0;
            if( cache_entry ) nextobs = cache_entry->nextPossibleObservations(action);
            if( cache_entry && nextobs ) {
                for( int obs = 0; obs < numObs_; ++obs ) {
                    double prob = nextobs[obs];
                    if( prob > 0 ) {
                        const Belief *belief_ao = cache_entry->belief_ao(action, obs, numActions_);
                        assert(belief_ao != 0);
                        BeliefHash::Data data = const_cast<BeliefHash*>(hash)->lookup(*belief_ao, false, PD.hashAll_).second;
                        qvalue += prob * data.value_;
                    }
                }
            } else {
                if( !cache_entry ) cache_entry = cache_.insert(belief, numActions_, numObs_);
                const Belief &belief_a = belief.update(model_, action);
                bzero(nextobs_, numObs_ * sizeof(double));
                belief_a.nextPossibleObservations(model_, action, nextobs_);
                cache_entry->insertObservations(action,nextobs_, numActions_, numObs_);
                for( int obs = 0; obs < numObs_; ++obs ) {
                    double prob = nextobs_[obs];
                    if( prob > 0 ) {
                        const Belief &belief_ao = belief_a.update(model_, action, obs);
                        cache_entry->insertBelief(action, obs, belief_ao, numActions_, numObs_);
                        BeliefHash::Data data = const_cast<BeliefHash*>(hash)->lookup(belief_ao, false, PD.hashAll_).second;
                        qvalue += prob * data.value_;
                    }
                }
            }
            qvalue = cost(belief, action) + qvalue;
        }
        return qvalue;
    }
    void bestQValue(const Belief &belief, QResult &qresult, BeliefCache::Entry *cache_entry, const BeliefHash *hash) const {
        ++expansions_;
        qresult.numTies_ = 0;
        qresult.value_ = DBL_MAX;
        for( int action = 0; action < numActions_; ++action ) {
            double qvalue = QValue(belief, action, cache_entry, hash);
            if( (qresult.numTies_ == 0) || (qvalue <= qresult.value_) ) {
                if( qvalue < qresult.value_ ) qresult.numTies_ = 0;
                qresult.ties_[qresult.numTies_++] = action;
                qresult.value_ = qvalue;
            }
        }
    }

    bool checkSolved(BeliefHash::Entry current, std::list<BeliefHash::Entry> &closed);
    virtual void learnAlgorithm(Result& result);
    virtual void controlAlgorithm(Result& result, const Sondik *sondik) const;

    virtual void statistics(std::ostream &os) const {
        POMDP::statistics(os);
        cache_.statistics(os);
    }
    virtual double cost(const Belief &belief, int action) const {
        double sum = 0;
        const StandardBelief &bel = static_cast<const StandardBelief&>(belief);
        for( StandardBelief::const_iterator it = bel.begin(); it != bel.end(); ++it ) {
            sum += (*it).second * model_->cost((*it).first, action);
        }
        return sum;
    }
    virtual bool isAbsorbing(const Belief &belief) const {
        const StandardBelief &bel = static_cast<const StandardBelief&>(belief);
        for( StandardBelief::const_iterator it = bel.begin(); it != bel.end(); ++it ) {
            if( !model_->isAbsorbing((*it).first) ) return false;
        }
        return true;
    }
    virtual bool isGoal(const Belief &belief) const {
        const StandardBelief &bel = static_cast<const StandardBelief&>(belief);
        for( StandardBelief::const_iterator it = bel.begin(); it != bel.end(); ++it ) {
            if( !model_->isGoal((*it).first) ) return false;
        }
        return true;
    }
    virtual bool applicable(const Belief &belief, int action) const {
        const StandardBelief &bel = static_cast<const StandardBelief&>(belief);
        for( StandardBelief::const_iterator it = bel.begin(); it != bel.end(); ++it ) {
            if( !model_->applicable((*it).first,action) ) return false;
        }
        return true;
    }
    virtual double QValue(const Belief &belief, int action) const { return QValue(belief, action, 0, 0); }
    virtual void bestQValue(const Belief &belief, QResult &qresult) const { return bestQValue(belief, qresult, 0, 0); }
    virtual int getBestAction(const Belief &belief) const {
        bestQValue(belief, *qresult_);
        return qresult_->numTies_ > 0 ? qresult_->ties_[0] : -1;
    }
    virtual const Belief& getInitialBelief() const {
        return *model_->initialBelief_;
    }
};

#endif // _StandardPOMDP_INCLUDE

