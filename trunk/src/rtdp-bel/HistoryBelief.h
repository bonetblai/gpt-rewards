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

class HistoryBelief : public Belief {
  protected:
    History *history_;
    std::multiset<int> pfilter_;

    static int num_particles_;
    static int *particles_;
    static double *weights_;
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

    static void initialize(int num_states, int num_actions, int num_observations, int num_particles) {
        History::initialize(num_actions, num_observations);
        num_particles_ = num_particles;
        particles_ = new int[num_particles_];
        weights_ = new double[num_particles_];
    }
    static void finalize() {
        History::finalize();
        num_particles_ = 0;
        delete[] particles_;
        particles_ = 0;
        delete[] weights_;
        weights_ = 0;
    }

    bool empty() const { return history_->empty() && pfilter_.empty(); }
    int num_particles() const { return num_particles_; }
    void insert_particle(int particle) {
        pfilter_.insert(particle);
    }

    void clear() {
        history_->clear();
        pfilter_.clear();
    }

    // TODO: improve check
    virtual bool check() const {
        return (int)pfilter_.size() == num_particles_;
    }
    virtual bool check(int state) {
        return check();
    }

    virtual Belief::Constructor getConstructor() const {
        return (Belief::Constructor)&HistoryBelief::constructor;
    }
    virtual int sampleState() const {
        return Random::sample(pfilter_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bzero(nextobs, m->numObs() * sizeof(double));
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int nstate = *it;
	    const double *dptr = m->observation_[nstate*m->numActions() + action];
            for( int obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
                double p = *(dptr+obs) / num_particles_;
                nextobs[obs] += p;
                assert(nextobs[obs] <= 1);
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
            int nstate = Random::sample(*transition);
            bel_a_.insert_particle(nstate);
        }
        return bel_a_;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_ao_.clear();

        // update history
        *bel_ao_.history_ = *history_;
        bel_ao_.history_->push_obs(obs);

        // update particle filter:
        //     (1) compute weights for each particle given evidence (observation)
        //     (2) re-sample N=num_particles using weights

        // compute weights
        int i = 0;
        double mass = 0.0;
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it, ++i ) {
            int nstate = *it;
            particles_[i] = nstate;
            weights_[i] = m->observation_[nstate*m->numActions() + action][obs];
            mass += weights_[i];
        }

        // normalize
        for(int j = 0; j < num_particles_; ++j ) {
            weights_[j] /= mass;
        }

        // re-sample particles
        for( int j = 0; j < num_particles_; ++j ) {
            int i = Random::sample(weights_, num_particles_);
            bel_ao_.insert_particle(particles_[i]);
        }

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
        os << "},num=" << num_particles_ << ")";
    }

    const HistoryBelief& operator=(const HistoryBelief& belief) {
        *history_ = *belief.history_;
        pfilter_ = belief.pfilter_;
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
    particle_iterator particle_begin() { return pfilter_.begin(); }
    particle_iterator particle_end() { return pfilter_.end(); }

    typedef std::multiset<int>::const_iterator const_particle_iterator;
    const_particle_iterator particle_begin() const { return pfilter_.begin(); }
    const_particle_iterator particle_end() const { return pfilter_.end(); }
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
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        const HashType::Entry *entry = HashType::lookup(bel);
        if( entry ) {
            return std::make_pair(&bel, entry->data_);
        } else {
            BeliefHash::Data data(heuristic(bel), false);
            if( insert ) {
                HashType::insert(bel, data);
            }
            return std::make_pair(&bel, data);
        }
    }

    virtual void insert(const Belief &belief, double value = 0, bool solved = false) {
        HashType::insert(static_cast<const HistoryBelief&>(belief), BeliefHash::Data(value, solved));
    }

    virtual void update(const Belief &belief, double value, bool) {
        const HistoryBelief &bel = static_cast<const HistoryBelief&>(belief);
        HashType::Entry *entry = HashType::lookup(bel);
        if( entry ) {
            entry->data_.value_ *= 1+entry->data_.updates_;
            entry->data_.value_ += value;
            ++entry->data_.updates_;
            entry->data_.value_ /= 1+entry->data_.updates_;
        } else {
            HashType::insert(bel, BeliefHash::Data(value));
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

