//  Theseus
//  History.h -- Histories implementation.
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _History_INCLUDE_
#define _History_INCLUDE_

#include <cassert>
#include <iostream>
#include <strings.h>

inline int num_words(int num_entries, int num_entries_per_word) {
    int nw = num_entries / num_entries_per_word;
    if( num_entries % num_entries_per_word != 0 ) ++nw;
    return nw;
}

inline int num_bytes(int num_entries, int num_entries_per_word) {
    return num_words(num_entries, num_entries_per_word) * sizeof(unsigned);
}

inline int fetch(const unsigned *seq, int idx, int off, unsigned mask) {
    unsigned word = seq[idx];
    word = word >> off;
    return word & mask;
}

inline void set(unsigned *seq, int entry, int idx, int off, unsigned mask) {
    unsigned word = seq[idx];
    unsigned clear_mask = ~(mask << off);
    word = word & clear_mask;
    word = word | (entry << off);
    seq[idx] = word;
}

class History {
  protected:
    unsigned *act_seq_;
    int num_act_;
    int act_cap_;

    unsigned *obs_seq_;
    int num_obs_;
    int obs_cap_;

    mutable const History *next_;

    static History *pool_;

    static int num_bits_per_act_;
    static int num_act_per_wrd_;
    static unsigned act_mask_;

    static int num_bits_per_obs_;
    static int num_obs_per_wrd_;
    static unsigned obs_mask_;

  public:
    enum { DEFAULT_CAPACITY = 10 };
    History(unsigned capacity = DEFAULT_CAPACITY)
      : act_seq_(0), num_act_(0), act_cap_(0),
        obs_seq_(0), num_obs_(0), obs_cap_(0),
        next_(0) {
        reserve(capacity, capacity);
    }
    History(const History &h)
      : act_seq_(0), num_act_(0), act_cap_(0),
        obs_seq_(0), num_obs_(0), obs_cap_(0),
        next_(0) {
        copy(h);
    }
    virtual ~History() {
        delete[] act_seq_;
        delete[] obs_seq_;
    }

    // TODO: check border case: num_act = 1, num_obs = 1
    static void initialize(int num_actions, int num_observations) {
        assert(num_actions > 0);
        assert(num_observations > 0);
        std::cout << "num-actions      = " << num_actions << std::endl
                  << "num-observations = " << num_observations << std::endl;

        // initialize pool of histories to the empty pool
        pool_ = 0;

        // set parameters for actions 
        num_bits_per_act_ = 0;
        act_mask_ = 0;
        while( num_actions > (1 << num_bits_per_act_) ) {
            ++num_bits_per_act_;
            act_mask_ = act_mask_ << 1;
            ++act_mask_;
        }
#if 0
        while( num_actions > 1 ) {
            ++num_bits_per_act_;
            act_mask_ = act_mask_ << 1;
            ++act_mask_;
            num_actions = num_actions >> 1;
        }
#endif
        if( num_bits_per_act_ > 0 )
            num_act_per_wrd_ = (8 * sizeof(unsigned)) / num_bits_per_act_;
   
        // set parameters for observations 
        num_bits_per_obs_ = 0;
        obs_mask_ = 0;
        while( num_observations > (1 << num_bits_per_obs_) ) {
            ++num_bits_per_obs_;
            obs_mask_ = obs_mask_ << 1;
            ++obs_mask_;
        }
#if 0
        while( num_observations > 1 ) {
            ++num_bits_per_obs_;
            obs_mask_ = obs_mask_ << 1;
            ++obs_mask_;
            num_observations = num_observations >> 1;
        }
#endif
        if( num_bits_per_act_ > 0 )
            num_obs_per_wrd_ = (8 * sizeof(unsigned)) / num_bits_per_obs_;

        std::cout << "num-bits-per-act = " << num_bits_per_act_ << std::endl
                  << "num-act-per-word = " << num_act_per_wrd_ << std::endl
                  << "num-bits-per-obs = " << num_bits_per_obs_ << std::endl
                  << "num-obs-per-word = " << num_obs_per_wrd_ << std::endl;
    }
    static void finalize() { }

    // allocate history from pool (if available)
    static History* allocate(const History *h = 0) {
        if( pool_ != 0 ) {
            History *new_h = pool_;
            pool_ = const_cast<History*>(pool_->next_);
            new_h->next_ = 0;
            if( h != 0 ) new_h->copy(*h);
            return new_h;
        } else {
            return h == 0 ? new History : new History(*h);
        }
    }

    // deallocate history by inserting it into pool
    static void deallocate(const History *h) {
        const_cast<History*>(h)->clear();
        h->next_ = pool_;
        pool_ = const_cast<History*>(h);
    }

    unsigned num_act() const { return num_act_; }
    unsigned num_obs() const { return num_obs_; }
    unsigned act_cap() const { return act_cap_; }
    unsigned obs_cap() const { return obs_cap_; }

    int act(int i) const {
        if( num_bits_per_act_ == 0 )
            return 0;
        else {
            int idx = i / num_act_per_wrd_;
            int off = i % num_act_per_wrd_;
            return fetch(act_seq_, idx, off * num_bits_per_act_, act_mask_);
        }
    }
    int obs(int i) const {
        if( num_bits_per_obs_ == 0 )
            return 0;
        else {
            int idx = i / num_obs_per_wrd_;
            int off = i % num_obs_per_wrd_;
            return fetch(obs_seq_, idx, off * num_bits_per_obs_, obs_mask_);
        }
    }

