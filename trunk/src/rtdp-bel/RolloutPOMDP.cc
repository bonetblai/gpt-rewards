//  RolloutPOMDP.cc -- Reinforcement Learning for POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#include "RolloutPOMDP.h"
#include "Problem.h"
#include "RtStandard.h"
#include "Result.h"
#include "Exception.h"
#include "HashHeuristic.h"

using namespace std;

//#define DEBUG

void RolloutPOMDP::controlAlgorithm(Result& result, const Sondik *) const {

    // initialize result
    result.runType_ = 0;
    result.numSteps_ = 0;
    result.accReward_ = 0.0;
    result.accDiscountedReward_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // set initial state and belief
    int state = model_->initialBelief_->sampleState();
    SampleBelief belief;
    while( belief.num_particles() < num_particles_ ) {
        int state = model_->initialBelief_->sampleState();
        belief.insert_particle(state);
    }

#ifdef DEBUG
    cout << "begin trial" << endl;
#endif

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {

#ifdef DEBUG
        cout << "  bel=" << belief << ", state=" << state << endl;
#endif

        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }

        // select action performing simulated rollouts
        int action = Rollout(nesting_, belief);

#ifdef DEBUG
        cout << "  action=" << action << endl;
        if( ((result.numSteps_ % 2 == 0) && (action != 0)) ||
            ((result.numSteps_ % 2 == 1) && (state != 2-action)) ) {
            cout << "  wrong action act=" << action
                 << " in bel=" << belief
                 << " w/ state=" << state
                 << endl;
        }
#endif

        // sample state and observation
        int nstate = model_->sampleNextState(state, action);
        while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, action);
        int obs = model_->sampleNextObservation(nstate, action);

        // get real reward
        double reward = model_->reward(state, action, nstate);
        double discounted_reward = reward * pow(model_->underlyingDiscount_, result.numSteps_);
        result.accReward_ += reward;
        result.accDiscountedReward_ += discounted_reward;

#ifdef DEBUG
        cout << "  nstate=" << nstate
             << ", obs=" << obs
             << ", reward=" << reward
             << ", discounted-reward=" << discounted_reward
             << endl;
#endif

        // insert step (increases number of steps)
        result.push_back(state, action, obs);

        // compute belief_ao
        const SampleBelief &belief_a = static_cast<const SampleBelief&>(belief.update(model_, action));
        const SampleBelief &belief_ao = static_cast<const SampleBelief&>(belief_a.update(model_, action, obs));

#ifdef DEBUG
        cout << "  bel_a=" << belief_a << endl;
        cout << "  bel_ao=" << belief_ao << endl;
#endif

        // update state and belief
        state = nstate;
        belief = belief_ao;
    }

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();
}

