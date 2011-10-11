//  POMDP.h -- Abstract POMDPs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _POMDP_INCLUDE_
#define _POMDP_INCLUDE_

#include "Belief.h"
#include "Model.h"
#include "Serialization.h"
#include "Utils.h"

#include <iostream>
#include <list>

class Result;
class Sondik;

struct QResult {
    double value_;
    int numTies_;
    int *ties_;
    QResult(int size)
      : value_(0), numTies_(0), ties_(new int[size]) { }
    ~QResult() {
        delete[] ties_;
    }
};

class POMDP : public Serializable {
  protected:
    int numActions_;
    int numObs_;
    int cutoff_;
    bool randomTies_;
    double epsilonGreedy_;
    mutable unsigned expansions_;
    BeliefHash* beliefHash_;
    const Model *model_;
    double learningTime_;
    double controlTime_;

  public:
    POMDP(const Model *model)
      : numActions_(model->numActions()), numObs_(model->numObs()),
        cutoff_(100), randomTies_(true), epsilonGreedy_(0), expansions_(0),
        beliefHash_(0), model_(model), learningTime_(0), controlTime_(0) { }
    virtual ~POMDP() { }

    const BeliefHash* beliefHash() const { return beliefHash_; }
    void printHash(std::ostream &os) const { beliefHash_->print(os); }
    void cleanHash() { beliefHash_->clean(); } 
    const Model* model() const { return model_; }
    int numActions() const { return numActions_; } 
    int numObs() const { return numObs_; }
    void setCutoff(int cutoff) { cutoff_ = cutoff; }
    void setRandomTies(bool randomTies) { randomTies_ = randomTies; }
    double epsilonGreedy() const { return epsilonGreedy_; }
    void setEpsilonGreedy(double epsilonGreedy) { epsilonGreedy_ = epsilonGreedy; }
    bool emptyBeliefHash() const { return beliefHash_->numEntries() == 0; }
    void setHeuristic(const Heuristic *heuristic) {
        if( beliefHash_ != 0 )
            beliefHash_->setHeuristic(heuristic);
    }
    void incLearningTime(double time) { learningTime_ += time; }
    void incControlTime(double time) { controlTime_ += time; }
    double learningTime() const { return learningTime_; }
    double controlTime() const { return controlTime_; }

    // algorithms
    virtual void learnAlgorithm(Result& result) = 0;
    virtual void controlAlgorithm(Result& result, const Sondik *sondik) const = 0;

    // virtual methods
    virtual void statistics(std::ostream &os) const {
        BeliefHash::Data data = beliefHash_->lookup(getInitialBelief(), false, false).second;
        os << "%pomdp initialBeliefValue " << data.value_ << std::endl
           << "%pomdp expansions " << expansions_ << std::endl
           << "%pomdp learningTime " << learningTime_ << std::endl
           << "%pomdp controlTime " << controlTime_ << std::endl;
        beliefHash_->statistics(os);
    }

    virtual double cost(const Belief &belief, int action) const = 0;
    virtual bool isAbsorbing(const Belief &belief) const = 0;
    virtual bool isGoal(const Belief &belief) const = 0;
    virtual bool applicable(const Belief &belief, int action) const = 0;
    virtual double QValue(const Belief &belief, int action) const = 0;
    virtual void bestQValue(const Belief &belief, QResult &qresult) const = 0;
    virtual int getBestAction(const Belief &belief) const = 0;
    virtual const Belief& getInitialBelief() const = 0;

    // serialization
    virtual void write(std::ostream &os) const {
        Serialize::safeWrite(&numActions_, sizeof(int), 1, os);
        Serialize::safeWrite(&cutoff_, sizeof(int), 1, os);
        Serialize::safeWrite(&randomTies_, sizeof(bool), 1, os);
        Serialize::safeWrite(&expansions_, sizeof(unsigned), 1, os);
        Serialize::write(beliefHash_, os);
    }
    static void read(std::istream &is, POMDP &pomdp) {
        Serialize::safeRead(&pomdp.numActions_, sizeof(int), 1, is);
        Serialize::safeRead(&pomdp.cutoff_, sizeof(int), 1, is);
        Serialize::safeRead(&pomdp.randomTies_, sizeof(bool), 1, is);
        Serialize::safeRead(&pomdp.expansions_, sizeof(unsigned), 1, is);
        pomdp.beliefHash_ = static_cast<BeliefHash*>(Serialize::read(is));
    }
};

#endif // _POMDP_INCLUDE

