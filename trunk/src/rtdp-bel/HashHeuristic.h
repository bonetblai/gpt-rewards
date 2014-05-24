//  HashHeuristic.h -- Hash heuristic for beliefs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _HashHeuristic_INCLUDE_
#define _HashHeuristic_INCLUDE_

#include "Belief.h"
#include "SB.h"
#include "Hash.h"
#include "Heuristic.h"
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
        const StandardBelief *bel = dynamic_cast<const StandardBelief*>(&belief);
        return bel != 0 ? value(*bel) : 0;
    }
};

#endif // _HashHeuristic_INCLUDE

