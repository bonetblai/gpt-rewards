//  SampleBelief.h -- Sample beliefs
//  Blai Bonet, Hector Geffner (c)

#ifndef _SampleBelief_INCLUDE_
#define _SampleBelief_INCLUDE_

#include "Belief.h"
#include "Hash.h"
#include "History.h"
#include "Problem.h"
#include "StandardModel.h"
#include "Utils.h"
#include "Random.h"

#include <math.h>
#include <cassert>
#include <set>
#include <vector>

class SampleBelief : public Belief {
  protected:
    std::multiset<int> pfilter_;

    static int num_particles_;
    static std::vector<int> particles_;
    static std::vector<double> weights_;
    static SampleBelief bel_a_;
    static SampleBelief bel_ao_;

  public:
    SampleBelief(const SampleBelief &belief) {
        *this = belief;
    }
    SampleBelief() { }
    virtual ~SampleBelief() { }

    static void initialize(int num_particles) {
        num_particles_ = num_particles;
    }
    static void finalize() {
        num_particles_ = 0;
    }

    int num_particles() const { return pfilter_.size(); }
    bool empty() const { return pfilter_.empty(); }
    void insert_particle(int state) {
        pfilter_.insert(state);
    }
    bool contains(int state) const {
        return pfilter_.find(state) != pfilter_.end();
    }

    void clear() {
        pfilter_.clear();
    }

    // TODO: improve check
    virtual bool check() const {
        return (int)pfilter_.size() == num_particles_;
    }
    virtual bool check(int state) {
        return contains(state);
    }

    virtual int sampleState() const {
        return Random::sample(pfilter_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const { }
    virtual const Belief& update(const Model *model, int action) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_a_.clear();

        // update particles 
        for( const_iterator it = begin(); it != end(); ++it ) {
            int state = *it;
            const std::vector<std::pair<int, double> > *transition = m->transition_[state*m->numActions() + action];
            int nstate = Random::sample(*transition);
            while( m->isAbsorbing(nstate) ) nstate = Random::sample(*transition);
            bel_a_.insert_particle(nstate);
        }
        return bel_a_;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_ao_.clear();

        // update particle filter:
        //     (1) compute weights for each particle given evidence (observation)
        //     (2) re-sample N=num_particles using weights

        // compute weights
        int i = 0;
        double mass = 0.0;
        particles_.clear();
        weights_.clear();
        for( const_iterator it = begin(); it != end(); ++it, ++i ) {
            int nstate = *it;
            double p = m->observation_[nstate*m->numActions() + action][obs];
            if( p > 0 ) {
                particles_.push_back(nstate);
                weights_.push_back(p);
                mass += p;
            }
        }

        if( weights_.size() > 0 ) {
            // normalize
            for(int j = 0, jsz = weights_.size(); j < jsz; ++j )
                weights_[j] /= mass;

            // re-sample particles
            for( int j = 0; j < num_particles_; ++j ) {
                int i = Random::sample(&weights_[0], weights_.size());
                assert(weights_[i] > 0);
                bel_ao_.insert_particle(particles_[i]);
            }
        } else {
            std::cout << "HOLA" << std::endl;
            while( bel_ao_.num_particles() < num_particles_ ) {
                int nstate = m->initialBelief_->sampleState();
                double p = m->observation_[nstate*m->numActions() + action][obs];
                if( p > 0 ) bel_ao_.insert_particle(nstate);
            }
        }
        assert(bel_ao_.num_particles() == num_particles_);

        return bel_ao_;
    }

    virtual Belief* clone() const {
        return new SampleBelief(*this);
    }

    virtual size_t hash() const { return 0; }

    virtual void print(std::ostream& os) const {
        os << "({";
        for( const_iterator it = begin(); it != end(); ++it )
            os << *it << ",";
        os << "},num=" << num_particles() << ")";
    }

    const SampleBelief& operator=(const SampleBelief& belief) {
        pfilter_ = belief.pfilter_;
        return *this;
    }
    virtual const Belief& operator=(const Belief &belief) {
        return operator=(static_cast<const SampleBelief&>(belief));
    }
    bool operator==(const SampleBelief& belief) const {
        return pfilter_ == belief.pfilter_;
    }
    virtual bool operator==(const Belief &belief) const {
        return operator==(static_cast<const SampleBelief&>(belief));
    }

    // iterators
    typedef std::multiset<int>::iterator iterator;
    iterator begin() { return pfilter_.begin(); }
    iterator end() { return pfilter_.end(); }

    typedef std::multiset<int>::const_iterator const_iterator;
    const_iterator begin() const { return pfilter_.begin(); }
    const_iterator end() const { return pfilter_.end(); }
};

class HistoryAndSampleBelief : public Belief {
  protected:
    History *history_;
    std::multiset<int> pfilter_;

    static int num_particles_;
    static std::vector<int> particles_;
    static std::vector<double> weights_;
    static HistoryAndSampleBelief bel_a_;
    static HistoryAndSampleBelief bel_ao_;

