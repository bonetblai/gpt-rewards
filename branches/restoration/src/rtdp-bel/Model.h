//  Theseus
//  Model.cc -- Abstract Model Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Model_INCLUDE_
#define _Model_INCLUDE_

#include "Serialization.h"
#include "Belief.h"

#include <iostream>
#include <float.h>
#include <list>

class Model : public Serializable {
  public:
    int numActions_;
    int numStates_;
    int numObs_;
    double underlyingDiscount_;
    double maxReward_;
    double minCost_;
    const Belief* initialBelief_;

    Model()
      : numActions_(0), numStates_(0), numObs_(0), underlyingDiscount_(1),
        maxReward_(DBL_MIN), minCost_(DBL_MAX), initialBelief_(0) { }
    virtual ~Model() { }

    int numActions() const { return numActions_; }
    int numStates() const { return numStates_; }
    int numObs() const { return numObs_; }
    double underlyingDiscount() const { return underlyingDiscount_; }
    double maxReward() const { return maxReward_; }
    const Belief* getInitialBelief() const { return initialBelief_; }
    void statistics(std::ostream &os, const char *prefix = 0) const {
        os << (prefix?prefix:"%model ") << "numberActions " << numActions_ << std::endl
           << (prefix?prefix:"%model ") << "numberStates " << numStates_ << std::endl
           << (prefix?prefix:"%model ") << "numberObservations " << numObs_ << std::endl;
    }

    virtual double cost(int state, int action, int nstate) const = 0;
    virtual double cost(int state, int action) const = 0;
    virtual bool applicable(int state, int action) const = 0;
    virtual bool isAbsorbing(int state) const = 0;
    virtual bool isGoal(int state) const = 0;
    virtual int numGoals() const = 0;
    virtual int sampleNextState(int state, int action) const = 0;
    virtual int sampleNextObservation(int nstate, int action) const = 0;
    virtual int newState() = 0;
    virtual void newTransition(int state, int action, int statePrime, double probability) = 0;

    // serialization
    virtual void write(std::ostream &os) const {
        Serialize::safeWrite(&numActions_, sizeof(int), 1, os);
        Serialize::safeWrite(&numStates_, sizeof(int), 1, os);
        Serialize::safeWrite(&numObs_, sizeof(int), 1, os);
        Serialize::safeWrite(&underlyingDiscount_, sizeof(double), 1, os);
        Serialize::write(initialBelief_, os);
    }
    static void read(std::istream &is, Model &model) {
        Serialize::safeRead(&model.numActions_, sizeof(int), 1, is);
        Serialize::safeRead(&model.numStates_, sizeof(int), 1, is);
        Serialize::safeRead(&model.numObs_, sizeof(int), 1, is);
        Serialize::safeRead(&model.underlyingDiscount_, sizeof(double), 1, is); 
        model.initialBelief_ = static_cast<Belief*>(Serialize::read(is));
    }
};

#endif // _Model_INCLUDE

