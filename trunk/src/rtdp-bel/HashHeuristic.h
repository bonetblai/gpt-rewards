//  HashHeuristic.h -- Hash heuristic for beliefs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _HashHeuristic_INCLUDE_
#define _HashHeuristic_INCLUDE_

#include "Belief.h"
#include "Hash.h"
#include "Heuristic.h"
#include "SB.h"
#include "Utils.h"

#include <iostream>

class HashHeuristic : public Heuristic {
  protected:
    BeliefHash *hash_;
  public:
    HashHeuristic(BeliefHash *hash = 0) : hash_(hash) { }
    virtual ~HashHeuristic() { }

    double value(const StandardBelief &belief) const {
        if( hash_ == 0 ) {
            return 0;
        } else {
            std::pair<const Belief*, BeliefHash::Data> p = hash_->lookup(belief, false, false);
            return p.second.value_;
        }
    }

    virtual int action(int state) const { return 0; }
    virtual double value(int state) const { return 0; }
    virtual double value(const Belief &belief) const {
        std::cout << "CHECK1" << std::endl;
        const StandardBelief *bel = dynamic_cast<const StandardBelief*>(&belief);
        return bel != 0 ? value(*bel) : 0;
    }

    // serialization
    static HashHeuristic* constructor() { return new HashHeuristic; }
    virtual void write(std::ostream &os) const { Heuristic::write(os); }
    static void read(std::istream &is, HashHeuristic &h) { Heuristic::read(is, h); }
};

#endif // _HashHeuristic_INCLUDE

