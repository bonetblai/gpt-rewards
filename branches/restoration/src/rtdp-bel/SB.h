//  Theseus
//  SB.h -- Standard Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _SB_INCLUDE_
#define _SB_INCLUDE_

#include "Belief.h"
#include "Hash.h"
#include "StandardModel.h"
#include "Problem.h"
#include "Utils.h"

#include <iostream>
#include <strings.h>
#include <math.h>
#include <algorithm>
#include <vector>

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

class StandardBelief : public Belief {
  protected:
    std::pair<int, double> *vec_;
    unsigned capacity_;
    unsigned size_;
    static StandardBelief bel_a_;
    static StandardBelief bel_ao_;
    static int *state_heap_;
    static int state_heapsz_;
    static double *state_table_;

  public:
    StandardBelief(const StandardBelief &belief)
      : vec_(0), capacity_(0), size_(0) {
        *this = belief;
    }
    StandardBelief(unsigned capacity = 0)
      : vec_(0), capacity_(0), size_(0) {
        resize(capacity);
    }
    StandardBelief(const std::vector<std::pair<int, double> > &vector)
      : vec_(0), capacity_(0), size_(0) {
        *this = vector;
    }
    virtual ~StandardBelief() {
        if( vec_ ) delete[] vec_;
    }
    static void initialize(int numStates);
    static void finalize();

    unsigned size() const { return size_; }
    unsigned capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    void clear() { size_ = 0; }
    void resize(unsigned capacity) {
        if( capacity_ < capacity ) {
            capacity_ = capacity;
            std::pair<int, double> *oldvec = vec_;
            vec_ = new std::pair<int, double>[capacity_];
            bcopy(oldvec, vec_, size_ * sizeof(std::pair<int, double>));
            if( oldvec ) delete[] oldvec;
        }
    }

    void normalize() {
        double sum = 0;
        for( iterator it = begin(); it != end(); ++it )
            sum += (*it).second;
        for( iterator it = begin(); it != end(); ++it )
            (*it).second /= sum;
    }
    void push_back(int state, double probability) {
        resize(size_ + 1);
        vec_[size_++] = std::make_pair(state, probability);
    }
    unsigned insert(int state, double probability, unsigned loc) {
        resize(size_ + 1);
        bcopy(&vec_[loc], &vec_[loc+1], (size_ - loc) * sizeof(std::pair<int, double>));
        vec_[loc] = std::make_pair(state, probability);
        ++size_;
        return loc;
    }

    virtual bool check() const {
        for( int i = 0; i < (int)size_ - 1; ++i ) {
            if( vec_[i].first >= vec_[i+1].first )
                return false;
        }
        return true;
    }
    virtual bool check(int state) {
        unsigned i = 0;
        while( (i < size_) && (vec_[i].first < state) ) ++i;
        if( (i == size_) || (vec_[i].first > state) ) {
            i = insert(state, PD.epsilon_, i);
            assert(vec_[i].first == state);
            assert(vec_[i].second == PD.epsilon_);
            normalize();
            return false;
        } else {
            return true;
        }
    }

    virtual Belief::Constructor getConstructor() const {
        return (Belief::Constructor)&StandardBelief::constructor;
    }
    virtual int sampleState() const {
        return ::randomSampling(vec_, size_);
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bzero(nextobs, m->numObs() * sizeof(double));
        for( const std::pair<int, double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr ) {
            assert(ptr->second > 0);
            int nstate = ptr->first;
            const double *dptr = m->observation_[nstate*m->numActions() + action];
            for( unsigned obs = 0, sz = m->numObs(); obs < sz; ++obs ) {
                double p = ptr->second * *(dptr + obs);
                nextobs[obs] += p;
            }
        }
    }
    virtual const Belief& update(const Model *model, int action) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_a_.clear();

        double sum = 0.0;
        for( const std::pair<int, double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr ) {
            assert(ptr->second > 0);
            int state = ptr->first;
            double prob = ptr->second;
            const std::vector<std::pair<int, double> > *vec = m->transition_[state*m->numActions() + action];
            assert(vec != 0);
            for( const std::pair<int, double> *vptr = &(*vec)[0], *vend = &(*vec)[vec->size()]; vptr != vend; ++vptr ) {
                assert(vptr->second > 0);
                int nstate = vptr->first;
                double p = prob * vptr->second;
                if( p > 0 ) {
                    if( state_table_[nstate] == 0 ) {
                        state_heap_[state_heapsz_++] = nstate;
                        std::push_heap(state_heap_, &state_heap_[state_heapsz_]);
                    }
                    state_table_[nstate] += p;
                    sum += p;
                }
            }
        }

