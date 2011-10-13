//  RolloutPOMDP.h -- Reinforcement Learning for POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _RolloutPOMDP_INCLUDE_
#define _RolloutPOMDP_INCLUDE_

#include "POMDP.h"
#include "SampleBelief.h"
#include <math.h>

//#define DEBUG
//#define REWARDS

class RolloutPOMDP : public POMDP {
  protected:
    SampleBelief initialBelief_;
    int depth_;
    int width_;
    int nesting_;
    int num_particles_;

  public:
    RolloutPOMDP(const StandardModel *model = 0, int depth = 20, int width = 1, int nesting = 1, int num_particles = 1)
      : POMDP(model),
        depth_(depth),
        width_(width),
        nesting_(nesting),
        num_particles_(num_particles) {
    }
    virtual ~RolloutPOMDP() { }

    double QValue(const SampleBelief &belief, int state, int action, BeliefHash *hash) const {
#if 0
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
#endif
        return 0;
    }
    void bestQValue(const SampleBelief &belief, int state, QResult &qresult, BeliefHash *hash) const {
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

    double simulate(int state, int nesting, SampleBelief *bel) const {
        double cost = 0;
        for( int steps = 0; !model_->isAbsorbing(state) && (steps < depth_); ++steps ) {
            if( nesting == 1 ) {
                int action = heuristic_ == 0 ? Random::uniform(model_->numActions_) : heuristic_->action(state);
                int nstate = model_->sampleNextState(state, action);
#ifdef REWARDS
                while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);
                cost += model_->reward(state, action, nstate) * pow(model_->underlyingDiscount_, steps);
#else
                cost += model_->cost(state, action, nstate);
#endif
                state = nstate;
            } else {
                assert(bel->contains(state));
                int action = Rollout(nesting-1, *bel);
                const SampleBelief &bel_a = static_cast<const SampleBelief&>(bel->update(model_, action));
                int nstate = bel_a.sampleState();
                assert(!model_->isAbsorbing(nstate));
#ifdef REWARDS
                cost += model_->reward(state, action, nstate) * pow(model_->underlyingDiscount_, steps);
#else
                cost += model_->cost(state, action, nstate);
#endif

                // sample obs, update bel w/ act + obs, and iterate
                int obs = model_->sampleNextObservation(nstate, action);
                const SampleBelief &bel_ao = static_cast<const SampleBelief&>(bel_a.update(model_, action, obs));
                *bel = bel_ao;
                if( !bel->contains(nstate) ) bel->insert_particle(nstate);
                state = nstate;
            }
        }
        return cost;
    }

    int Rollout(int nesting, const SampleBelief &bel) const {
#ifdef REWARDS
        double best_value = DBL_MIN;
#else
        double best_value = DBL_MAX;
#endif
        int best_action = 0;
        for( int action = 0; action < numActions_; ++action ) {

#ifdef DEBUG
            std::cout << std::setw(2 * (1 + nesting_ - nesting)) << ""
                      << "begin estimate for act=" << action << std::endl;
#endif

            double estimate = 0;
            for( int w = 0; w < width_; ++w ) {

                int state = bel.sampleState();
                int nstate = model_->sampleNextState(state, action);
                while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);

                double simulation = 0;
                if( nesting == 1 ) {
                    simulation = simulate(nstate, 1, 0);
                } else {
                    const SampleBelief &bel_a = static_cast<const SampleBelief&>(bel.update(model_, action));
                    SampleBelief *nbel = new SampleBelief(bel_a);
                    if( !nbel->contains(nstate) ) nbel->insert_particle(nstate);
                    int obs = model_->sampleNextObservation(nstate, action);
                    const SampleBelief &bel_ao = static_cast<const SampleBelief&>(nbel->update(model_, action, obs));
                    *nbel = bel_ao;
                    if( !nbel->contains(nstate) ) nbel->insert_particle(nstate);
                    simulation = simulate(nstate, nesting, nbel);
                    delete nbel;
                }

#ifdef REWARDS
                simulation *= model_->underlyingDiscount_;
                simulation += model_->reward(state, action, nstate);
#else
                simulation += model_->cost(state, action, nstate);
#endif

                estimate += simulation;

#ifdef DEBUG
                std::cout << std::setw(2 * (2 + nesting_ - nesting)) << ""
                          << "state=" << state
                          << ", nstate=" << nstate 
                          << ", simulation=" << simulation
                          << std::endl;
#endif
            }
            estimate /= width_;

#ifdef DEBUG
            std::cout << std::setw(2 * (1 + nesting_ - nesting)) << ""
                      << "estimate=" << estimate << std::endl;
#endif

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

#ifdef DEBUG
        std::cout << std::setw(2 * (1 + nesting_ - nesting)) << ""
                  << "best-action=" << best_action << std::endl;
#endif

        return best_action;
    }

    virtual void learnAlgorithm(Result& result) { }
    virtual void controlAlgorithm(Result& result, const Sondik *sondik) const;

    virtual void statistics(std::ostream &os) const {
        POMDP::statistics(os);
    }
    virtual double cost(const Belief &belief, int action) const { return 0; }
    virtual bool isAbsorbing(const Belief &belief) const { return false; }
    virtual bool isGoal(const Belief &belief) const { return false; }
    virtual bool applicable(const Belief &belief, int action) const { return true; }
    virtual double QValue(const Belief &belief, int action) const { return 0; }
    virtual void bestQValue(const Belief &belief, QResult &qresult) const { }
    virtual int getBestAction(const Belief &belief) const { return 0; }
    virtual const Belief& getInitialBelief() const { return initialBelief_; }
};

#undef DEBUG

#endif // _RolloutPOMDP_INCLUDE

