//  Theseus
//  QMDP.cc -- QMDP Heuristic for Beliefs
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "QMDP.h"
#include "StandardModel.h"

#include <iostream>
#include <math.h>
#include <map>

using namespace std;

void QMDPHeuristic::compute() {
    size_ = model_->numStates();
    double *oldtable = new double[size_];
    double *newtable = new double[size_];
    memset(oldtable, 0, size_ * sizeof(double));
    memset(newtable, 0, size_ * sizeof(double));

    // solve the MDP using Gauss-Seidel Value Iteration
    double error = 0;
    do {
        for( int state = 0; state < size_; ++state ) {
            if( !model_->isAbsorbing(state) ) {
                bool noBest = true;
                for( int action = 0; action < model_->numActions(); ++action ) {
                    if( model_->applicable(state, action) ) { // compute value for an action 
                        double sum = 0.0;
                        const vector<pair<int, double> > *vec = model_->transition_[state*model_->numActions() + action];
                        assert(vec != 0);
                        for( unsigned i = 0, sz = vec->size(); i < sz; ++i ) {
                            int nstate = (*vec)[i].first;
                            double prob = (*vec)[i].second;
                            assert( prob > 0 );
                            double value = nstate < state ? newtable[nstate] : oldtable[nstate];
                            sum += prob * value;
                        }
                        sum = model_->cost(state, action) + discount_*sum;
                        if( noBest || (sum < newtable[state]) ) {
                            noBest = false;
                            newtable[state] = sum;
                        }
                    }
                }
            }
        }

        error = 0.0;
        for( int state = 0; state < model_->numStates(); ++state ) {
            double localError = fabs(newtable[state] - oldtable[state]);
            error = localError > error ? localError : error;
        }

        double *tmptable = oldtable;
        oldtable = newtable;
        newtable = tmptable;

        if( PD.verboseLevel_ >= 100 )
            *PD.outputFile_ << "QMDP_residual=" << error << endl;
    } while( error > PD.epsilon_ );

    table_ = newtable;
    delete[] oldtable;
}

