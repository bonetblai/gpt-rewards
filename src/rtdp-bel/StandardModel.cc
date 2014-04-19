//  StandardModel.cc -- Standard POMDP models and translation from
//                      reward-based discounted POMDPs into Goal POMDPs.
//                      Parsing of POMDPs given in Cassandra's format.
//
//  Blai Bonet, Hector Geffner (c)

#include "StandardModel.h"
#include "SB.h"

#include <stdio.h>
#include <mdp/sparse-matrix.h>

using namespace std;

extern "C" {
    extern int gNumGoals;
    extern int gGoalList[];
}

StandardModel::StandardModel(const char *cassandraFilename)
  : Model(), goal_(0), goalSize_(0), numGoals_(0), application_(0),
    cost_(0), cassandra_(true), transition_(0), observation_(0) {

    cout << "parsing file '" << cassandraFilename << "' ... " << flush;
    if( !readMDP((char*)cassandraFilename) ) { // parse file with Cassandra's MDP parser
        cerr << "Fatal Error: when reading file: " << cassandraFilename << endl;
        exit(-1);
    }
    cout << "done" << endl;

    // fundamental vars
    numActions_ = gNumActions;
    numStates_ = gNumStates;
    numObs_ = gNumObservations;
    underlyingDiscount_ = gDiscount;
    isRewardBased_ = gValueType == 0;
    if( isRewardBased_ && (underlyingDiscount_ == 1) ) {
        cerr << "Fatal Error: cannot handle a reward-based model with a discount factor equal to 1" << endl;
        exit(-1);
    }
    cout << "problem is instance of a " << (isRewardBased_ ? "reward" : "cost") << "-based model" << endl;

    // allocating space for model
    application_ = new unsigned[(numActions_ * (1+numStates_) + 32) / 32];
    cost_ = new double[numActions_ * (1+numStates_)];
    bzero(cost_, numActions_ * (1+numStates_) * sizeof(double));
    transition_ = new vector<pair<int, double> >*[numActions_ * (1+numStates_)];
    bzero(transition_, numActions_ * (1+numStates_) * sizeof(void*));
    observation_ = new double*[numActions_ * (1+numStates_)];
    for( int action = 0; action < numActions_; ++action ) {
        for( int nstate = 0; nstate <= numStates_; ++nstate ) {
            observation_[nstate*numActions_ + action] = new double[1 + numObs_];
            bzero(observation_[nstate*numActions_ + action], (1+numObs_) * sizeof(double));
        }
    }

    // compute rewards and maxReward
    cout << "extracting and translating costs ... " << flush;
    for( int state = 0; state < numStates_; ++state ) {
        for( int action = 0; action < numActions_; ++action ) {
            double r = getEntryMatrix(Q, action, state);
            if( isRewardBased_ )
                maxReward_ = maxReward_ > r ? maxReward_ : r;
            else
                minCost_ = minCost_ > r ? r : minCost_;
            cost_[state*numActions_ + action] = r;
        }
    }
    if( isRewardBased_ ) {
        for( int state = 0; state < numStates_; ++state ) {
            for( int action = 0; action < numActions_; ++action ) {
                double c = 1.0 + maxReward_ - cost_[state*numActions_ + action];
                cost_[state*numActions_ + action] = c;
                minCost_ = minCost_ > c ? c : minCost_;
            }
        }
    }
    cout << "done" << endl;
    //destroyMatrix(Q);

    // compute model
    cout << "computing model for action:" << flush;
    for( int action = 0; action < numActions_; ++action ) {
        cout << " " << action << flush; 

        // transitions
        assert(P[action]->num_rows == numStates_);
        for( int state = 0; state < P[action]->num_rows; ++state ) {
            unsigned index = state*numActions_ + action;
            for( int j = P[action]->row_start[state], jsz = P[action]->row_start[state] + P[action]->row_length[state]; j < jsz; j++ ) {
                int nstate = P[action]->col[j];
                double p = P[action]->mat_val[j];
                assert(p > 0.0);
                p *= underlyingDiscount_;
                if( transition_[index] == 0 ) transition_[index] = new vector<pair<int, double> >;
                transition_[index]->push_back(make_pair(nstate, p));
            }
        }

        // observations
        assert(R[action]->num_rows == numStates_);
        for( int nstate = 0; nstate < R[action]->num_rows; ++nstate ) {
            unsigned index = nstate*numActions_ + action;
            for( int j = R[action]->row_start[nstate], jsz = R[action]->row_start[nstate] + R[action]->row_length[nstate]; j < jsz; j++ ) {
                int obs = R[action]->col[j];
                double p = R[action]->mat_val[j];
                observation_[index][obs] = p;
            }
        }

        // application model
        for( int state = 0; state < P[action]->num_rows; ++state) {
            unsigned index = state*numActions_ + action;
            unsigned j = index >> 5, off = index & 0x1F, app = application_[j];
            application_[j] = app | (1<<off);
        }
    }
    cout << " done" << endl;

#if 0
    for( int state = 0; state < numStates_; ++state ) {
        for( int action = 0; action < numActions_; ++action ) {
            unsigned index = state*numActions_ + action;

            // transition model: P( s' | s, a )
            for( int nstate = 0; nstate < numStates_; ++nstate ) {
                double p = getEntryMatrix(P[action], state, nstate);
                if( p > 0.0 ) {
                    p *= underlyingDiscount_;
                    if( transition_[index] == 0 )
                        transition_[index] = new vector<pair<int, double> >;
                    transition_[index]->push_back(make_pair(nstate, p));
                }
            }

            // observation model: P( o | s', a ) (in this computation s stands for s')
            for( int obs = 0; obs < numObs_; ++obs ) {
                double p = getEntryMatrix(R[action], state, obs);
                if( p > 0.0 ) observation_[index][obs] = p;
            }

            // application model
            unsigned j = index >> 5, off = index & 0x1F, app = application_[j];
            application_[j] = app | (1<<off);
        }
    }
#endif

#if 0
    for( int action = 0; action < numActions_; ++action ) {
        destroyMatrix(P[action]);
        destroyMatrix(R[action]);
    }
    free(P);
    free(R);
#endif

    // set model for absorbing state
    if( isRewardBased_ || (underlyingDiscount_ < 1) ) {
        assert(underlyingDiscount_ < 1);
        absorbing_ = numStates_;
        for( int action = 0; action < numActions_; ++action ) {
            unsigned aindex = absorbing_*numActions_ + action;
            if( transition_[aindex] == 0 ) transition_[aindex] = new vector<pair<int, double> >;
            transition_[aindex]->push_back(make_pair(absorbing_, 1.0));
            for( int state = 0; state < numStates_; ++state ) {
                unsigned index = state*numActions_ + action;
                if( transition_[index] == 0 ) transition_[index] = new vector<pair<int, double> >;
                transition_[index]->push_back(make_pair(absorbing_, 1 - underlyingDiscount_));
            }
            observation_[aindex][numObs_] = 1.0;
            unsigned j = aindex >> 5, off = aindex & 0x1F, app = application_[j];
            application_[j] = app | (1<<off);
        }
        ++numStates_;
        ++numObs_;
    }

    // set the initial belief
    StandardBelief *belief = new StandardBelief;
    for( int state = numStates_ - 1; state >= 0; --state ) {
        if( (state != absorbing_) && (gInitialBelief[state] > 0.0) ) {
            belief->push_back(state, (double)gInitialBelief[state]);
        }
    }
    belief->normalize();
    initialBelief_ = belief;
    free(gInitialBelief);

    // set goal states (extension to Cassandra's format)
    for( int i = 0; i < gNumGoals; ++i ) {
        int goal = gGoalList[i];
        setGoal(goal);
    }
}

