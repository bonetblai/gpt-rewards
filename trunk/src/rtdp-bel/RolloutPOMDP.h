//  Theseus
//  RolloutPOMDP.h -- Reinforcement Learning for POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _RolloutPOMDP_INCLUDE_
#define _RolloutPOMDP_INCLUDE_

#include "POMDP.h"
#include "HistoryBelief.h"
#include <math.h>

class RolloutPOMDP : public POMDP {
  protected:
    HistoryAndSampleBelief initialBelief_;
    int width_;
    int nesting_;
    int num_particles_;

  public:
    RolloutPOMDP(const StandardModel *model = 0, int width = 1, int nesting = 1, int num_particles = 1)
      : POMDP(model),
        width_(width),
        nesting_(nesting),
        num_particles_(num_particles) {
    }
    virtual ~RolloutPOMDP() { }

    double QValue(const HistoryAndSampleBelief &belief, int state, int action, BeliefHash *hash) const {
        //const StandardModel *m = static_cast<const StandardModel*>(model_);
        double qvalue = DBL_MAX;
#if 0
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
#endif
        return qvalue;
    }
    void bestQValue(const HistoryAndSampleBelief &belief, int state, QResult &qresult, BeliefHash *hash) const {
#if 0
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
#endif
    }

    double simulate(int state, int nesting, HistoryAndSampleBelief *bel) const {
        double cost = 0;
        if( nesting == 1 ) {
            int nsteps = 0;
            for( int steps = 0; !model_->isAbsorbing(state) && (steps < PD.cutoff_); ++steps ) {
                int action = 2-state;//Random::uniform(model_->numActions_);
                int nstate = model_->sampleNextState(state, action);
#ifdef REWARDS
                while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);
                cost += model_->reward(state, action, nstate) * pow(model_->underlyingDiscount_, steps);
#else
                cost += model_->cost(state, action, nstate);
#endif
//std::cout << ", reward (act=" << action << ")=" << cost;
                state = nstate;
                ++nsteps;
            }
            if( nsteps == PD.cutoff_ ) std::cout << "#steps=" << nsteps << std::endl;
        } else {
            for( int steps = 0; !model_->isAbsorbing(state) && (steps < PD.cutoff_); ++steps ) {
                //std::cout << "state=" << state << ", bel=" << *bel << std::endl;
                assert(bel->contains(state));
                int action = Rollout(nesting-1, *bel);
                const HistoryAndSampleBelief &bel_a = static_cast<const HistoryAndSampleBelief&>(bel->update(model_, action));
                int nstate = bel_a.sampleState();
                cost += model_->reward(state, action, nstate);

                // sample obs, update bel w/ act + obs, and iterate
                int obs = model_->sampleNextObservation(nstate, action);
                const HistoryAndSampleBelief &bel_ao = static_cast<const HistoryAndSampleBelief&>(bel_a.update(model_, action, obs));
                *bel = bel_ao;
                state = nstate;
                if( !bel->contains(nstate) )
                    bel->insert_particle(nstate);
            }
        }
        return cost;
    }

    int Rollout(int nesting, const HistoryAndSampleBelief &bel) const {
#ifdef REWARDS
        double best_value = DBL_MIN;
#else
        double best_value = DBL_MAX;
#endif
        int best_action = 0;
        for( int action = 0; action < numActions_; ++action ) {
            double estimate = 0;
//std::cout << "begin estimate for act=" << action << std::endl;
            for( int w = 0; w < width_; ++w ) {
                int state = bel.sampleState();
                int nstate = model_->sampleNextState(state, action);
#ifdef REWARDS
                while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);
                double est = model_->reward(state, action, nstate);
#else
                while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);
                double est = model_->cost(state, action, nstate);
#endif
//std::cout << "  state=" << state << ", nstate=" << nstate << std::flush;
                if( nesting == 1 ) {
#ifdef REWARDS
                    est += model_->underlyingDiscount_ * simulate(nstate, 1, 0);
#else
                    est += simulate(nstate, 1, 0);
#endif
                } else {
                    // get obs and update beliefs
                    const HistoryAndSampleBelief &bel_a = static_cast<const HistoryAndSampleBelief&>(bel.update(model_, action));
                    int obs = model_->sampleNextObservation(nstate, action);
                    const HistoryAndSampleBelief &bel_ao = static_cast<const HistoryAndSampleBelief&>(bel_a.update(model_, action, obs));
                    HistoryAndSampleBelief *nbel = new HistoryAndSampleBelief(bel_ao);
                    if( !nbel->contains(nstate) )
                        nbel->insert_particle(nstate);
                    estimate += simulate(nstate, nesting, nbel);
                    delete nbel;
                }
                //std::cout << ", est=" << est << std::endl;
                estimate += est;
            }
            estimate /= width_;
//std::cout << "estimate=" << estimate << std::endl;

            // set best action
#ifdef REWARDS
            if( estimate > best_value ) {
                best_value = estimate;
                best_action = action;
            }
#else
            if( estimate < best_value ) {
                best_value = estimate;
                best_action = action;
            }
#endif
        }
        return best_action;
    }

#if 0
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
#endif

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
        //bestQValue(belief, *qresult_);
        //return qresult_->numTies_ > 0 ? qresult_->ties_[0] : -1;
        return 0;
    }
    virtual const Belief& getInitialBelief() const {
        return initialBelief_;
    }

    // serialization
    RolloutPOMDP* constructor() const {
        return new RolloutPOMDP;
    }
    virtual void write(std::ostream& os) const {
        POMDP::write(os);
    }
    static void read(std::istream& is, RolloutPOMDP &pomdp) {
        POMDP::read(is, pomdp);
    }
};

#endif // _RolloutPOMDP_INCLUDE

