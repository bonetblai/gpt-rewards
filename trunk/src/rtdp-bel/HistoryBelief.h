//  Theseus
//  HistoryBelief.h -- History Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _HistoryBelief_INCLUDE_
#define _HistoryBelief_INCLUDE_

#include "Belief.h"
#include "History.h"
#include "Utils.h"

#include <cassert>
#include <set>

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

#define HASH_ROT(x,k) (((x)<<(k))|((x)>>(32-(k))))
#define HASH_MIX(a,b,c) \
{ \
  a -= c; a ^= HASH_ROT(c, 4); c += b; \
  b -= a; b ^= HASH_ROT(a, 6); a += c; \
  c -= b; c ^= HASH_ROT(b, 8); b += a; \
  a -= c; a ^= HASH_ROT(c,16); c += b; \
  b -= a; b ^= HASH_ROT(a,19); a += c; \
  c -= b; c ^= HASH_ROT(b, 4); b += a; \
}
#define HASH_FINAL(a,b,c) \
{ \
  c ^= b; c -= HASH_ROT(b,14); \
  a ^= c; a -= HASH_ROT(c,11); \
  b ^= a; b -= HASH_ROT(a,25); \
  c ^= b; c -= HASH_ROT(b,16); \
  a ^= c; a -= HASH_ROT(c,4);  \
  b ^= a; b -= HASH_ROT(a,14); \
  c ^= b; c -= HASH_ROT(b,24); \
}

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

    void push_back(int state, double probability) {
        //vec_[size_++] = std::make_pair(state, probability);
    }

    virtual bool check() const {
        //for( int i = 0; i < (int)size_-1; ++i ) {
        //    if( vec_[i].first >= vec_[i+1].first ) { return false; }
        //}
        return true;
    }
    virtual bool check(int state) {
        //unsigned i = 0;
        //while( (i < size_) && (vec_[i].first < state) ) ++i;
        //if( (i == size_) || (vec_[i].first > state) ) {
        //    i = insert(state, PD.epsilon_, i);
        //    assert(vec_[i].first == state);
        //    assert(vec_[i].second == PD.epsilon_);
        //    return false;
        //} else {
        //    return true;
        //}
        return true;
    }

    virtual Belief::Constructor getConstructor() const {
        return (Belief::Constructor)&HistoryBelief::constructor;
    }
    virtual int sampleState() const {
        return ::randomSampling(particles_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
        //const StandardModel *m = static_cast<const StandardModel*>(model);
        //bzero(nextobs, m->numObs() * sizeof(double));
        //for( const std::pair<int, double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr ) {
        //    assert(ptr->second > 0);
        //    int nstate = ptr->first;
        //    const double *dptr = m->observation_[nstate*m->numActions() + action];
        //    for( unsigned obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
        //        double p = ptr->second * *(dptr+obs);
        //        nextobs[obs] += p;
        //    }
        //}
    }
    virtual const Belief& update(const Model *model, int action) const {
        //const StandardModel *m = static_cast<const StandardModel*>(model);
        //double sum = 0.0;
        //bel_a_.clear();
        //for(const std::pair<int,double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr) {
        //    assert(ptr->second > 0);
        //    int state = ptr->first;
        //    double prob = ptr->second;
        //    const std::vector<std::pair<int, double> > *vec = m->transition_[state*m->numActions() + action];
        //    assert(vec != 0);
        //    for( const std::pair<int, double> *vptr = &(*vec)[0], *vend = &(*vec)[vec->size()]; vptr != vend; ++vptr ) {
        //        assert(vptr->second > 0);
        //        int nstate = vptr->first;
        //        double p = prob * vptr->second;
        //        if( p > 0 ) {
        //            if( state_table_[nstate] == 0 ) {
        //                state_heap_[state_heapsz_++] = nstate;
        //                std::push_heap(state_heap_, &state_heap_[state_heapsz_]);
        //            }
        //            state_table_[nstate] += p;
        //            sum += p;
        //        }
        //    }
        //}
        //while( state_heapsz_ ) { // populate bel_a
        //    int nstate = *state_heap_;
        //    std::pop_heap(state_heap_, &state_heap_[state_heapsz_]);
        //    bel_a_.push_back(nstate, state_table_[nstate]/sum);
        //    state_table_[nstate] = 0;
        //    --state_heapsz_;
        //}
        //return bel_a_;
        return *this;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        //const StandardModel *m = static_cast<const StandardModel*>(model);
        //double sum = 0.0;
        //bel_ao_.clear();
        //for( const std::pair<int,double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr ) {
        //    assert(ptr->second > 0);
        //    int nstate = ptr->first;
        //    double p = ptr->second * m->observation_[nstate*m->numActions()+action][obs];
        //    if( p > 0 ) {
        //        bel_ao_.push_back(nstate, p);
        //        sum += p;
        //    }
        //}
        //assert(sum > 0);
        //for( std::pair<int, double> *ptr = bel_ao_.vec_, *pend = &bel_ao_.vec_[bel_ao_.size_]; ptr != pend; ++ptr ) {
        //    ptr->second /= sum;
        //}
        //return bel_ao_;
        return *this;
    }

    virtual Belief* clone() const {
        return new HistoryBelief(*this);
    }

    virtual unsigned hashFunction() const {
#if 0
        register unsigned i = 0;
        register unsigned length = size_<<1;
        register unsigned a, b, c;
        a = b = c = 0xdeadbeef + (length<<2) + 0;
        if( length == 0 ) return c;

        unsigned *ptr;
        while( length > 6 ) {
            ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
            a += (unsigned)vec_[i].first;
            b += ptr[0] + ptr[1];
            ++i;
            c += (unsigned)vec_[i].first;
            HASH_MIX(a,b,c);
            ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
            a += ptr[0] + ptr[1];
            ++i;
            ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
            b += (unsigned)vec_[i].first;
            c += ptr[0] + ptr[1];
            HASH_MIX(a, b, c);
            ++i;
            length -= 6;
        }
        assert((length==6) || (length==4) || (length==2));

        ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
        a += (unsigned)vec_[i].first;
        b += ptr[0] + ptr[1];
        if( length == 2 ) {
            HASH_FINAL(a, b, c);
            return c;
        }
        ++i;
        c += (unsigned)vec_[i].first;
        HASH_MIX(a, b, c);
        ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
        a += ptr[0] + ptr[1];
        if( length == 4 ) {
            HASH_FINAL(a, b, c);
            return c;
        }
        ++i;
        ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
        b += (unsigned)vec_[i].first;
        c += ptr[0] + ptr[1];
        HASH_FINAL(a, b, c);
        return c; 
#endif
        return 0;
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
    typedef History::iterator iterator;
    typedef History::const_iterator const_iterator;
    typedef std::multiset<int>::iterator particle_iterator;
    typedef std::multiset<int>::const_iterator const_particle_iterator;

    iterator begin() { return history_->begin(); }
    iterator end() { return history_->end(); }
    const_iterator begin() const { return history_->begin(); }
    const_iterator end() const { return history_->end(); }

    particle_iterator particle_begin() { return particles_.begin(); }
    particle_iterator particle_end() { return particles_.end(); }
    const_particle_iterator particle_begin() const { return particles_.begin(); }
    const_particle_iterator particle_end() const { return particles_.end(); }
};

#endif // _SB_INCLUDE

