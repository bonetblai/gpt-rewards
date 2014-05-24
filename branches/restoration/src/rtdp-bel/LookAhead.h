//  LookAhead.h -- Look-ahead heuristics
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _LookAhead_INCLUDE_
#define _LookAhead_INCLUDE_

#include "Belief.h"
#include "Exception.h"
#include "Heuristic.h"
#include "POMDP.h"
#include "Problem.h"

#include <iostream>

class LookAheadHeuristic : public Heuristic {
  protected:
    const POMDP *pomdp_;
    const Heuristic *h_;
    mutable int lookahead_;
    mutable double *nextobs_;

  public:
    LookAheadHeuristic(const POMDP *pomdp = 0, const Heuristic *h = 0, int lookahead = 0)
      : pomdp_(pomdp), h_(h), lookahead_(lookahead),
        nextobs_(new double[pomdp_->numObs()]) { }
    virtual ~LookAheadHeuristic() {
        delete[] nextobs_;
    }

    void setPOMDP(const POMDP *pomdp) { pomdp_ = pomdp; }
    const POMDP* getPOMDP() { return pomdp_; }
    void setHeuristic(const Heuristic *h) { h_ = h; }
    const Heuristic* getHeuristic() { return h_; }
    void setLookahead(int lookahead) const { lookahead_ = lookahead; }
    int getLookahead() { return lookahead_; }

    double value(int level, const Belief &belief) const {
        if( level == 0 ) {
            return h_->value(belief);
        } else if( !pomdp_->isAbsorbing(belief) ) {
            double best = 0;
            bool noBest = true;
            for( int action = 0; action < pomdp_->numActions(); ++action ) {
                if( pomdp_->applicable(belief, action) ) {
                    const Belief &belief_a = belief.update(pomdp_->model(), action);
                    belief_a.nextPossibleObservations(pomdp_->model(), action, nextobs_);
                    double sum = 0.0;
                    for( unsigned obs = 0, sz = pomdp_->numObs(); obs < sz; ++obs ) {
                        double prob = nextobs_[obs];
                        if( prob > 0 ) {
                            const Belief &belief_ao = belief_a.update(pomdp_->model(), action, obs);
                            double hval = value(level - 1, belief_ao);
                            sum += prob * hval;
                        }
                    }
                    sum = pomdp_->cost(belief, action) + sum;
                    if( noBest || (sum > best) ) {
                        noBest = false;
                        best = sum;
                    }
                }
            }
            return best;
        } else {
            return 0;
        }
    }

    virtual double value(int state) const {
        return h_->value(state);
    }
    virtual double value(const Belief &belief) const {
        return value(lookahead_, belief);
    }

    // serialization
    static LookAheadHeuristic* constructor() {
        return new LookAheadHeuristic;
    }
    virtual void write(std::ostream &os) const {
        Heuristic::write(os);
        Serialize::safeWrite(&lookahead_, sizeof(int), 1, os);
    }
    static void read(std::istream &is, LookAheadHeuristic &lah) {
        Heuristic::read(is, lah);
        Serialize::safeRead(&lah.lookahead_, sizeof(int), 1, is);
    }
};

#endif // _LookAhead_INCLUDE

