//  Theseus
//  HistoryBelief.h -- History Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _HistoryBelief_INCLUDE_
#define _HistoryBelief_INCLUDE_

#include "Belief.h"
#include "Hash.h"
#include "History.h"
#include "Problem.h"
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

    static void initialize(int num_states, int num_actions, int num_observations) {
        History::initialize(num_actions, num_observations);
    }
    static void finalize() {
        History::finalize();
    }

    bool empty() const { return history_->empty() && particles_.empty(); }
    int num_particles() const { return particles_.size(); }
    void insert_particle(int particle) {
        particles_.insert(particle);
    }

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
//std::cout << "A1 = " << std::flush; bel_a_.print(std::cout); std::cout << std::endl;

        // update history
        *bel_a_.history_ = *history_;
//std::cout << "A2 = " << std::flush; bel_a_.print(std::cout); std::cout << std::endl;
        bel_a_.history_->push_act(action);
//std::cout << "A3 = " << std::flush; bel_a_.print(std::cout); std::cout << std::endl;

        // update particles 
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int state = *it;
            const std::vector<std::pair<int, double> > *transition = m->transition_[state*m->numActions() + action];
            int nstate = randomSampling(*transition);
            bel_a_.particles_.insert(nstate);
        }
//std::cout << "A4 = " << std::flush; bel_a_.print(std::cout); std::cout << std::endl;
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
        assert(history_ != 0);
        os << "(";
        history_->print(os);
        os << ",{";
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it )
            os << *it << ",";
        os << "},sz=" << particles_.size() << ")";
    }

    const HistoryBelief& operator=(const HistoryBelief& belief) {
        //std::cout << "start copying belief" << std::endl;
        *history_ = *belief.history_;
        //std::cout << "middle copying belief" << std::endl;
        particles_ = belief.particles_; // TODO: check if necessary or correct
        //std::cout << "end copying belief" << std::endl;
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
    typedef std::multiset<int>::iterator particle_iterator;
    particle_iterator particle_begin() { return particles_.begin(); }
    particle_iterator particle_end() { return particles_.end(); }

    typedef std::multiset<int>::const_iterator const_particle_iterator;
    const_particle_iterator particle_begin() const { return particles_.begin(); }
    const_particle_iterator particle_end() const { return particles_.end(); }
};

class HistoryBeliefHash : public BeliefHash, public Hash<const HistoryBelief, BeliefHash::Data> {
  public:
    typedef Hash<const HistoryBelief, BeliefHash::Data> HashType;

    HistoryBeliefHash(unsigned size = 0)
      : BeliefHash(), Hash<const HistoryBelief, BeliefHash::Data>(size) { }
    virtual ~HistoryBeliefHash() { }

    bool inHash(const HistoryBelief &belief) const {
        const HashType::Entry *entry = HashType::lookup(belief);
        return entry != 0;
    }

    virtual void resetStats() const {
        HashType::resetStats();
    }
    virtual unsigned nlookups() const {
        return HashType::nlookups();
    }
    virtual unsigned nfound() const {
        return HashType::nfound();
    }
    virtual double heuristic(const Belief &belief) const {
        //std::cout << "CHECK2" << std::endl;
        //if( heuristic_ ) std::cout << "h = " << heuristic_->value(dynamic_cast<const HistoryBelief&>(belief)) << std::endl;
        return !heuristic_ ? 0 : heuristic_->value(dynamic_cast<const HistoryBelief&>(belief));
    }

    virtual BeliefHash::const_Entry fetch(const Belief &belief) const {
        const HashType::Entry *entry = HashType::lookup(static_cast<const HistoryBelief&>(belief));
        if( entry )
            return BeliefHash::const_Entry(entry->key_, &entry->data_);
        else
            return BeliefHash::const_Entry(0, 0);
    }
    virtual BeliefHash::Entry fetch(const Belief &belief) {
        HashType::Entry *entry = HashType::lookup(static_cast<const HistoryBelief&>(belief));
        if( entry )
            return BeliefHash::Entry(entry->key_, &entry->data_);
        else
            return BeliefHash::Entry(0, 0);
    }

    virtual std::pair<const Belief*, BeliefHash::Data> lookup(const Belief &belief, bool quantizied, bool insert) {
        //std::cout << "entering lookup" << std::endl;
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        const HashType::Entry *entry = HashType::lookup(bel);
        //std::cout << "  entry = " << entry << std::endl;
        if( entry ) {
            return std::make_pair(&bel, entry->data_);
        } else {
            BeliefHash::Data data(heuristic(bel), false);
            if( insert ) {
                //std::cout << "before insert" << std::endl;
                HashType::insert(bel, data);
                //std::cout << "after insert" << std::endl;
            }
            return std::make_pair(&bel, data);
        }
    }

    virtual void insert(const Belief &belief, double value = 0, bool solved = false) {
        HashType::insert(static_cast<const HistoryBelief&>(belief), BeliefHash::Data(value, solved));
    }

    virtual void update(const Belief &belief, double value, bool solved = false) {
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        HashType::Entry *entry = HashType::lookup(bel);
        if( entry ) {
            entry->data_.solved_ = solved;
            if( !PD.maxUpdate_ || (value > entry->data_.value_) )
                entry->data_.value_ = value;
            ++entry->data_.updates_;
        } else {
            HashType::insert(bel, BeliefHash::Data(value, solved));
        }
    }
 
    virtual void print(std::ostream & os) const { HashType::print(os); }
    virtual void statistics(std::ostream &os) const { HashType::statistics(os); }
    virtual void clean() { HashType::clean(); }
    virtual unsigned numEntries() const { return HashType::nentries(); }

    // seriailzation
    static HistoryBeliefHash* constructor() { return new HistoryBeliefHash; }
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, HistoryBeliefHash &hash) { }
};

#endif // _HistoryBelief_INCLUDE

