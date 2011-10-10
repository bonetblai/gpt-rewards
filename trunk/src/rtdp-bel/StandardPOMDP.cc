//  Theseus
//  StandardPOMDP.cc -- Standard POMDP Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "Problem.h"
#include "Result.h"
#include "StandardPOMDP.h"
#include "HashHeuristic.h"
#include "Sondik.h"

#include <iostream>
#include <list>
#include <stack>
#include <math.h>

using namespace std;

extern unsigned glookups, gfound;

void StandardPOMDP::learnAlgorithm(Result& result) {
    assert(!PD.pddlProblem_ || ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType));
    stack<BeliefHash::Entry> stack; // for stopping rule

    // initialize result
    result.runType_ = 1;
    result.numSteps_ = 0;
    result.accCost_ = 0.0;
    result.accDisCost_ = 0.0;
    result.goalReached_ = false;
    result.startTimer();

    // set initial belief
    StandardBelief belief(model_->numStates());
    int state = model_->initialBelief_->sampleState();

    belief = static_cast<const StandardBelief&>(*model_->initialBelief_);
    BeliefCache::Entry *cache_entry = cache_.lookup(belief);
    if( !cache_entry ) cache_entry = cache_.insert(belief, numActions_, numObs_);

    pair<const Belief*, BeliefHash::Data> p = beliefHash_->lookup(belief, false, true);
    QBelief qbelief = *static_cast<const QBelief*>(p.first);
    BeliefHash::Data qdata = p.second;
    result.initialValue_ = qdata.value_;
    result.solved_ = qdata.solved_;
    beliefHash_->resetStats();

    // go for it!!!
    while( (PD.signal_ < 0) && (result.numSteps_ < cutoff_) ) {

        // verbosity output
        if( PD.verboseLevel_ >= 30 ) {
            *PD.outputFile_ << endl
	                    << "state=" << state << endl
	                    << "belief=" << belief << endl
		            << "qbelief=" << qbelief << endl
		            << "data=" << qdata << endl;
        }

        // check for trial termination
        if( model_->isGoal(state) || model_->isAbsorbing(state) ) {
            result.goalReached_ = true;
            break;
        }
   
        // compute the best QValues and update value
        bestQValue(belief, *qresult_, cache_entry, beliefHash_);
        beliefHash_->update(qbelief, qresult_->value_);
        if( PD.useStopRule_ ) {
            stack.push(beliefHash_->fetch(qbelief));
        }

        // greedy selection of best action
        int bestAction = -1;
        if( qresult_->numTies_ > 0 ) {
            int index = !randomTies_? 0 : ::unifRandomSampling(qresult_->numTies_);
            bestAction = qresult_->ties_[index];
        } else { // we have a dead-end
            beliefHash_->update(qbelief, DBL_MAX, true);
            result.push_back(state, -1, -1);
            break;
        }
        if( (epsilonGreedy() > 0) && (::realRandomSampling() < epsilonGreedy()) )
            bestAction = ::unifRandomSampling(model_->numActions());

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( model_->isAbsorbing(nstate) ) {
            nstate = model_->sampleNextState(state, bestAction);
        }
        int observation = model_->sampleNextObservation(nstate, bestAction);
        result.push_back(state, bestAction, observation);

        // update belief (using cache)
        const Belief *belief_ao = static_cast<const StandardBelief*>(cache_entry->belief_ao(bestAction, observation, numActions_));
        assert(belief_ao != 0);
        state = nstate;
        belief = *belief_ao;

        cache_entry = cache_.lookup(belief);
        if( !cache_entry ) cache_entry = cache_.insert(belief, numActions_, numObs_);
        p = beliefHash_->lookup(belief, false, true);
        qbelief = *static_cast<const QBelief*>(p.first);
        qdata = p.second;

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
        if( PD.useStopRule_ ) { // cleanup
            while( !stack.empty() ) stack.pop();
        }
        throw(SignalException(signal));
    }

    // stopping rule
    if( PD.useStopRule_ && (result.numSteps_ < cutoff_) ) {
        list<BeliefHash::Entry> closed;
        while( !stack.empty() ) {
            BeliefHash::Entry entry = stack.top();
            stack.pop();
            if( !entry.second->solved_ ) { // if not solved, attempt labeling
                closed.clear();
                if( checkSolved(entry, closed) ) {
                    for( list<BeliefHash::Entry>::iterator it = closed.begin(); it != closed.end(); ++it ) {
                        (*it).second->solved_ = true;
                        *PD.outputFile_ << "solved belief=" << *(*it).first << ", value=" << (*it).second->value_ << endl;
                    }
                } else {
                    break;
                }
            }
        }
    }
    while( !stack.empty() ) stack.pop();
    result.stopTimer();

    // set initial belief data
    p = beliefHash_->lookup(*model_->initialBelief_, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

void StandardPOMDP::controlAlgorithm(Result& result, const Sondik *sondik) const {
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
        hash->setQuantization(quantization_);
    }

    // set initial belief and state
    StandardBelief belief(model_->numStates());
    int state = model_->initialBelief_->sampleState();

    belief = static_cast<const StandardBelief&>(*model_->initialBelief_);
    BeliefCache::Entry *cache_entry = cache_.lookup(belief);
    if( !cache_entry ) cache_entry = cache_.insert(belief, numActions_, numObs_);

    pair<const Belief*, BeliefHash::Data> p = hash->lookup(belief, false, true);
    QBelief qbelief = *static_cast<const QBelief*>(p.first);
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

        int bestAction = -1;
        if( !sondik ) {
            bestQValue(belief, *qresult_, cache_entry,hash);
            if( PD.controlUpdates_ ) hash->update(qbelief, qresult_->value_);
            if( qresult_->numTies_ > 0 ) {
                int index = !randomTies_ ? 0 : ::unifRandomSampling(qresult_->numTies_);
                bestAction = qresult_->ties_[index];
            } else { // we have a dead-end
                if( PD.controlUpdates_ ) hash->update(qbelief, DBL_MAX, true);
                result.push_back(state, -1, -1);
                break;
            }
        } else {
            pair<double, int> p = sondik->value(belief);
            bestAction = p.second;
            if( bestAction == -1 ) {
                result.push_back(state, -1, -1);
                break;
            }
        }

        // sample state and observation
        int nstate = model_->sampleNextState(state, bestAction);
        while( model_->isAbsorbing(nstate) ) {
            nstate = model_->sampleNextState(state, bestAction);
        }
        int observation = model_->sampleNextObservation(nstate, bestAction);
        double realCost = model_->cost(state, bestAction, nstate);
        result.accCost_ += realCost;
        result.accDisCost_ += realCost * powf(model_->underlyingDiscount_, result.numSteps_);
        result.push_back(state, bestAction, observation);

        // update belief (using cache)
        if( !sondik ) {
            const Belief *belief_ao = static_cast<const StandardBelief*>(cache_entry->belief_ao(bestAction, observation, numActions_));
            if( belief_ao == 0 ) {
                assert(*cache_entry->belief_ == belief);
                assert(cache_entry->table_ != 0);
                int index = (1+observation) * numActions_ + bestAction;
                assert(cache_entry->table_[index] != 0);
                assert(valid_ptr(cache_entry->table_[index]));
            }
            assert(belief_ao != 0);
            belief = *belief_ao;
        } else {
            const Belief &belief_a = belief.update(model_, bestAction);
            const Belief &belief_ao = belief_a.update(model_, bestAction, observation);
            belief = belief_ao;
        }
        state = nstate;

        cache_entry = cache_.lookup(belief);
        if( !cache_entry ) cache_entry = cache_.insert(belief, numActions_, numObs_);
        p = hash->lookup(belief, false, true);
        qbelief = *static_cast<const QBelief*>(p.first);
    }

    // check for abortion
    if( PD.signal_ >= 0 ) {
        int signal = PD.signal_;
        PD.signal_ = -1;
        throw(SignalException(signal));
    }
    result.stopTimer();
    glookups += hash->nlookups() + beliefHash_->nlookups();
    gfound += hash->nfound() + beliefHash_->nfound();

    if( PD.controlUpdates_ ) {
        delete hash;
        delete h;
    }

    // set initial belief data
    p = beliefHash_->lookup(*model_->initialBelief_, false, true);
    result.initialValue_ = p.second.value_;
    result.solved_ = p.second.solved_;
}

