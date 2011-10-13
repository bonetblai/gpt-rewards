//  RLPOMDP.h -- Reinforcement Learning for POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _RLPOMDP_INCLUDE_
#define _RLPOMDP_INCLUDE_

#include "POMDP.h"
#include "HistoryBelief.h"

class RLPOMDP : public POMDP {
  protected:
    HistoryBelief initialBelief_;
    mutable QResult *qresult_;
    mutable double *nextobs_;

  public:
    RLPOMDP(const StandardModel *model = 0)
      : POMDP(model),
        qresult_(new QResult(numActions_)),
        nextobs_(new double[numObs_]) {
        beliefHash_ = new HistoryBeliefHash; // TODO: fix this
    }
    virtual ~RLPOMDP() {
        delete beliefHash_;
        delete[] nextobs_;
        delete qresult_;
    }

    double QValue(const Belief &belief, int state, int action, BeliefHash *hash) const {
        const StandardModel *m = static_cast<const StandardModel*>(model_);
        double qvalue = DBL_MAX;
        state = Random::uniform(2);
        if( applicable(belief, action) ) {
            qvalue = 0;
            const Belief &belief_a = belief.update(model_, action);
            double cost = m->cost(state, action);
            int N = 1;
            for( int i = 0; i < N; ++i ) {
                int nstate = m->sampleNextState(state, action);
                //while( m->isAbsorbing(nstate) ) nstate = m->sampleNextState(state, action);
	        const double *dptr = m->observation_[nstate*m->numActions() + action];
                double qv = 0;
                for( int obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
                    double prob = *(dptr+obs);
                    if( prob > 0 ) {
                        const Belief &belief_ao = belief_a.update(model_, action, obs);
                        double value = hash->lookup(belief_ao, false, false).second.value_;
//std::cout << "  fetched value of " << belief_ao << " = " << data.value_ << std::endl;
                        qv += prob * value;
                    }
                }
                qvalue += qv;
            }
            qvalue = cost + qvalue / N;
            hash->update(belief_a, qvalue);
//std::cout << "update " << belief_a << " w/ " << qvalue << std::endl;
        }
        return qvalue;
    }
    void bestQValue(const Belief &belief, int state, QResult &qresult, BeliefHash *hash) const {
        ++expansions_;
        qresult.numTies_ = 0;
        qresult.value_ = DBL_MAX;
        for( int action = 0; action < numActions_; ++action ) {
            double qvalue = QValue(belief, state, action, hash);
            if( (qresult.numTies_ == 0) || (qvalue <= qresult.value_) ) {
                if( qvalue < qresult.value_ ) qresult.numTies_ = 0;
                qresult.ties_[qresult.numTies_++] = action;
                qresult.value_ = qvalue;
            }
        }
    }
    void bestQValue2(const Belief &belief, int state, QResult &qresult, BeliefHash *hash) const {
        qresult.numTies_ = 0;
        for( int action = 0; action < numActions_; ++action ) {
            const Belief &belief_a = belief.update(model_, action);
            double qvalue = hash->lookup(belief_a, false, false).second.value_;
std::cout << "  value for " << belief_a << " = " << qvalue << std::endl;
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
#if 0
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        double sum = 0;
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it )  {
            sum += model_->cost(*it, action);
        }
        return sum / bel.num_particles();
#endif
        return -1;
    }
    virtual bool isAbsorbing(const Belief &belief) const {
#if 0
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->isAbsorbing(*it) )
                return false;
        }
#endif
        return true;
    }
    virtual bool isGoal(const Belief &belief) const {
#if 0
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->isGoal(*it) )
                return false;
        }
#endif
        return true;
    }
    virtual bool applicable(const Belief &belief, int action) const {
#if 0
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        for( HistoryBelief::const_particle_iterator it = bel.particle_begin(); it != bel.particle_end(); ++it ) {
            if( !model_->applicable(*it, action) )
                return false;
        }
#endif
        return true;
    }
    virtual double QValue(const Belief &belief, int action) const {
        //return QValue(belief, action, 0);
        return 0;
    }
    virtual void bestQValue(const Belief &belief, QResult &qresult) const {
        //bestQValue(belief, qresult, 0);
    }
    virtual int getBestAction(const Belief &belief) const {
        bestQValue(belief, *qresult_);
        return qresult_->numTies_ > 0 ? qresult_->ties_[0] : -1;
    }
    virtual const Belief& getInitialBelief() const {
        return initialBelief_;
    }
};

#endif // _RLPOMDP_INCLUDE

