//  Theseus
//  StandardModel.cc -- Standard Model Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include <stdio.h>
#include <typeinfo>
#include <StandardModel.h>
#include <SB.h>

extern "C" {
extern int gNumGoals;
extern int gGoalList[];
}

StandardModel::StandardModel( const char *cassandraFilename )
  : Model(), goal_(0), goalSize_(0), numGoals_(0), application_(0), cost_(0), cassandra_(true), transition_(0), observation_(0)
{
  if( !readMDP((char*)cassandraFilename) ) { // parse file with Cassandra's MDP parser
    std::cerr << "Fatal Error: when reading file: " << cassandraFilename << std::endl;
    exit(-1);
  }

  // fundamental vars
  numActions_ = gNumActions;
  numStates_ = gNumStates;
  numObs_ = gNumObservations;
  underlyingDiscount_ = gDiscount;

  application_ = new unsigned[(numActions_*(1+numStates_)+32)/32];
  cost_ = new double[numActions_*(1+numStates_)];
  bzero(cost_,numActions_*(1+numStates_)*sizeof(double));
  transition_ = new std::vector<std::pair<int,double> >*[numActions_*(1+numStates_)];
  bzero(transition_,numActions_*(1+numStates_)*sizeof(void*));
  observation_ = new double*[numActions_*(1+numStates_)];
  for( int action = 0; action < numActions_; ++action ) {
    for( int nstate = 0; nstate <= numStates_; ++nstate ) {
      observation_[nstate*numActions_+action] = new double[1+numObs_];
      bzero(observation_[nstate*numActions_+action],(1+numObs_)*sizeof(double));
    }
  }

  // compute rewards and maxReward
  for( int state = 0; state < numStates_; ++state ) {
    for( int action = 0; action < numActions_; ++action ) {
      double r = getEntryMatrix(Q,action,state);
      maxReward_ = (maxReward_>r?maxReward_:r);
      cost_[state*numActions_+action] = r;
    }
  }
  for( int state = 0; state < numStates_; ++state ) {
    for( int action = 0; action < numActions_; ++action ) {
      double c = 1.0+maxReward_ - cost_[state*numActions_+action];
      cost_[state*numActions_+action] = c;
      minCost_ = (minCost_>c?c:minCost_);
    }
  }
  //destroyMatrix(Q);

  // compute model
  for( int state = 0; state < numStates_; ++state ) {
    for( int action = 0; action < numActions_; ++action ) {
      // transition model: P( s' | s, a )
      unsigned index = state*numActions_+action;
      for( int nstate = 0; nstate < numStates_; ++nstate ) {
        double p = getEntryMatrix(P[action],state,nstate);
        if( p > 0.0 ) {
          p *= underlyingDiscount_;
          if( transition_[index] == 0 ) transition_[index] = new std::vector<std::pair<int,double> >;
          transition_[index]->push_back(std::make_pair(nstate,p));
        }
      }
      // observation model: P( o | s, a )
      for( int obs = 0; obs < numObs_; ++obs ) {
        double p = getEntryMatrix(R[action],state,obs);
        if( p > 0.0 ) observation_[index][obs] = p;
      }
      // application model
      unsigned j = index>>5, off = index&0x1F, app = application_[j];
      application_[j] = app|(1<<off);
    }
  }

#if 0
  for( int action = 0; action < numActions_; ++action ) {
    destroyMatrix(P[action]);
    destroyMatrix(R[action]);
  }
  free(P);
  free(R);
#endif

  // set model for absorbing state
  absorbing_ = numStates_;
  for( int action = 0; action < numActions_; ++action ) {
    unsigned aindex = absorbing_*numActions_+action;
    if( transition_[aindex] == 0 ) transition_[aindex] = new std::vector<std::pair<int,double> >;
    transition_[aindex]->push_back(std::make_pair(absorbing_,1.0));
    for( int state = 0; state < numStates_; ++state ) {
      unsigned index = state*numActions_+action;
      if( transition_[index] == 0 ) transition_[index] = new std::vector<std::pair<int,double> >;
      transition_[index]->push_back(std::make_pair(absorbing_,1-underlyingDiscount_));
    }
    observation_[aindex][numObs_] = 1.0;
    unsigned j = aindex>>5, off = aindex&0x1F, app = application_[j];
    application_[j] = app|(1<<off);
  }
  ++numStates_;
  ++numObs_;

  // set the initial belief
  StandardBelief *belief = new StandardBelief;
  for( int state = numStates_-1; state >= 0; --state ) {
    if( (state != absorbing_) && (gInitialBelief[state] > 0.0) ) {
      belief->push_back(state,(double)gInitialBelief[state]);
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

void
StandardModel::outputCASSANDRA( std::ostream &os ) const
{
  if( cassandra_ ) {
    extern int writeTransPOMDP(std::ostream&,double);
    writeTransPOMDP(os,-1.0-maxReward_);
    os << "# reward-shift = " << -1.0-maxReward_ << std::endl;
  }
  else {
    os.setf(std::ios::fixed);
    os << "# StandardPOMDP Model -- Automatically Generated Model" << std::endl
       << std::endl
       << "discount: 1.0" << std::endl
       << "values: reward" << std::endl
       << "states: " << numStates_ << std::endl
       << "actions: " << numActions_ << std::endl
       << "observations: " << numObs_ << std::endl;

    if( numGoals_ > 0 ) {
      os << "goals:";
      for( int s = 0; s < numStates_; ++s ) {
        if( isGoal(s) ) os << " " << s;
      }
      os << std::endl;
    }
    os << std::endl;

    const StandardBelief *ibel = static_cast<const StandardBelief*>(initialBelief_);
    std::map<int,double> imap;
    for( StandardBelief::const_iterator it = ibel->begin(); it != ibel->end(); ++it ) {
      imap.insert(std::make_pair((*it).first,(*it).second));
    }
    os << "start:" << std::endl;
    int last = 0;
    for( std::map<int,double>::const_iterator it = imap.begin(); it != imap.end(); ++it ) {
      int state = (*it).first;
      double prob = (*it).second;
      for( int s = last; s < state; ++s ) os << "0.0 ";
      os << prob << " ";
      last = 1+state;
    }
    for( int s = last; s < numStates_; ++s ) os << " 0.0";
    os << std::endl << std::endl;

    os << "# Transitions" << std::endl;
    for( int action = 0; action < numActions_; ++action ) {
      for( int state = 0; state < numStates_; ++state ) {
        unsigned index = state*numActions_+action;
        if( transition_[index] ) {
          for( std::vector<std::pair<int,double> >::const_iterator it = transition_[index]->begin(); it != transition_[index]->end(); ++it ) {
            int nstate = (*it).first;
            double prob = (*it).second;
            os << "T: " << action << " : " << state << " : " << nstate << " " << prob << std::endl;
          }
        }
      }
    }
    os << std::endl;

    os << "# Observations" << std::endl;
    for( int action = 0; action < numActions_; ++action ) {
      for( int nstate = 0; nstate < numStates_; ++nstate ) {
        unsigned index = nstate*numActions_+action;
        for( int obs = 0; obs < numObs_; ++obs ) {
          double prob = observation_[index][obs];
          if( prob > 0.0 ) os << "O: " << action << " : " << nstate << " : " << obs << " " << prob << std::endl;
        }
      }
    }
    os << std::endl;

    os << "# Rewards" << std::endl;
    for( int action = 0; action < numActions_; ++action ) {
      for( int state = 0; state < numStates_-1; ++state ) {
        for( int nstate = 0; nstate < numStates_-1; ++nstate ) {
          double reward = getEntryMatrix(EQ[action],state,nstate);
          reward -= 1.0+maxReward_;
          os << "R: " << action << " : " << state << " : " << nstate << " : * " << reward << std::endl;
        }
      }
    }
    os << "# maxReward = " << maxReward_ << std::endl;
    os << std::endl;
  }
}

