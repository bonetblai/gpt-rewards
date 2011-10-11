//  QBelief.h -- Quantization of beliefs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _QBelief_INCLUDE_
#define _QBelief_INCLUDE_

#include "Belief.h"
#include "SB.h"
#include "Hash.h"
#include "StandardModel.h"
#include "Problem.h"
#include "Utils.h"

#include <iostream>
#include <strings.h>
#include <math.h>
#include <map>
#include <set>

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

class QBelief : public Belief {
  protected:
    unsigned *vec_;
    unsigned capacity_;
    unsigned size_;

  public:
    QBelief(const QBelief &belief) : vec_(0), capacity_(0), size_(0) {
        *this = belief;
    }
    QBelief(unsigned capacity = 0) : vec_(0), capacity_(0), size_(0) {
        resize(capacity);
    }
    virtual ~QBelief() { if( vec_ ) delete[] vec_; }

    unsigned size() const { return size_; }
    unsigned capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    void clear() { size_ = 0; }
    void resize(unsigned capacity) {
        if( capacity_ < capacity ) {
            capacity_ = capacity;
            unsigned *oldvec = vec_;
            vec_ = new unsigned[capacity_];
            bcopy(oldvec, vec_, size_ * sizeof(unsigned));
            if( oldvec ) delete[] oldvec;
        }
    }

    void push_back(unsigned entry) {
        resize(size_ + 1);
        vec_[size_++] = entry;
    }
    void push_back(int state, int rank) {
        resize(size_ + 1);
        vec_[size_++] = (((unsigned)state)<<8) | rank;
    }
    unsigned insert(int state, unsigned rank, unsigned loc) {
        resize(size_ + 1);
        bcopy(&vec_[loc], &vec_[loc+1], (size_ - loc) * sizeof(unsigned));
        vec_[loc] = (((unsigned)state)<<8) | rank;
        ++size_;
        return loc;
    }

    virtual bool check() const {
        for( int i = 0; i < (int)size_ - 1; ++i ) {
            if( (vec_[i]>>8) >= (vec_[i+1]>>8) ) return false;
        }
        return true;
    }
    virtual bool check(int state) {
        throw(0);
        return 0;
    }
    virtual Belief::Constructor getConstructor() const {
        return (Belief::Constructor)&QBelief::constructor;
    }
    virtual int sampleState() const {
        throw(0);
        return 0;
    }

    virtual void nextPossibleObservations(const Model *model, int action, double *nextobs) const {
        throw(0);
    }
    virtual const Belief& update(const Model *model, int action) const {
        throw(0);
        return *this;
    }
    virtual const Belief& update(const Model *model, int action, int obs) const {
        throw(0);
        return *this;
    }

    virtual Belief* clone() const { return new QBelief(*this); }
    virtual size_t hash() const {
        register unsigned a, b, c, length = size_;
        a = b = c = 0xdeadbeef + (length<<2) + 0;
        if( length == 0 ) return c;

        register unsigned *p = vec_;
        while( length > 3 ) {
            a += *p++;
            b += *p++;
            c += *p++;
            HASH_MIX(a, b, c);
            length -= 3;
        }
        assert((length==3) || (length==2) || (length==1));

        switch( length ) {
            case 3: c += p[2];
            case 2: b += p[1];
            case 1: a += p[0];
                HASH_FINAL(a, b, c);
        }
        return c; 
    }

    virtual void print(std::ostream &os) const {
        os << "[ ";
        for( const_iterator it = begin(); it != end(); ++it ) {
            int state = (*it) >> 8;
            int rank = (*it) & 0xFF;
            os << state << ":" << rank << " ";
        }
        os << "]";
    }

    const QBelief& operator=(const QBelief& belief) {
        resize(belief.size_);
        size_ = belief.size_;
        bcopy(belief.vec_, vec_, size_ * sizeof(unsigned));
        return *this;
    }
    virtual const Belief& operator=(const Belief &belief) {
        return operator=(static_cast<const QBelief&>(belief));
    }
    bool operator==(const QBelief& belief) const {
        if( size_ != belief.size_ ) return false;
        for( unsigned i = 0; i < size_; ++i ) {
            if( vec_[i] != belief.vec_[i] ) return false;
        }
        return true;
    }
    virtual bool operator==(const Belief &belief) const {
        return operator==(static_cast<const QBelief&>(belief));
    }

