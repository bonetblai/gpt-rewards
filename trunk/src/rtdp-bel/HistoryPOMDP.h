//  Theseus
//  HistoryPOMDP.h -- History POMDP Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _HistoryPOMDP_INCLUDE_
#define _HistoryPOMDP_INCLUDE_

#include "POMDP.h"
#include "HistoryBelief.h"

#if 0
#include "Exception.h"
#include "Problem.h"
#include "SB.h"
//#include "StandardBelief.h"
#include "QBelief.h"
#include "BeliefCache.h"
#include "Quantization.h"
#include "Serialization.h"
#include "Utils.h"

#include <iostream>
#include <float.h>
#include <map>
#include <list>
#endif

class HistoryPOMDP : public POMDP {
  protected:
    int num_particles_;
    HistoryBelief initialBelief_;
    mutable QResult *qresult_;
    mutable double *nextobs_;

  public:
    HistoryPOMDP(const StandardModel *model = 0, int num_particles = 0)
      : POMDP(model),
        num_particles_(num_particles),
        qresult_(new QResult(numActions_)),
        nextobs_(new double[numObs_]) {
        beliefHash_ = new HistoryBeliefHash; // TODO: fix this
    }
    virtual ~HistoryPOMDP() {
        delete beliefHash_;
        delete[] nextobs_;
        delete qresult_;
    }

    double QValue(const Belief &belief, int action, const BeliefHash *hash) const {
//std::cout << "entering qvalue" << std::endl;
        double qvalue = DBL_MAX;
        if( applicable(belief, action) ) {
//std::cout << "  applicable action = " << action << std::endl;
            qvalue = 0;
            const Belief &belief_a = belief.update(model_, action);
//std::cout << "  got bel_a = " << belief_a << std::endl;
            bzero(nextobs_, numObs_ * sizeof(double));
            belief_a.nextPossibleObservations(model_, action, nextobs_);
            for( int obs = 0; obs < numObs_; ++obs ) {
                double prob = nextobs_[obs];
//std::cout << "  prob[obs=" << obs << "] = " << prob << std::endl;
                if( prob > 0 ) {
                    const Belief &belief_ao = belief_a.update(model_, action, obs);
//std::cout << "  got bel_ao = " << belief_ao << std::endl;
                    BeliefHash::Data data = const_cast<BeliefHash*>(hash)->lookup(belief_ao, false, PD.hashAll_).second;
//std::cout << "    value of " << belief_ao << " = " << data.value_ << std::endl;
                    qvalue += prob * data.value_;
                }
            }
            qvalue += cost(belief, action);
        }
//std::cout << "  qvalue = " << qvalue << std::endl;
//std::cout << "exiting qvalue" << std::endl;
        return qvalue;
    }
    void bestQValue(const Belief &belief, QResult &qresult, const BeliefHash *hash) const {
        ++expansions_;
        qresult.numTies_ = 0;
        for( int action = 0; action < numActions_; ++action ) {
            double qvalue = QValue(belief, action, hash);
            if( (qresult.numTies_ == 0) || (qvalue <= qresult.value_) ) {
                if( qvalue < qresult.value_ ) qresult.numTies_ = 0;
                qresult.ties_[qresult.numTies_++] = action;
                qresult.value_ = qvalue;
            }
        }
    }

    virtual void learnAlgorithm(Result& result);
    virtual void controlAlgorithm(Result& result, const Sondik *sondik) const;

    virtual void statistics(std::ostream &os) const {
        POMDP::statistics(os);
        //cache_.statistics(os);
    }
    virtual double cost(const Belief &belief, int action) const {
        double sum = 0;
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it )  {
            sum += model_->cost(*it, action);
//std::cout << "    cost of (act=" << action << ",state=" << *it << ") = " << model_->cost(*it, action) << std::endl;
        }
//std::cout << "cost of (act=" << action << ") = " << sum/bel.num_particles() << std::endl;
        return sum / bel.num_particles();
    }
    virtual bool isAbsorbing(const Belief &belief) const {
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->isAbsorbing(*it) )
                return false;
        }
        return true;
    }
    virtual bool isGoal(const Belief &belief) const {
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->isGoal(*it) )
                return false;
        }
        return true;
    }
    virtual bool applicable(const Belief &belief, int action) const {
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->applicable(*it, action) )
                return false;
        }
        return true;
    }
    virtual double QValue(const Belief &belief, int action) const {
        return QValue(belief, action, 0);
    }
    virtual void bestQValue(const Belief &belief, QResult &qresult) const {
        return bestQValue(belief, qresult, 0);
    }
    virtual int getBestAction(const Belief &belief) const {
        bestQValue(belief, *qresult_);
        return qresult_->numTies_ > 0 ? qresult_->ties_[0] : -1;
    }
    virtual const Belief& getInitialBelief() const {
        return initialBelief_;
    }

    // serialization
    HistoryPOMDP* constructor() const {
        return new HistoryPOMDP;
    }
    virtual void write(std::ostream& os) const {
        POMDP::write(os);
    }
    static void read(std::istream& is, HistoryPOMDP &pomdp) {
        POMDP::read(is, pomdp);
    }
};

#endif // _HistoryPOMDP_INCLUDE

