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

extern unsigned glookups, gfound;

void RolloutPOMDP::learnAlgorithm(Result& result) {

    // initialize result
    result.runType_ = 1;
    result.numSteps_ = 0;
    result.accReward_ = 0.0;
    result.accDiscountedReward_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();
    result.stopTimer();

    // set initial belief data
    pair<const Belief*, BeliefHash::Data> q = beliefHash_->lookup(initialBelief_, false, false);
    result.initialValue_ = q.second.value_;
    result.solved_ = q.second.solved_;
}

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
    HistoryAndSampleBelief belief;
    while( belief.num_particles() < num_particles_ ) {
        int state = model_->initialBelief_->sampleState();
        belief.insert_particle(state);
    }

    // go for it!!!
//cout << "begin trial" << endl;
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {

        assert(!belief.contains(2));
//cout << "  bel=" << belief << ", state=" << state << endl;
        if( !belief.contains(state) ) {
            cout << "  bel=" << belief << ", state=" << state << endl;
        }

        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }

        // compute best action performing simulated rollouts
        //cout << "  begin of rollout" << endl;
        int bestAction = Rollout(nesting_, belief);
        //cout << "  end of rollout" << endl;
//cout << "  best-act=" << bestAction << endl;
        if( ((result.numSteps_ % 2 == 0) && (bestAction != 0)) ||
            ((result.numSteps_ % 2 == 1) && (state != 2-bestAction)) ) {
#if 0
            cout << "  wrong action act=" << bestAction
                 << " in bel=" << belief
                 << " w/ state=" << state
                 << endl;
#endif
        }

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, bestAction);
        int observation = model_->sampleNextObservation(nstate, bestAction);
#if 0
        cout << "  nstate=" << nstate
             << ", obs=" << observation
             << endl;
#endif

        // get real reward
        double reward = model_->reward(state, bestAction, nstate);
        result.accReward_ += reward;
        result.accDiscountedReward_ += reward * pow(model_->underlyingDiscount_, result.numSteps_);

        // insert step (increases number of steps)
        result.push_back(state, bestAction, observation);

        // compute belief_ao
        const HistoryAndSampleBelief &belief_a = static_cast<const HistoryAndSampleBelief&>(belief.update(model_, bestAction));
        //cout << "  bel_a=" << belief_a << endl;
        const HistoryAndSampleBelief &belief_ao = static_cast<const HistoryAndSampleBelief&>(belief_a.update(model_, bestAction, observation));
        //cout << "  bel_ao=" << belief_ao << endl;

        // update state and belief
        state = nstate;
        belief = belief_ao;
    }
    //glookups += hash->nlookups() + beliefHash_->nlookups();
    //gfound += hash->nfound() + beliefHash_->nfound();

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();

    // set initial belief data
    //pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(initialBelief_, false, false);
    //result.initialValue_ = p.second.value_;
    //result.solved_ = p.second.solved_;
}