        while( state_heapsz_ ) { // populate bel_a
            int nstate = *state_heap_;
            std::pop_heap(state_heap_, &state_heap_[state_heapsz_]);
            bel_a_.push_back(nstate, state_table_[nstate] / sum);
            state_table_[nstate] = 0;
            --state_heapsz_;
        }
        return bel_a_;
    }

    virtual const Belief& update(const Model *model, int action, int obs) const {
        const StandardModel *m = static_cast<const StandardModel*>(model);
        bel_ao_.clear();

        double sum = 0.0;
        for( const std::pair<int, double> *ptr = vec_, *pend = &vec_[size_]; ptr != pend; ++ptr ) {
            assert(ptr->second > 0);
            int nstate = ptr->first;
            double p = ptr->second * m->observation_[nstate*m->numActions() + action][obs];
            if( p > 0 ) {
                bel_ao_.push_back(nstate, p);
                sum += p;
            }
        }
        assert(sum > 0);

        for( std::pair<int, double> *ptr = bel_ao_.vec_, *pend = &bel_ao_.vec_[bel_ao_.size_]; ptr != pend; ++ptr ) {
            ptr->second /= sum;
        }
        return bel_ao_;
    }

    virtual Belief* clone() const {
        return new StandardBelief(*this);
    }

    virtual unsigned hash() const {
        register unsigned i = 0;
        register unsigned length = size_ << 1;
        register unsigned a, b, c;
        a = b = c = 0xdeadbeef + (length << 2) + 0;
        if( length == 0 ) return c;

        unsigned *ptr;
        while( length > 6 ) {
            ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
            a += (unsigned)vec_[i].first;
            b += ptr[0] + ptr[1];
            ++i;
            c += (unsigned)vec_[i].first;
            HASH_MIX(a, b, c);
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
        if( length == 2 ) goto final;

        ++i;
        c += (unsigned)vec_[i].first;
        HASH_MIX(a, b, c);
        ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
        a += ptr[0] + ptr[1];
        if( length == 4 ) goto final;

        ++i;
        ptr = reinterpret_cast<unsigned*>(&vec_[i].second);
        b += (unsigned)vec_[i].first;
        c += ptr[0] + ptr[1];

      final:
        HASH_FINAL(a, b, c);
        return c; 
    }

    virtual void print(std::ostream &os) const {
        os << "[ ";
        for( const_iterator it = begin(); it != end(); ++it )
            os << (*it).first << ":" << (*it).second << " ";
        os << "]";
    }

    const StandardBelief& operator=(const StandardBelief &belief) {
        resize(belief.size_);
        size_ = belief.size_;
        bcopy(belief.vec_, vec_, size_ * sizeof(std::pair<int, double>));
        return *this;
    }
    virtual const Belief& operator=(const Belief &belief) {
        return operator=(static_cast<const StandardBelief&>(belief));
    }
    bool operator==(const StandardBelief &belief) const {
        if( size_ != belief.size_ ) return false;
        for( unsigned i = 0; i < size_; ++i ) {
            if( vec_[i] != belief.vec_[i] )
                return false;
        }
        return true;
    }
    virtual bool operator==(const Belief &belief) const {
        return operator==(static_cast<const StandardBelief&>(belief));
    }

    // serialization
    static StandardBelief* constructor() {
        return new StandardBelief;
    }
    virtual void write(std::ostream &os) const {
        Belief::write(os);
        Serialize::safeWrite(&size_, sizeof(unsigned), 1, os);
        for( unsigned i = 0; i < size_; ++i ) {
            Serialize::safeWrite(&vec_[i].first, sizeof(int), 1, os);
            Serialize::safeWrite(&vec_[i].second, sizeof(double), 1, os);
        }
    }
    static void read(std::istream &is, StandardBelief &belief) {
        Belief::read(is,belief);
        belief.clear();
        unsigned size = 0;
        Serialize::safeRead(&size, sizeof(unsigned), 1, is);
        for( unsigned i = 0; i < size; ++i ) {
            int s = 0;
            double p = 0;
            Serialize::safeRead(&s, sizeof(int), 1, is);
            Serialize::safeRead(&p, sizeof(double), 1, is);
            belief.push_back(s, p);
        }
    }

    struct iterator {
        std::pair<int, double> *p_;
        iterator(std::pair<int, double> *p) : p_(p) { }
        std::pair<int, double>& operator*() { return *p_; }
        void operator++() { ++p_; }
        void operator--() { --p_; }
        bool operator!=(const iterator &i) const { return p_ != i.p_; }
        bool operator==(const iterator &i) const { return p_ == i.p_; }
        const iterator& operator=(const iterator &i) {
            p_ = i.p_;
            return *this;
        }
    };
    iterator begin() { return iterator(vec_); }
    iterator end() { return iterator(&vec_[size_]); }

    struct const_iterator {
        const std::pair<int, double> *p_;
        const_iterator(const std::pair<int, double> *p) : p_(p) { }
        const std::pair<int, double>& operator*() { return *p_; }
        void operator++() { ++p_; }
        void operator--() { --p_; }
        bool operator!=(const const_iterator &i) const { return p_ != i.p_; }
        bool operator==(const const_iterator &i) const { return p_ == i.p_; }
        const const_iterator& operator=(const const_iterator &i) {
            p_ = i.p_;
            return *this;
        }
    };
    const_iterator begin() const { return const_iterator(vec_); }
    const_iterator end() const { return const_iterator(&vec_[size_]); }
};

#endif // _SB_INCLUDE

