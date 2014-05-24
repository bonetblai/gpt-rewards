//  Belief.h -- Abstract beliefs and belief hashes
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Belief_INCLUDE_
#define _Belief_INCLUDE_

#include "Heuristic.h"
#include "Serialization.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>

class Model;
class Quantization;

class Belief : public Serializable {
  public:
    typedef Belief* (*Constructor)();

    Belief() { }
    virtual ~Belief() { }

    virtual Constructor getConstructor() const = 0;
    virtual bool check() const = 0;
    virtual bool check(int state) = 0;
    virtual int sampleState() const = 0;
    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const = 0;
    virtual const Belief& update(const Model *model, int action) const = 0;
    virtual const Belief& update(const Model *model, int action, int obs) const = 0;
    virtual Belief* clone() const = 0;
    virtual unsigned hash() const = 0;
    virtual void print(std::ostream &os) const = 0;
    virtual const Belief& operator=(const Belief &belief) = 0;
    virtual bool operator==(const Belief &belief) const = 0;

    // serialization
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, Belief &belief) { }
};

inline std::ostream& operator<<(std::ostream& os, const Belief& belief) {
    belief.print(os);
    return os;
}

class BeliefHash: public Serializable {
  protected:
    const Heuristic *heuristic_;
    const Quantization *quantization_;

  public:
    BeliefHash() : heuristic_(0), quantization_(0) { }
    virtual ~BeliefHash() { }
    void setHeuristic(const Heuristic *heuristic) { heuristic_ = heuristic; }
    void setQuantization(const Quantization *quantization) { quantization_ = quantization; }

    struct Data {
        double value_;
        bool solved_;
        unsigned updates_;
        unsigned timestamp_;
        Data(double value = 0, bool solved = false, unsigned updates = 0, unsigned timestamp = 0)
          : value_(value), solved_(solved), updates_(updates), timestamp_(timestamp) { }
    };

    typedef std::pair<const Belief*, const Data*> const_Entry;
    typedef std::pair<const Belief*, Data*> Entry;

    virtual void resetStats() const = 0;
    virtual unsigned nlookups() const = 0;
    virtual unsigned nfound() const = 0;
    virtual double heuristic(const Belief &belief) const = 0;
    virtual const_Entry fetch(const Belief &belief) const = 0;
    virtual Entry fetch(const Belief &belief) = 0;
    virtual std::pair<const Belief*, Data> lookup(const Belief &belief, bool quantizied, bool insert) = 0;
    virtual void insert(const Belief &belief, double value = 0, bool solved = false) = 0;
    virtual void update(const Belief &belief, double value, bool solved = false) = 0;
    virtual void print(std::ostream &os) const = 0;
    virtual void statistics(std::ostream &os) const = 0;
    virtual void clean() = 0;
    virtual unsigned numEntries() const = 0;

    // serialization
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, BeliefHash &beliefHash) { }
};

inline std::ostream& operator<<(std::ostream &os, const BeliefHash::Data &data) {
    os << "[" << "value=" << data.value_
              << ",solved=" << (data.solved_ ? "true" : "false")
       << "],updates=" << data.updates_
       << ",timestamp=" << data.timestamp_;
    return os;
}

#endif // _Belief_INCLUDE