bool StandardPOMDP::checkSolved(BeliefHash::Entry current, list<BeliefHash::Entry> &closed) {
    list<BeliefHash::Entry> open;
    set<BeliefHash::Entry> aux;

    // initialization
    closed.clear();
    if( !current.second->solved_ ) open.push_front(current);

    // dfs
    bool rv = true;
    while( !open.empty() ) {
        // get first from queue
        current = open.front();
        open.pop_front();
        closed.push_front(current);

        // check epsilon condition 
        bestQValue(*current.first, *qresult_);
        if( qresult_->numTies_ == 0 ) { // dead-end state
            current.second->value_ = DBL_MAX;
            current.second->solved_ = true;
            rv = false;
            continue;
        }
        if( fabs(qresult_->value_-current.second->value_) > PD.SREpsilon_ ) {
            rv = false;
            continue;
        }
        int action = qresult_->ties_[0];

        // unfold control
        const Belief &belief_a = current.first->update(model_, action);
        belief_a.nextPossibleObservations(model_, action, nextobs_);
        for( int obs = 0; obs < numObs_; ++obs ) {
            double prob = nextobs_[obs];
            if( prob > 0 ) { // compute and process belief_ao
                const Belief &belief_ao = belief_a.update(model_, action, obs);
                if( !isAbsorbing(belief_ao) ) {
                    const QBelief *qbelief = &(*quantization_)(static_cast<const StandardBelief&>(belief_ao));
                    BeliefHash::Entry entry = beliefHash_->fetch(*qbelief);
                    if( (entry.first != 0) && !entry.second->solved_ && (aux.find(entry) == aux.end()) ) {
                        open.push_front(entry);
                        aux.insert(entry);
                    }
                }
            }
        }
    }

    // process nodes in dfs postorder
    if( !rv ) {
        while( !closed.empty() ) {
            current = closed.front();
            closed.pop_front();
            bestQValue(*current.first, *qresult_);
            if( qresult_->numTies_ > 0 )
                current.second->value_ = qresult_->value_;
        }
    }
    return rv;
}