    bool empty() const { return (num_act_ == 0) && (num_obs_ == 0); }
    void clear() {
        if( num_bits_per_act_ > 0 )
            bzero(act_seq_, num_bytes(num_act_, num_act_per_wrd_));
        if( num_bits_per_obs_ > 0 )
            bzero(obs_seq_, num_bytes(num_obs_, num_obs_per_wrd_));
        num_act_ = num_obs_ = 0;
    }

    void reserve(int new_act_cap, int new_obs_cap) {
        //std::cout << "reserve: new_act_cap=" << new_act_cap << ", new_obs_cap=" << new_obs_cap << std::endl
        //          << "         old_act_cap=" << act_cap_ << ", old_obs_cap=" << obs_cap_ << std::endl;
        if( (act_cap_ < new_act_cap) && (num_bits_per_act_ > 0) ) {
            //std::cout << "  inc. act_seq: new_cap = " << new_act_cap << std::endl;
            int nwords = num_words(act_cap_, num_act_per_wrd_);
            int new_nwords = num_words(new_act_cap, num_act_per_wrd_);
            unsigned *old_act_seq = act_seq_;
            act_seq_ = new unsigned[new_nwords];
            bcopy(old_act_seq, act_seq_, nwords * sizeof(unsigned));
            bzero(&act_seq_[nwords], (new_nwords-nwords) * sizeof(unsigned));
            delete[] old_act_seq;
            act_cap_ = new_nwords * num_act_per_wrd_;
        }
        if( (obs_cap_ < new_obs_cap) && (num_bits_per_obs_ > 0) ) {
            //std::cout << "  inc. obs_seq: new_cap = " << new_obs_cap << std::endl;
            int nwords = num_words(obs_cap_, num_obs_per_wrd_);
            int new_nwords = num_words(new_obs_cap, num_obs_per_wrd_);
            unsigned *old_obs_seq = obs_seq_;
            obs_seq_ = new unsigned[new_nwords];
            bcopy(old_obs_seq, obs_seq_, nwords * sizeof(unsigned));
            bzero(&obs_seq_[nwords], (new_nwords-nwords) * sizeof(unsigned));
            delete[] old_obs_seq;
            obs_cap_ = new_nwords * num_obs_per_wrd_;
        }
    }

    void copy(const History &h) {
//std::cout << "C1: num-act = " << h.num_act_ << ", num-obs = " << h.num_obs_ << std::endl;
        reserve(h.num_act_, h.num_obs_);
//std::cout << "C2: cap-act = " << act_cap_ << ", cap-obs = " << obs_cap_ << std::endl;
//std::cout << "    seq-act = " << act_seq_ << ", seq-obs = " << obs_seq_ << std::endl;
        num_act_ = h.num_act_;
        num_obs_ = h.num_obs_;
//std::cout << "C3: " << num_bytes(h.num_act_, num_act_per_wrd_) << std::endl;
        if( num_bits_per_act_ > 0 )
            bcopy(h.act_seq_, act_seq_, num_bytes(h.num_act_, num_act_per_wrd_));
//std::cout << "C4: " << num_bytes(h.num_obs_, num_obs_per_wrd_) << std::endl;
        if( num_bits_per_obs_ > 0 )
            bcopy(h.obs_seq_, obs_seq_, num_bytes(h.num_obs_, num_obs_per_wrd_));
//std::cout << "C5" << std::endl;
    }

    void push(int act, int obs) {
        push_act(act);
        push_obs(obs);
    }
    void push_act(int act) {
        if( num_bits_per_act_ > 0 ) {
            reserve(num_act_+1, num_obs_);
            //std::cout << "X1 = " << std::flush; print(std::cout); std::cout << std::endl;
            int idx = num_act_ / num_act_per_wrd_;
            int off = num_act_ % num_act_per_wrd_;
            set(act_seq_, act, idx, off * num_bits_per_act_, act_mask_);
            //std::cout << "X2 = " << std::flush; print(std::cout); std::cout << std::endl;
        }
        ++num_act_;
        //std::cout << "X3 = " << std::flush; print(std::cout); std::cout << std::endl;
    }
    void push_obs(int obs) {
        if( num_bits_per_obs_ > 0 ) {
            reserve(num_act_, num_obs_+1);
            int idx = num_obs_ / num_obs_per_wrd_;
            int off = num_obs_ % num_obs_per_wrd_;
            set(obs_seq_, obs, idx, off * num_bits_per_obs_, obs_mask_);
        }
        ++num_obs_;
    }

    void print(std::ostream &os) const {
        os << "<";
        for( int i = 0; (i < num_act_) || (i < num_obs_); ++i ) {
            if( i < num_act_ ) os << "a=" << act(i) << ",";
            if( i < num_obs_ ) os << "o=" << obs(i) << ",";
        }
        os << ">";
    }

    const History& operator=(const History &h) {
        copy(h);
        return *this;
    }
    bool operator==(const History &h) const {
        if( (num_act_ != h.num_act_) || (num_obs_ != h.num_obs_) )
            return false;
        if( num_bits_per_act_ > 0 ) {
            for( int i = 0, isz = num_words(num_act_, num_act_per_wrd_); i < isz; ++i ) {
                if( act_seq_[i] != h.act_seq_[i] )
                    return false;
            }
        }
        if( num_bits_per_obs_ > 0 ) {
            for( int i = 0, isz = num_words(num_obs_, num_obs_per_wrd_); i < isz; ++i ) {
                if( obs_seq_[i] != h.obs_seq_[i] )
                    return false;
            }
        }
        return true;
    }

    // TODO: fix hash function
    size_t hash() const {
        return 0;
    }

    // serialization
    static History* constructor() { return new History; }
    virtual void write(std::ostream& os) const {
        // TODO: fix write
    }
    static void read(std::istream& is, History &history) {
        // TODO: fix read
    }
};

#endif

