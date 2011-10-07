//  Theseus
//  BeliefCache.h -- Belief Cache classes
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _BeliefCache_INCLUDE_
#define _BeliefCache_INCLUDE_

#include <iostream>
#include "Belief.h"
#include "SB.h"

class Model;
class Quantization;

class BeliefCache {
public:
  struct Entry {
    unsigned refs_;
    Belief *belief_;
    Belief **table_;
    double **obs_table_;
    Entry() : refs_(0), belief_(0), table_(0), obs_table_(0) { }
    ~Entry() { delete belief_; delete[] table_; delete[] obs_table_; }
    void initialize( const Belief &belief, int nactions, int nobs )
    {
      refs_ = 0;
      if( belief_ )
        *belief_ = belief;
      else
        belief_ = belief.clone();
      if( table_ ) {
        for( int a = 0; a < nactions; ++a ) {
          for( int o = 0; o < nobs; ++o ) {
            int index = o*nactions+a;
            if( table_[index] && !((unsigned)table_[index]&0x1) ) table_[index] = (Belief*)((unsigned)table_[index]^0x1);
          }
	  if( obs_table_[a] && !((unsigned)obs_table_[a]&0x1) ) obs_table_[a] = (double*)((unsigned)obs_table_[a]^0x1);
        }
      }
    }
    void allocate( int nactions, int nobs )
    {
      if( !table_ ) {
        table_ = new Belief*[nactions*nobs];
        bzero(table_,nactions*nobs*sizeof(void*));
      }
    }
    void allocate( int nactions )
    {
      if( !obs_table_ ) {
        obs_table_ = new double*[nactions];
        bzero(obs_table_,nactions*sizeof(double*));
      }
    }
    void deallocate( int nactions, int nobs )
    {
      if( table_ ) {
        for( int a = 0; a < nactions; ++a ) {
          for( int o = 0; o < nobs; ++o ) {
            int index = o*nactions+a;
            if( table_[index] ) {
              if( (unsigned)table_[index]&0x1 ) table_[index] = (Belief*)((unsigned)table_[index]^0x1);
              delete table_[index];
            }
          }
	  if( obs_table_[a] ) {
	    if( (unsigned)obs_table_[a]&0x1 ) obs_table_[a] = (double*)((unsigned)obs_table_[a]^0x1);
            delete[] obs_table_[a];
          }
        }
      }
    }

    void insertObservations( int action, double *obs, int nactions, int nobs )
    {
      allocate(nactions);
      if( !obs_table_[action] ) obs_table_[action] = new double[nobs];
      if( (unsigned)obs_table_[action]&0x1 ) obs_table_[action] = (double*)((unsigned)obs_table_[action]^0x1);
      bcopy(obs,obs_table_[action],nobs*sizeof(double));
    }
    void insertBelief( int action, int obs, const Belief &belief, int nactions, int nobs )
    {
      allocate(nactions,nobs);
      int index = obs*nactions+action;
      if( table_[index] ) {
        if( (unsigned)table_[index]&0x1 ) table_[index] = (Belief*)((unsigned)table_[index]^0x1);
        *table_[index] = belief;
      }
      else
        table_[index] = belief.clone();
    }
    const double* nextPossibleObservations( int action ) const
    {
      return(obs_table_?((unsigned)obs_table_[action]&0x1?0:obs_table_[action]):0);
    }
    const Belief* belief_ao( int action, int obs, int nactions ) const
    {
      int index = obs*nactions+action;
      return(table_?((unsigned)table_[index]&0x1?0:table_[index]):0);
    }
    void dump( std::ostream &os, int nactions, int nobs ) const
    {
      os << "belief=" << *belief_ << std::endl;
      for( int action = 0; action < nactions; ++action ) {
        os << "  table for action=" << action << std::endl
           << "    nextObs[" << action << "]=[";
        if( obs_table_[action] && !((unsigned)obs_table_[action]&0x1) ) {
          for( int obs = 0; obs < nobs; ++obs ) os << " " << obs_table_[action][obs];
          os << " ]";
	}
        else
          os << "<null>";
        os << std::endl;
        for( int obs = 0; obs < nobs; ++obs ) {
          os << "    bel_ao[" << action << "," << obs << "]=";
          int index = obs*nactions+action;
          if( table_[index] && !((unsigned)table_[index]&0x1) )
            os << *table_[index];
          else
            os << "<null>";
          os << std::endl;
        }
      }
    }
  };

protected:
  unsigned n_;
  unsigned m_;
  unsigned nmask_;
  mutable unsigned lookups_;
  mutable unsigned hits_;
  Entry *cache_;
public:
  BeliefCache( unsigned n = 11, unsigned m = 5 ) : n_(1<<n), m_(1<<m), nmask_((1<<(n+1))-1), lookups_(0), hits_(0) { cache_ = new Entry[1<<(n+m)]; }
  virtual ~BeliefCache() { delete[] cache_; }
  double hitRatio() const { return((double)hits_/(double)lookups_); }
  std::pair<int,int> emptyStats() const
  {
    std::pair<int,int> p(0,0);
    for( unsigned i = 0; i < n_; ++i ) {
      p.first += (cache_[i*m_].belief_?1:0);
      for( unsigned j = 0; j < m_; ++j )
        p.second += (cache_[i*m_+j].belief_?1:0);
    }
    return(p);
  }
  Entry* lookup( const Belief &belief ) const
  {
    ++lookups_;
    unsigned base = belief.hashFunction()&nmask_;
    for( unsigned i = 0; i < m_; ++i ) {
      if( cache_[base+i].belief_ && (*cache_[base+i].belief_ == belief) ) {
        ++hits_;
        ++cache_[base+i].refs_;
        return(&cache_[base+i]);
      }
    }
    return(0);
  }
  Entry* insert( const Belief &belief, int nactions, int nobs )
  {
    unsigned base = belief.hashFunction()&nmask_;
    unsigned imin = 0, min = cache_[base+imin].refs_;
    for( unsigned i = 1; i < m_; ++i ) {
      if( cache_[base+i].refs_ < min ) { imin = i; min = cache_[base+imin].refs_; }
    }
    cache_[base+imin].initialize(belief,nactions,nobs);
    return(&cache_[base+imin]);
  }
  void print( std::ostream &os ) const
  {
    os << "BeliefCache: n=" << n_ << ", m=" << m_ << std::endl;
    for( unsigned i = 0; i < n_; ++i ) {
      for( unsigned j = 0; j < m_; ++j ) {
        os << "Entry(" << i << "," << j << ")=[refs=" << cache_[i*m_+j].refs_ << ",belief=" << (cache_[i*m_+j].belief_?"":"<null>");
        if( cache_[i*m_+j].belief_ ) os << *cache_[i*m_+j].belief_;
        os << std::endl;
      }
    }
  }
  void statistics( std::ostream &os ) const
  {
    std::pair<int,int> stats = emptyStats();
    os << "%cache emptyBuckets " << stats.first << std::endl
       << "%cache emptySlots " << stats.second << std::endl
       << "%cache hitRatio " << hitRatio() << std::endl;
  }
  void deallocate( int nactions, int nobs )
  {
    for( unsigned i = 0; i < n_; ++i ) {
      for( unsigned j = 0; j < m_; ++j ) {
        cache_[i*m_+j].deallocate(nactions,nobs);
      }
    }
  }
};

inline std::ostream& operator<<( std::ostream&os, const BeliefCache &cache ) { cache.print(os); return(os); }

#endif // _BeliefCache_INCLUDE

