//  RLPOMDP.cc -- Reinforcement Learning for POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#include "RLPOMDP.h"
#include "Problem.h"
#include "RtStandard.h"
#include "Result.h"
#include "Exception.h"
#include "HashHeuristic.h"

using namespace std;

extern unsigned glookups, gfound;

void RLPOMDP::learnAlgorithm(Result& result) {

    // initialize result
    result.runType_ = 1;
    result.numSteps_ = 0;
    result.accReward_ = 0.0;
    result.accDiscountedReward_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // set initial states
    int state = model_->initialBelief_->sampleState();

    // set initial belief
    HistoryBelief belief;
#if 0
    for( int i = 0; i < num_particles_; ++i ) {
        int particle = model_->initialBelief_->sampleState();
        belief.insert_particle(particle);
    }
#endif
    beliefHash_->resetStats();

#if 0
    pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, false);
    cout << "state=" << state
         << ", ibel=" << belief
         << ", stored value=" << p.second.value_
         << endl;
#endif

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {

        // verbosity output
        if( PD.verboseLevel_ >= 30 ) {
            pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, false);
            *PD.outputFile_ << endl
	                    << "state=" << state << endl
	                    << "belief=" << belief << endl
		            << "data=" << p.second << endl;
        }

        // compute best QValue
        bestQValue(belief, state, *qresult_, beliefHash_);
        beliefHash_->update(belief, qresult_->value_);

#if 0
        pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, false);
std::cout << "update " << belief
          << " w/ " << qresult_->value_
          << ". New stored value = " << p.second.value_
          << std::endl;
#endif

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_? 0 : Random::uniform(qresult_->numTies_);
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            beliefHash_->update(belief, DBL_MAX, true);
            result.push_back(-1, -1, -1);
            break;
        }
        if( (epsilonGreedy() > 0) && (Random::unit_interval() < epsilonGreedy()) )
            bestAction = Random::uniform(model_->numActions());

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        //while( model_->isAbsorbing(nstate) ) nstate = model_->sampleNextState(state, bestAction);
        int observation = model_->sampleNextObservation(nstate, bestAction);
        result.push_back(state, bestAction, observation);

#if 0
        cout << "state=" << state
             << ", act=" << bestAction
             << ", nstate=" << nstate
             << ", obs=" << observation
             << ", cost=" << model_->cost(state, bestAction, nstate)
             << endl;
#endif

        // terminate trial
        if( model_->isAbsorbing(nstate) ) {
            result.goalReached_ = true;
            break;
        }

        // compute belief_ao
        const HistoryBelief &belief_a = static_cast<const HistoryBelief&>(belief.update(model_, bestAction));
        const HistoryBelief &belief_ao = static_cast<const HistoryBelief&>(belief_a.update(model_, bestAction, observation));

        // update state and belief
        state = nstate;
        belief = belief_ao;

        // verbosity output
        if( PD.verboseLevel_ >= 30 ) {
            *PD.outputFile_ << "action=" << bestAction << ", obs=" << observation << endl;
        }
    }
    glookups += beliefHash_->nlookups();
    gfound += beliefHash_->nfound();

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();

    // set initial belief data
    pair<const Belief*, BeliefHash::Data> q = beliefHash_->lookup(initialBelief_, false, false);
    result.initialValue_ = q.second.value_;
    result.solved_ = q.second.solved_;
}

void RLPOMDP::controlAlgorithm(Result& result, const Sondik *) const {

    // initialize result
    result.runType_ = 0;
    result.numSteps_ = 0;
    result.accReward_ = 0.0;
    result.accDiscountedReward_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // setup hash for control
    BeliefHash *hash = beliefHash_;
    const HashHeuristic *h = 0;
    if( PD.controlUpdates_ ) {
        h = new HashHeuristic(beliefHash_);
        hash = new HistoryBeliefHash;
        hash->setHeuristic(h);
    }

    // set initial states
    int state = model_->initialBelief_->sampleState();

    // set initial belief
    HistoryBelief belief;
#if 0
    for( int i = 0; i < num_particles_; ++i ) {
        int particle = model_->initialBelief_->sampleState();
        belief.insert_particle(particle);
    }
#endif
    beliefHash_->resetStats();
    hash->resetStats();

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {

        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }

        // compute best action performing (only) action lookahead
        bestQValue2(belief, state, *qresult_, hash);

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_ ? 0 : Random::uniform(qresult_->numTies_);
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            result.push_back(state, -1, -1);
            break;
        }

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( model_->isAbsorbing(nstate) )
            nstate = model_->sampleNextState(state, bestAction);
        int observation = model_->sampleNextObservation(nstate, bestAction);
        result.push_back(state, bestAction, observation);

        // get real reward
        double reward = model_->reward(state, bestAction, nstate);
        result.accReward_ += reward;
        result.accDiscountedReward_ += reward * pow(model_->underlyingDiscount_, result.numSteps_);

        // compute belief_ao
        const HistoryBelief &belief_a = static_cast<const HistoryBelief&>(belief.update(model_, bestAction));
        const HistoryBelief &belief_ao = static_cast<const HistoryBelief&>(belief_a.update(model_, bestAction, observation));

        // update state and belief
        state = nstate;
        belief = belief_ao;
    }
    glookups += hash->nlookups() + beliefHash_->nlookups();
    gfound += hash->nfound() + beliefHash_->nfound();

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();

    if( PD.controlUpdates_ ) {
        delete hash;
        delete h;
    }

    // set initial belief data
    pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(initialBelief_, false, false);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

