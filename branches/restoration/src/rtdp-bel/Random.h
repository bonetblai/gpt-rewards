//  Random.h -- Functions for random numbers and sampling
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Random_INCLUDE_
#define _Random_INCLUDE_

#include "Serialization.h"

#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <map>
#include <set>
#include <vector>

using namespace std;

namespace Random {

inline double unit_interval() {
    return drand48();
}

inline unsigned uniform(unsigned max) {
    return lrand48() % max;
}

template<typename T, class Alloc>
inline T sample(const std::vector<std::pair<T, double>, Alloc> &v) {
    register double d = unit_interval();
    for( typename std::vector<std::pair<T, double>, Alloc>::const_iterator it = v.begin(); it != v.end(); ++it ) {
        if( d <= (*it).second ) return (*it).first;
        d -= (*it).second;
    }
    return v[uniform(v.size())].first;
}

template<typename T>
inline T sample(const std::pair<T, double> *vec, unsigned size) {
    register double d = unit_interval();
    for( unsigned i = 0; i < size; ++i ) {
        if( d <= vec[i].second ) return vec[i].first;
        d -= vec[i].second;
    }
    return vec[uniform(size)].first;
}

template<typename T>
inline T sample(const std::multiset<T> &ms) {
    assert(!ms.empty());
    register int i = uniform(ms.size());
    typename std::multiset<T>::const_iterator it = ms.begin();
    for( ; it != ms.end(); ++it, --i ) {
        if( i == 0 ) break;
    }
    return *it;
}

inline int sample(double *vec, unsigned size) {
    register double d = unit_interval();
    for( unsigned i = 0; i < size; ++i ) {
        if( d <= vec[i] ) return i;
        d -= vec[i];
    }
    return uniform(size);
}

}; // namespace

#endif // _Random_INCLUDE

