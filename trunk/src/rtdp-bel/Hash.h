//  Hash.h -- Hash Implementation
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Hash_INCLUDE_
#define _Hash_INCLUDE_

#include <iostream>

template<typename Key, typename Data> class Hash {
  public:
    struct Entry {
        const Key *key_; 
        Data data_; 
        Entry *next_;
        Entry(const Key &key, const Data &data, Entry *next) {
            key_ = new Key(key);
            data_ = data;
            next_ = next;
        }
        ~Entry() { delete key_; }
    };

  protected:
    Entry **table_;
    unsigned size_;
    unsigned mask_;
    unsigned nentries_;;
    unsigned freeslots_;
    mutable unsigned nlookups_;
    mutable unsigned nfound_;

  public:
    enum { DEFAULT_SIZE = 1<<20 };

    Hash(unsigned size = DEFAULT_SIZE)
      : table_(0), size_(0), mask_(0), nentries_(0),
        freeslots_(0), nlookups_(0), nfound_(0) {
        if( size == 0 )
            size_ = DEFAULT_SIZE;
        else {
            for( size_ = 1; (size_<<1) < size; size_ = size_<<1 );
            size_ = size_<<1;
        }
        freeslots_ = size_;
        mask_ = size_-1;
        table_ = new Entry*[size_];
        for( unsigned i = 0; i < size_; ++i ) table_[i] = 0;
    }
    virtual ~Hash() {
        clean();
        delete[] table_;
    }
  
    unsigned size() const {
        return size_;
    }
    unsigned nentries() const {
        return nentries_;
    }
    unsigned nlookups() const {
        return nlookups_;
    }
    unsigned nfound() const {
        return nfound_;
    }
    void resetStats() const {
        nlookups_ = 0;
        nfound_ = 0;
    }
    unsigned bucket(Key &key) const {
        return key.hash() & mask_;
    }

    const Entry* lookup(const Key &key) const {
        unsigned index = bucket(key);
        const Entry *entry = table_[index];
        while( (entry != 0) && !(key == *entry->key_) )
            entry = entry->next_;
        ++nlookups_;
        nfound_ += entry ? 1 : 0;
        return entry;
    }
    Entry* lookup(const Key &key) {
        unsigned index = bucket(key);
        Entry *entry = table_[index];
        while( (entry != 0) && !(key == *entry->key_) )
            entry = entry->next_;
        ++nlookups_;
        nfound_ += entry ? 1 : 0;
        return entry;
    }

    const Entry* insert(const Key &key, const Data &data) {
        unsigned index = bucket(key);
        Entry *entry = new Entry(key, data, table_[index]);
        if( table_[index] == 0 ) --freeslots_;
        table_[index] = entry;
        ++nentries_;
        return entry;
    }

    void clean() {
        for( unsigned i = 0; i < size_; ++i ) {
            Entry *entry = table_[i];
            while( entry != 0 ) {
                Entry *next = entry->next_;
                delete entry;
                entry = next;
            }
            table_[i] = 0;
        }
        nentries_ = 0;
        freeslots_ = size_;
    }

    void statistics(std::ostream& os) const {
        int slots = 0, diam = 0;
        for( unsigned i = 0; i < size_; ++i ) {
            int n = 0;
            for( const Entry *entry = table_[i]; entry != 0; entry = entry->next_, ++n );
            diam = n > diam ? n : diam;
            slots += n > 0 ? 1 : 0;
        }
        os << "%hash size " << size_ << std::endl
           << "%hash numEntries " << nentries_ << std::endl
           << "%hash freeSlots " << freeslots_ << std::endl
           << "%hash diameter " << diam << std::endl
           << "%hash avgLength " << (double)nentries_/(double)slots << std::endl
           << "%hash nlookups " << nlookups_ << std::endl
           << "%hash nfound " << nfound_ << std::endl;
    }

    void print(std::ostream& os) const {
        for( unsigned i = 0; i < size_; ++i ) {
            if( table_[i] != 0 ) {
                os << "[SLOT " << i << "]" << std::endl;
                for( const Entry *e = table_[i]; e != 0; e = e->next_ ) {
                    os << "  Key=" << *e->key_
                       << ", Data=" << e->data_ << std::endl;
                }
            }
        }
    }

    // iterators
    struct iterator {
        unsigned bucket_;
        Entry *entry_;
        Entry **table_;
        unsigned size_;
        iterator(unsigned bucket = 0, Entry *entry = 0, Entry **table = 0, unsigned size = 0)
          : bucket_(bucket), entry_(entry), table_(table), size_(size) { }
        ~iterator() { }
        Entry* operator*() const { return entry_; }
        void operator++() {
            assert(entry_ || (bucket_ == size_));
            if( bucket_ < size_ ) {
                entry_ = entry_->next_;
                while( !entry_ && (++bucket_ < size_) )
                    entry_ = table_[bucket_];
            }
            assert(entry_ || (bucket_ == size_));
        }
        bool operator!=(const iterator &i) const {
            return (bucket_!=i.bucket_) || (entry_!=i.entry_);
        }
        bool operator==(const iterator &i) const {
            return (bucket_==i.bucket_) && (entry_==i.entry_);
        }
        const iterator& operator=(const iterator &i) {
            bucket_ = i.bucket_;
            entry_ = i.entry_;
            return *this;
        }
    };
    iterator begin() {
        unsigned bucket = 0;
        Entry *entry = table_[bucket];
        while( !entry && (++bucket < size_) )
            entry = table_[bucket];
        return iterator(bucket, entry, table_,size_);
    }
    iterator end() { return iterator(size_, 0); }

    struct const_iterator {
        unsigned bucket_;
        const Entry *entry_;
        Entry** const table_;
        unsigned size_;
        const_iterator(unsigned bucket = 0, const Entry *entry = 0, Entry** const table = 0, unsigned size = 0)
          : bucket_(bucket), entry_(entry), table_(table), size_(size) { }
        ~const_iterator() { }
        void operator++() {
            assert(entry_ || (bucket_ == size_));
            if( bucket_ < size_ ) {
                entry_ = entry_->next_;
                while( !entry_ && (++bucket_ < size_) )
                    entry_ = table_[bucket_];
            }
            assert(entry_ || (bucket_ == size_));
        }
        const Entry* operator*() const { return entry_; }
        bool operator!=(const const_iterator &i) const {
            return (bucket_!=i.bucket_) || (entry_!=i.entry_);
        }
        bool operator==(const const_iterator &i) const {
            return (bucket_==i.bucket_) && (entry_==i.entry_);
        }
        const const_iterator& operator=(const const_iterator &i) {
            bucket_ = i.bucket_;
            entry_ = i.entry_;
            return *this;
        }
    };
    const_iterator begin() const {
        unsigned bucket = 0;
        const Entry *entry = table_[bucket];
        while( !entry && (++bucket < size_) )
            entry = table_[bucket];
        return const_iterator(bucket, entry, table_, size_);
    }
    const_iterator end() const { return const_iterator(size_, 0); }
};

#endif // _Hash_INCLUDE