    // serialization
    static QBelief* constructor() { return new QBelief; }
    virtual void write(std::ostream &os) const {
        Belief::write(os);
        Serialize::safeWrite(&size_, sizeof(unsigned), 1, os);
        for( unsigned i = 0; i < size_; ++i )
            Serialize::safeWrite(&vec_[i], sizeof(unsigned), 1, os);
    }
    static void read(std::istream &is, QBelief &belief) {
        Belief::read(is, belief);
        belief.clear();
        unsigned size = 0;
        Serialize::safeRead(&size, sizeof(unsigned), 1, is);
        for( unsigned i = 0; i < size; ++i ) {
            unsigned entry = 0;
            Serialize::safeRead(&entry, sizeof(unsigned), 1, is);
            belief.push_back(entry);
        }
    }

    struct iterator {
        unsigned *p_;
        iterator(unsigned *p) : p_(p) { }
        unsigned& operator*() { return *p_; }
        void operator++() { ++p_; }
        void operator--() { --p_; }
        bool operator!=(const iterator &i) const {
            return p_ != i.p_;
        }
        bool operator==(const iterator &i) const {
            return p_ == i.p_;
        }
        const iterator& operator=(const iterator &i) {
            p_ = i.p_;
            return *this;
        }
    };
    iterator begin() { return iterator(vec_); }
    iterator end() { return iterator(&vec_[size_]); }

    struct const_iterator {
        const unsigned *p_;
        const_iterator(const unsigned *p) : p_(p) { }
        const unsigned& operator*() { return *p_; }
        void operator++() { ++p_; }
        void operator--() { --p_; }
        bool operator!=(const const_iterator &i) const {
            return p_ != i.p_;
        }
        bool operator==(const const_iterator &i) const {
            return p_ == i.p_;
        }
        const const_iterator& operator=(const const_iterator &i) {
            p_ = i.p_;
            return *this;
        }
    };
    const_iterator begin() const { return const_iterator(vec_); }
    const_iterator end() const { return const_iterator(&vec_[size_]); }
};

class QBeliefHash : public BeliefHash, public Hash<const QBelief, BeliefHash::Data> {
    static unsigned timestamp_;

  public:
    typedef Hash<const QBelief, BeliefHash::Data> HashType;

    QBeliefHash(unsigned size = 0)
      : BeliefHash(), Hash<const QBelief, BeliefHash::Data>(size) { }
    virtual ~QBeliefHash() { }

    bool inHash(const QBelief &belief) const {
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
        //std::cout << "CHECK3" << std::endl;
        return !heuristic_ ? 0 : heuristic_->value(dynamic_cast<const StandardBelief&>(belief));
    }
    virtual BeliefHash::const_Entry fetch(const Belief &belief) const {
        const HashType::Entry *entry = HashType::lookup(static_cast<const QBelief&>(belief));
        if( entry )
            return BeliefHash::const_Entry(entry->key_, &entry->data_);
        else
            return BeliefHash::const_Entry(0, 0);
    }
    virtual BeliefHash::Entry fetch(const Belief &belief) {
        HashType::Entry *entry = HashType::lookup(static_cast<const QBelief&>(belief));
        if( entry )
            return BeliefHash::Entry(entry->key_, &entry->data_);
        else
            return BeliefHash::Entry(0, 0);
    }
    virtual std::pair<const Belief*, BeliefHash::Data> lookup(const Belief &belief, bool quantizied, bool insert);
    virtual void insert(const Belief &belief, double value = 0, bool solved = false) {
        HashType::insert(static_cast<const QBelief&>(belief), BeliefHash::Data(value, solved, 0, timestamp_++));
    }
    virtual void update(const Belief &belief, double value, bool solved = false) {
        const QBelief &bel = static_cast<const QBelief&>(belief);
        HashType::Entry *entry = HashType::lookup(bel);
        if( entry ) {
            entry->data_.solved_ = solved;
            if( !PD.maxUpdate_ || (value > entry->data_.value_) )
                entry->data_.value_ = value;
            ++entry->data_.updates_;
            entry->data_.timestamp_ = timestamp_++;
        } else {
            HashType::insert(bel, BeliefHash::Data(value, solved, 0, timestamp_++));
        }
    }
 
    virtual void print(std::ostream & os) const { HashType::print(os); }
    virtual void statistics(std::ostream &os) const { HashType::statistics(os); }
    virtual void clean() { HashType::clean(); }
    virtual unsigned numEntries() const { return HashType::nentries(); }

    // seriailzation
    static QBeliefHash* constructor() { return new QBeliefHash; }
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, QBeliefHash &hash) { }
};

#endif // _QBelief_INCLUDE_

