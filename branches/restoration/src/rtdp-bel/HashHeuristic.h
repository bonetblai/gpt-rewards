//  Theseus
//  HashHeuristic.h -- Hash Heuristic for Beliefs
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _HashHeuristic_INCLUDE_
#define _HashHeuristic_INCLUDE_

#include "Heuristic.h"
#include "Hash.h"
#include "Belief.h"
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
    virtual double value(int state) const { return 0; }
    virtual double value(const Belief &belief) const {
        const StandardBelief *sbelief = dynamic_cast<const StandardBelief*>(&belief);
        return sbelief != 0 ? value(*sbelief) : 0;
    }

    // serialization
    static HashHeuristic* constructor() { return new HashHeuristic; }
    virtual void write(std::ostream &os) const { Heuristic::write(os); }
    static void read(std::istream &is, HashHeuristic &h) { Heuristic::read(is, h); }
};

#endif // _HashHeuristic_INCLUDE

