//  StandardModel.h -- POMDP standard model 
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _StandardModel_INCLUDE_
#define _StandardModel_INCLUDE_

#include "Model.h"
#include "Random.h"

#include <strings.h>
#include <iostream>
#include <map>
#include <set>

#include <mdp/mdp.h>
void destroyImmRewards();

class StandardModel : public Model {
  public:
    // models
    int absorbing_;
    char *goal_;
    int goalSize_;
    int numGoals_;
    unsigned *application_;
    double *cost_;
    bool cassandra_;
    std::vector<std::pair<int, double> > **transition_;
    double **observation_;

    StandardModel()
      : Model(), absorbing_(-1), goal_(0), goalSize_(0),
        numGoals_(0), application_(0), cost_(0), cassandra_(false),
        transition_(0), observation_(0) {
    }

    StandardModel(const char *cassandraFilename);

    virtual ~StandardModel() {
        delete[] goal_;
        delete initialBelief_;
        for( int action = 0; action < numActions_; ++action ) {
            for( int state = 0; state < numStates_; ++state ) {
                delete[] observation_[state*numActions_ + action];
                delete transition_[state*numActions_ + action];
            }
        }
        delete[] observation_;
        delete[] transition_;
        delete[] cost_;
        delete[] application_;

        if( cassandra_ ) {
            //destroyImmRewards();
            for( int action = 0; action < numActions_; ++action )
                destroyMatrix(EQ[action]);
            free(EQ);
        }
    }

    void setGoal(int state) {
        if( state >= goalSize_ ) {
            char *old = goal_;
            int oldsz = goalSize_ / 8;
            while( state >= goalSize_ )
                goalSize_ = goalSize_ == 0 ? 8 : 2*goalSize_;
            goal_ = new char[goalSize_ / 8];
            bzero(goal_, (goalSize_ * sizeof(char))/8);
            bcopy(old, goal_, oldsz * sizeof(char));
            delete[] old;
        }
        goal_[state/8] |= (1 << (state%8));
        ++numGoals_;
    }

#if 0
    int P_row_start(int action, int state) const {
        return P[action]->row_start[state];
    }
    int P_row_length(int action, int state) const {
        return P[action]->row_length[state]);
    }
    int P_col(int action, int j) const {
        return P[action]->col[j];
    }
    double P_mat_val(int action, int j) const {
        return P[action]->mat_val[j];
    }
    double R_entry(int action, int nstate, int obs) const {
        return getEntryMatrix(R[action], nstate, obs);
    }
#endif

    virtual double reward(int state, int action, int nstate) const {
        double r = getEntryMatrix(EQ[action], state, nstate);
        return r;
    }
    virtual double cost(int state, int action) const {
        if( cassandra_ && (state == numStates_ - 1) ) {
            return 0;
        } else {
            return cost_[state*numActions_ + action];
        }
    }
    virtual bool applicable(int state, int action) const {
        unsigned n = state*numActions_ + action, idx = n >> 5, off = n & 0x1F;
        return application_[idx] & (1<<off);
    }
    virtual bool isAbsorbing(int state) const {
        return state == absorbing_;
    }
    virtual bool isGoal(int state) const {
        return state >= goalSize_ ? false : goal_[state >> 3] & (1 << (state & 0x7));
    }
    virtual int numGoals() const {
        return numGoals_;
    }
    virtual int sampleNextState(int state, int action) const {
        return Random::sample(*transition_[state*numActions_ + action]);
    }
    virtual int sampleNextObservation(int nstate, int action) const {
        return Random::sample(observation_[nstate*numActions_ + action], numObs_);
    }
    virtual int newState() {
        return numStates_++;
    }
    virtual void newTransition(int state, int action, int nstate, double probability) {
        unsigned n = state*numActions_ + action;
        unsigned idx = n >> 5, off = n & 0x1F, app = application_[idx];
        application_[idx] = app | (1<<off);
        if( transition_[n] == 0 )
            transition_[n] = new std::vector<std::pair<int, double> >;
        unsigned i = 0;
        while( (i < transition_[n]->size()) && ((*transition_[n])[i].first != nstate) ) ++i;
        if( i < transition_[n]->size() ) {
            (*transition_[n])[i].second = probability;
        } else {
            transition_[n]->push_back(std::make_pair(nstate, probability));
        }
    }
    void outputCASSANDRA(std::ostream &os) const;

    // serialization
    static StandardModel* constructor() {
        return new StandardModel;
    }
    virtual void write(std::ostream &os) const {
        Model::write(os);
    }
    static void read(std::istream &is, StandardModel &model) {
        Model::read(is, model);
    }
};

#endif // _StandardModel_INCLUDE

