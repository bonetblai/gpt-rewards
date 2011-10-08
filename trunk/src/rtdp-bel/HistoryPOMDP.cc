//  Theseus
//  HistoryPOMDP.cc -- History POMDP Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "HistoryPOMDP.h"
#include "Problem.h"
#include "RtStandard.h"
#include "Result.h"
#include "Exception.h"
#include "HashHeuristic.h"

#if 0
#include "Problem.h"
#include "Result.h"
#include "StandardPOMDP.h"
#include "Sondik.h"

#include <iostream>
#include <list>
#include <math.h>
#endif

using namespace std;

extern unsigned glookups, gfound;

void HistoryPOMDP::learnAlgorithm(Result& result) {
    assert(!PD.pddlProblem_ || ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType));

    // initialize result
    result.runType_ = 1;
    result.numSteps_ = 0;
    result.accCost_ = 0.0;
    result.accDisCost_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // set initial belief
    HistoryBelief belief;
    int absorbing = static_cast<const StandardModel*>(model_)->absorbing_;
    int state = model_->initialBelief_->sampleState();

    pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, true);
    BeliefHash::Data bel_data;
    result.initialValue_ = bel_data.value_;
    result.solved_ = bel_data.solved_;
    beliefHash_->resetStats();

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {
        if( PD.verboseLevel_ >= 30 ) {
            *PD.outputFile_ << endl
	                    << "state=" << state << endl
	                    << "belief=" << belief << endl
		            << "data=" << bel_data << endl;
        }

#if 0
        // check termination
        if( isAbsorbing(belief) || qdata.solved_ ) {
            if( !qdata.solved_ ) beliefHash_->update(qbelief, 0, true);
            result.goalReached_ = true;
            result.push_back(state, -1, -1);
            break;
        }
#endif

        //if( isGoal(belief) ) { result.goalReached_ = true; break; }
        if( model_->isGoal(state) ) {
            result.goalReached_ = true;
            break;
        }
   
        // compute the best QValues and update value
        bestQValue(belief, *qresult_, beliefHash_);
        beliefHash_->update(belief, qresult_->value_);

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_? 0 : lrand48() % qresult_->numTies_;
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            beliefHash_->update(belief, DBL_MAX, true);
            result.push_back(state, -1, -1);
            break;
        }
        if( (epsilonGreedy() > 0) && (drand48() < epsilonGreedy()) )
            bestAction = lrand48() % model_->numActions();

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( nstate == absorbing ) {
            nstate = model_->sampleNextState(state, bestAction);
        }
        int observation = model_->sampleNextObservation(nstate, bestAction);
        result.push_back(state, bestAction, observation);

        // update belief
        const Belief &belief_ao = belief.update(model_, bestAction, observation);
        state = nstate;
        belief = belief_ao;
        p = beliefHash_->lookup(belief, false, true);
        bel_data = p.second;

        if( PD.verboseLevel_ >= 30 ) { // print info
            *PD.outputFile_ << "action=" << bestAction << ", obs=" << observation << endl;
        }
    }
    glookups += beliefHash_->nlookups();
    gfound += beliefHash_->nfound();

    // check for abortion
    if( PD.signal_ >= 0 ) { // cleanup
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();

    // set initial belief data
    HistoryBelief initial_belief;
    p = beliefHash_->lookup(initial_belief, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

void HistoryPOMDP::controlAlgorithm(Result& result, const Sondik *sondik) const {
    assert(!PD.pddlProblem_ || ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType));

    // initialize result
    result.runType_ = 0;
    result.numSteps_ = 0;
    result.accCost_ = 0.0;
    result.accDisCost_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // setup hash for control
    BeliefHash *hash = beliefHash_;
    const HashHeuristic *h = 0;
    if( PD.controlUpdates_ ) {
        h = new HashHeuristic(beliefHash_);
        hash = new QBeliefHash;
        hash->setHeuristic(h);
    }

    // set initial belief and state
    HistoryBelief belief;
    int absorbing = static_cast<const StandardModel*>(model_)->absorbing_;
    int state = model_->initialBelief_->sampleState();

    pair<const Belief*, BeliefHash::Data> p = hash->lookup(belief, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
    beliefHash_->resetStats();
    hash->resetStats();

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {
        if( model_->isGoal(state) ) {
            result.goalReached_ = true;
            break;
        }

        // compute the best QValues and update value
        bestQValue(belief, *qresult_, hash);
        if( PD.controlUpdates_ ) hash->update(belief, qresult_->value_);

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_ ? 0 : lrand48() % qresult_->numTies_;
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            if( PD.controlUpdates_ ) hash->update(belief, DBL_MAX, true);
            result.push_back(state, -1, -1);
            break;
        }

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( nstate == absorbing ) {
            nstate = model_->sampleNextState(state, bestAction);
        }
        int observation = model_->sampleNextObservation(nstate, bestAction);
        double realCost = model_->cost(state, bestAction, nstate);
        result.accCost_ += realCost;
        result.accDisCost_ += realCost * powf(model_->underlyingDiscount_, result.numSteps_);
        result.push_back(state, bestAction, observation);

        // update belief
        const Belief &belief_ao = belief.update(model_, bestAction, observation);
        state = nstate;
        belief = belief_ao;
        p = hash->lookup(belief, false, true);
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
    HistoryBelief initial_belief;
    p = beliefHash_->lookup(initial_belief, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

