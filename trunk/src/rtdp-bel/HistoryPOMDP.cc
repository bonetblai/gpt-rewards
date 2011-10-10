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

    // set initial states
    //int state = model_->initialBelief_->sampleState();

    // set initial belief and insert particles
    HistoryBelief belief;
    for( int i = 0; i < num_particles_; ++i ) {
        int particle = model_->initialBelief_->sampleState();
        belief.insert_particle(particle);
    }

    pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, true);
    BeliefHash::Data bel_data;
    result.initialValue_ = bel_data.value_;
    result.solved_ = bel_data.solved_;
    beliefHash_->resetStats();

    // go for it!!!
cout << "ibel=" << belief << endl;
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {
//cout << "Z1: bel=" << belief << ", state=" << state << endl;

        // verbosity output
        if( PD.verboseLevel_ >= 30 ) {
            *PD.outputFile_ << endl
	                    //<< "state=" << state << endl
	                    << "belief=" << belief << endl
		            << "data=" << bel_data << endl;
        }

#if 0
        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }
#endif
   
        // compute the best QValues and update value
//cout << "before bqv" << endl;
        bestQValue(belief, *qresult_, beliefHash_);
cout << "bqv = " << qresult_->value_ << endl;
        beliefHash_->update(belief, qresult_->value_);
//cout << "after bqv" << endl;

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_? 0 : ::unifRandomSampling(qresult_->numTies_);
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            beliefHash_->update(belief, DBL_MAX, true);
            //result.push_back(state, -1, -1);
            break;
        }
        if( (epsilonGreedy() > 0) && (::realRandomSampling() < epsilonGreedy()) )
            bestAction = ::unifRandomSampling(model_->numActions());
//cout << "Z3: a=" << bestAction << endl;

        // compute belief_a
        const HistoryBelief &belief_a = static_cast<const HistoryBelief&>(belief.update(model_, bestAction));
//cout << "Z4: bel_a=" << belief_a << endl;

        // sample state and observation
        int nstate = belief_a.sampleState();
        //int nstate = model_->sampleNextState(state, bestAction);
        int observation = model_->sampleNextObservation(nstate, bestAction);
        //result.push_back(state, bestAction, observation);
        result.push_back(-1, bestAction, observation);
//cout << "Z?: nstate=" << nstate << ", obs=" << observation << endl;

        // terminate trial
        if( model_->isAbsorbing(nstate) ) {
//cout << "Z?: termination" << belief_a << endl;
            result.goalReached_ = true;
            break;
        }

        // compute belief_ao
        const HistoryBelief &belief_ao = static_cast<const HistoryBelief&>(belief_a.update(model_, bestAction, observation));
        //const Belief &belief_ao = belief_a.update(model_, bestAction, observation);
//cout << "Z5: bel_ao=" << belief_ao << endl;

        if( belief_ao.num_particles() == 0 ) break;

        // update state and beleif
        //state = nstate;
        belief = belief_ao;
        p = beliefHash_->lookup(belief, false, true);
        bel_data = p.second;

        // verbosity output
        if( PD.verboseLevel_ >= 30 ) {
            *PD.outputFile_ << "action=" << bestAction << ", obs=" << observation << endl;
        }
    }
    glookups += beliefHash_->nlookups();
    gfound += beliefHash_->nfound();
//cout << "num-steps=" << result.numSteps_ << endl;

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }

    // stop timer
    result.stopTimer();

    // set initial belief data
    p = beliefHash_->lookup(initialBelief_, false, true);
    result.initialValue_ = p.second.value_;
cout << "value for ibel = " << p.second.value_ << endl;
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
        hash = new HistoryBeliefHash;
        hash->setHeuristic(h);
    }

    // set initial states
    int state = model_->initialBelief_->sampleState();

    // set initial belief and insert particles
    HistoryBelief belief;
    for( int i = 0; i < num_particles_; ++i ) {
        int particle = model_->initialBelief_->sampleState();
        belief.insert_particle(particle);
    }

    pair<const Belief*, BeliefHash::Data> p = hash->lookup(belief, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
    beliefHash_->resetStats();
    hash->resetStats();

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {
        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }

        // compute the best QValues and update value
        bestQValue(belief, *qresult_, hash);
        if( PD.controlUpdates_ ) hash->update(belief, qresult_->value_);

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_ ? 0 : ::unifRandomSampling(qresult_->numTies_);
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            if( PD.controlUpdates_ ) hash->update(belief, DBL_MAX, true);
            result.push_back(state, -1, -1);
            break;
        }

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        //while( model_->isAbsorbing(nstate) ) {
        //    nstate = model_->sampleNextState(state, bestAction);
        //}
        int observation = model_->sampleNextObservation(nstate, bestAction);
        result.push_back(state, bestAction, observation);
        double realCost = model_->cost(state, bestAction, nstate);
        result.accCost_ += realCost;
        result.accDisCost_ += realCost * powf(model_->underlyingDiscount_, result.numSteps_);

        // update belief
        const Belief &belief_a = belief.update(model_, bestAction);
        const Belief &belief_ao = belief_a.update(model_, bestAction, observation);
        state = nstate;
        belief = belief_ao;
        p = hash->lookup(belief, false, true);
    }
    glookups += hash->nlookups() + beliefHash_->nlookups();
    gfound += hash->nfound() + beliefHash_->nfound();
cout << "num-steps=" << result.numSteps_ << endl;

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
    p = beliefHash_->lookup(initialBelief_, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