  public:
    HistoryAndSampleBelief(const HistoryAndSampleBelief &belief) {
        history_ = History::allocate();
        *this = belief;
    }
    HistoryAndSampleBelief() {
        history_ = History::allocate();
    }
    virtual ~HistoryAndSampleBelief() {
        History::deallocate(history_);
    }

    static void initialize(int num_states, int num_actions, int num_observations, int num_particles) {
        History::initialize(num_actions, num_observations);
        num_particles_ = num_particles;
    }
    static void finalize() {
        History::finalize();
        num_particles_ = 0;
    }

    int num_particles() const { return pfilter_.size(); }
    bool empty() const { return history_->empty() && pfilter_.empty(); }
    void insert_particle(int state) {
        pfilter_.insert(state);
    }
    bool contains(int state) const {
        return pfilter_.find(state) != pfilter_.end();
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
        return contains(state);
    }

    virtual int sampleState() const {
        return Random::sample(pfilter_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
#if 0
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bzero(nextobs, m->numObs() * sizeof(double));
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int nstate = *it;
	    const double *dptr = m->observation_[nstate*m->numActions() + action];
            for( int obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
                double p = *(dptr+obs) / num_particles_;
                nextobs[obs] += p;
                //assert(nextobs[obs] <= 1);
            }
        }
#endif
    }
    virtual const Belief& update(const Model *model, int action) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_a_.clear();

        // update history
        //*bel_a_.history_ = *history_;
        //bel_a_.history_->push_act(action);

        // update particles 
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it ) {
            int state = *it;
            const std::vector<std::pair<int, double> > *transition = m->transition_[state*m->numActions() + action];
            int nstate = Random::sample(*transition);
            while( m->isAbsorbing(nstate) ) nstate = Random::sample(*transition);
            bel_a_.insert_particle(nstate);
        }
        return bel_a_;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_ao_.clear();

        // update history
        //*bel_ao_.history_ = *history_;
        //bel_ao_.history_->push_obs(obs);

        // update particle filter:
        //     (1) compute weights for each particle given evidence (observation)
        //     (2) re-sample N=num_particles using weights

        // compute weights
        int i = 0;
        double mass = 0.0;
        particles_.clear();
        weights_.clear();
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it, ++i ) {
            int nstate = *it;
            double p = m->observation_[nstate*m->numActions() + action][obs];
            if( p > 0 ) {
                particles_.push_back(nstate);
                weights_.push_back(p);
                mass += p;
            }
        }

        if( weights_.size() > 0 ) {
            // normalize
            for(int j = 0, jsz = weights_.size(); j < jsz; ++j )
                weights_[j] /= mass;

            // re-sample particles
            for( int j = 0; j < num_particles_; ++j ) {
                int i = Random::sample(&weights_[0], weights_.size());
                assert(weights_[i] > 0);
                bel_ao_.insert_particle(particles_[i]);
            }
        } else {
            std::cout << "HOLA" << std::endl;
            while( bel_ao_.num_particles() < num_particles_ ) {
                int nstate = m->initialBelief_->sampleState();
                double p = m->observation_[nstate*m->numActions() + action][obs];
                if( p > 0 ) bel_ao_.insert_particle(nstate);
            }
        }
        assert(bel_ao_.num_particles() == num_particles_);

        return bel_ao_;
    }

    virtual Belief* clone() const {
        return new HistoryAndSampleBelief(*this);
    }

    virtual size_t hash() const {
        return history_->hash();
    }

    virtual void print(std::ostream& os) const {
        assert(history_ != 0);
        os << "(";
        //history_->print(os);
        os << ",{";
        for( const_particle_iterator it = particle_begin(); it != particle_end(); ++it )
            os << *it << ",";
        os << "},num=" << num_particles() << ")";
    }

    const HistoryAndSampleBelief& operator=(const HistoryAndSampleBelief& belief) {
        *history_ = *belief.history_;
        pfilter_ = belief.pfilter_;
        return *this;
    }
    virtual const Belief& operator=(const Belief &belief) {
        return operator=(static_cast<const HistoryAndSampleBelief&>(belief));
    }
    bool operator==(const HistoryAndSampleBelief& belief) const {
        return *history_ == *belief.history_;
    }
    virtual bool operator==(const Belief &belief) const {
        return operator==(static_cast<const HistoryAndSampleBelief&>(belief));
    }

    // iterators
    typedef std::multiset<int>::iterator particle_iterator;
    particle_iterator particle_begin() { return pfilter_.begin(); }
    particle_iterator particle_end() { return pfilter_.end(); }

    typedef std::multiset<int>::const_iterator const_particle_iterator;
    const_particle_iterator particle_begin() const { return pfilter_.begin(); }
    const_particle_iterator particle_end() const { return pfilter_.end(); }
};

#endif // _SampleBelief_INCLUDE