void StandardModel::outputCASSANDRA(ostream &os) const {
    if( cassandra_ ) {
        if( !isRewardBased_ ) {
            cerr << "Fatal Error: generation of Cassandra representation currently only supportted for reward-based models" << endl;
            exit(-1);
        }
        extern int writeTransPOMDP(ostream&, double);
        writeTransPOMDP(os, -1.0 - maxReward_);
        os << "# reward-shift = " << -1.0 - maxReward_ << endl;
    } else {
        os.setf(ios::fixed);
        os << "# StandardPOMDP Model -- Automatically Generated Model" << endl
           << endl
           << "discount: 1.0" << endl
           << "values: reward" << endl
           << "states: " << numStates_ << endl
           << "actions: " << numActions_ << endl
           << "observations: " << numObs_ << endl;

        if( numGoals_ > 0 ) {
            os << "goals:";
            for( int s = 0; s < numStates_; ++s ) {
                if( isGoal(s) ) os << " " << s;
            }
            os << endl;
        }
        os << endl;

        const StandardBelief *ibel = static_cast<const StandardBelief*>(initialBelief_);
        map<int,double> imap;
        for( StandardBelief::const_iterator it = ibel->begin(); it != ibel->end(); ++it ) {
            imap.insert(make_pair((*it).first, (*it).second));
        }
        os << "start:" << endl;
        int last = 0;
        for( map<int,double>::const_iterator it = imap.begin(); it != imap.end(); ++it ) {
            int state = it->first;
            double prob = it->second;
            for( int s = last; s < state; ++s ) os << "0.0 ";
            os << prob << " ";
            last = 1 + state;
        }
        for( int s = last; s < numStates_; ++s ) os << " 0.0";
        os << endl << endl;

        os << "# Transitions" << endl;
        for( int action = 0; action < numActions_; ++action ) {
            for( int state = 0; state < numStates_; ++state ) {
                unsigned index = state*numActions_ + action;
                if( transition_[index] ) {
                    for( vector<pair<int, double> >::const_iterator it = transition_[index]->begin(); it != transition_[index]->end(); ++it ) {
                        int nstate = it->first;
                        double prob = it->second;
                        os << "T: " << action
                           << " : " << state
                           << " : " << nstate
                           << " " << prob
                           << endl;
                    }
                }
            }
        }
        os << endl;

        os << "# Observations" << endl;
        for( int action = 0; action < numActions_; ++action ) {
            for( int nstate = 0; nstate < numStates_; ++nstate ) {
                unsigned index = nstate*numActions_ + action;
                for( int obs = 0; obs < numObs_; ++obs ) {
                    double prob = observation_[index][obs];
                    if( prob > 0.0 ) {
                        os << "O: " << action
                           << " : " << nstate
                           << " : " << obs
                           << " " << prob
                           << endl;
                    }
                }
            }
        }
        os << endl;

        os << "# Rewards" << endl;
        for( int action = 0; action < numActions_; ++action ) {
            for( int state = 0; state < numStates_ - 1; ++state ) {
                for( int nstate = 0; nstate < numStates_ - 1; ++nstate ) {
                    double reward = getEntryMatrix(EQ[action], state, nstate);
                    reward -= 1.0 + maxReward_;
                    os << "R: " << action
                       << " : " << state
                       << " : " << nstate
                       << " : * " << reward
                       << endl;
                }
            }
        }
        os << "# maxReward = " << maxReward_ << endl;
        os << endl;
    }
}

