//  Theseus
//  HistoryBelief.h -- History Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _HistoryBelief_INCLUDE_
#define _HistoryBelief_INCLUDE_

#include "Belief.h"
#include "History.h"
#include "StandardModel.h"
#include "Utils.h"

#include <cassert>
#include <set>
#include <vector>

#if 0
#include "Hash.h"
#include "StandardModel.h"
#include "Problem.h"
#include "Utils.h"

#include <iostream>
#include <strings.h>
#include <math.h>
#include <algorithm>
#include <vector>
#endif

class HistoryBelief : public Belief {
  protected:
    History *history_;
    std::multiset<int> particles_;

    static HistoryBelief bel_a_;
    static HistoryBelief bel_ao_;

  public:
    HistoryBelief(const HistoryBelief &belief) {
        history_ = History::allocate();
        *this = belief;
    }
    HistoryBelief() {
        history_ = History::allocate();
    }
    virtual ~HistoryBelief() {
        History::deallocate(history_);
    }

    static void initialize(int numStates);
    static void finalize();

    bool empty() const { return history_->empty() && particles_.empty(); }
    int num_particles() const { return particles_.size(); }

    void clear() {
        history_->clear();
        particles_.clear();
    }

    // TODO: improve check
    virtual bool check() const {
        return true;
    }
    virtual bool check(int state) {
        return true;
    }

    virtual Belief::Constructor getConstructor() const {
        return (Belief::Constructor)&HistoryBelief::constructor;
    }
    virtual int sampleState() const {
        return ::randomSampling(particles_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bzero(nextobs, m->numObs() * sizeof(double));
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int nstate = *it;
	    const double *dptr = m->observation_[nstate*m->numActions() + action];
            for( int obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
                double p = *(dptr+obs) / num_particles();
                nextobs[obs] += p;
            }
        }
    }
    virtual const Belief& update(const Model *model, int action) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_a_.clear();

        // update history
        *bel_a_.history_ = *history_;
        bel_a_.history_->push_act(action);

        // update particles 
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int state = *it;
            const std::vector<std::pair<int, double> > *transition = m->transition_[state*m->numActions() + action];
            int nstate = randomSampling(*transition);
            bel_a_.particles_.insert(nstate);
        }
        return bel_a_;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_ao_.clear();

        // update history
        *bel_ao_.history_ = *history_;
        bel_ao_.history_->push_obs(obs);

        // update particles 
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int nstate = *it;
            double p = m->observation_[nstate*m->numActions() + action][obs];
            if( p > 0 ) bel_ao_.particles_.insert(nstate);
        }
        assert(!bel_ao_.particles_.empty());
        return bel_ao_;
    }

    virtual Belief* clone() const {
        return new HistoryBelief(*this);
    }

    virtual size_t hash() const {
        return history_->hash();
    }

    virtual void print(std::ostream& os) const {
        history_->print(os);
        // TODO: print particles
    }

    const HistoryBelief& operator=(const HistoryBelief& belief) {
        *history_ = *belief.history_;
        particles_ = belief.particles_; // TODO: check if necessary or correct
        return *this;
    }
    virtual const Belief& operator=(const Belief &belief) {
        return operator=(static_cast<const HistoryBelief&>(belief));
    }
    bool operator==(const HistoryBelief& belief) const {
        return *history_ == *belief.history_;
    }
    virtual bool operator==(const Belief &belief) const {
        return operator==(static_cast<const HistoryBelief&>(belief));
    }

    // serialization
    static HistoryBelief* constructor() { return new HistoryBelief; }
    virtual void write(std::ostream& os) const {
        Belief::write(os);
        history_->write(os);
        // TODO: serialize particles
    }
    static void read(std::istream& is, HistoryBelief &belief) {
        Belief::read(is, belief);
        History::read(is, *belief.history_);
        // TODO: read particles
    }

    // iterators
    //typedef History::iterator iterator;
    //iterator begin() { return history_->begin(); }
    //iterator end() { return history_->end(); }

    //typedef History::const_iterator const_iterator;
    //const_iterator begin() const { return history_->begin(); }
    //const_iterator end() const { return history_->end(); }

    typedef std::multiset<int>::iterator particle_iterator;
    particle_iterator particle_begin() { return particles_.begin(); }
    particle_iterator particle_end() { return particles_.end(); }

    typedef std::multiset<int>::const_iterator const_particle_iterator;
    const_particle_iterator particle_begin() const { return particles_.begin(); }
    const_particle_iterator particle_end() const { return particles_.end(); }
};

#endif // _SB_INCLUDE

