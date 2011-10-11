//  Theseus
//  Utils.h -- Utilities Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Utils_INCLUDE_
#define _Utils_INCLUDE_

#include "Serialization.h"

#include <iostream>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <map>
#include <set>
#include <vector>

using namespace std;

namespace Utils {

inline double getTime() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + ((double)ru.ru_utime.tv_usec / (double)1000000);
}

inline void fill(std::map<int, double> &m, const double *array, int dimension) {
    for( int i = 0; i < dimension; ++i ) {
        if( array[i] > 0.0 )
            m[i] = array[i];
        else if( m.find(i) != m.end() )
            m.erase(i);
    }
}

template<class T, class Comp, class Alloc>
std::ostream& operator<<(std::ostream &os, const std::set<T, Comp, Alloc> &s) {
    register unsigned size = s.size();
    Serialize::safeWrite(&size, sizeof(int), 1, os);
    for( typename std::set<T, Comp, Alloc>::const_iterator it = s.begin(); it != s.end(); ++it ) {
        T elem = *it;
        Serialize::safeWrite(&elem, sizeof(T), 1, os);
    }
    return os;
}

template <class T1,class T2, class Comp, class Alloc>
std::ostream& operator<<(std::ostream &os, const std::map<T1, T2, Comp, Alloc> &m) {
    register unsigned size = m.size();
    Serialize::safeWrite(&size, sizeof(int), 1, os);
    for( typename std::map<T1, T2,Comp, Alloc>::const_iterator it = m.begin(); it != m.end(); ++it ) {
        T1 index = (*it).first;
        T2 elem = (*it).second;
        Serialize::safeWrite(&index, sizeof(T1), 1, os);
        Serialize::safeWrite(&elem, sizeof(T2), 1, os);
    }
    return os;
}

template<class T, class Alloc>
std::ostream& operator<<(std::ostream &os, const std::vector<T,Alloc> &v) {
    register unsigned size = v.size();
    Serialize::safeWrite(&size, sizeof(int), 1, os);
    for( typename std::vector<T, Alloc>::const_iterator it = v.begin(); it != v.end(); ++it ) {
        T elem = *it;
        Serialize::safeWrite(&elem, sizeof(T), 1, os);
    }
    return os;
}

template<class T, class Comp, class Alloc>
std::istream& operator>>(std::istream &is, std::set<T, Comp, Alloc> &s) {
    T elem;
    unsigned n;
    Serialize::safeRead(&n, sizeof(int), 1, is);
    for( unsigned i = 0; i < n; ++i ) {
        Serialize::safeRead(&elem, sizeof(T), 1, is);
        s.insert(elem);
    }
    return is;
}

template <class T1,class T2, class Comp, class Alloc>
std::istream& operator>>(std::istream &is, std::map<T1, T2, Comp, Alloc> &m) {
    T1 index;
    T2 elem;
    unsigned n;
    Serialize::safeRead(&n, sizeof(int), 1, is);
    for( unsigned i = 0; i < n; ++i ) {
        Serialize::safeRead(&index, sizeof(T1), 1, is);
        Serialize::safeRead(&elem, sizeof(T2), 1, is);
        m.insert(std::make_pair(index, elem));
    }
    return is;
}

template<class T, class Alloc>
std::istream& operator>>(std::istream &is, std::vector<T, Alloc> &v) {
    T elem;
    unsigned n;
    Serialize::safeRead(&n, sizeof(int), 1, is);
    for( unsigned i = 0; i < n; ++i ) {
        Serialize::safeRead(&elem, sizeof(T), 1, is);
        v.push_back(elem);
    }
    return is;
}

template<class T, class Alloc>
typename std::vector<T,Alloc>::iterator find_in_vector(std::vector<T, Alloc> &v, int i) {
    for( typename std::vector<T,Alloc>::iterator it = v.begin(); it != v.end(); ++it ) {
        if( (*it).first == i ) return it;
    }
    return v.end();
}

template<class T, class Alloc>
typename std::vector<T,Alloc>::const_iterator find_in_vector(const std::vector<T, Alloc> &v, int i) {
    for( typename std::vector<T, Alloc>::const_iterator it = v.begin(); it != v.end(); ++it ) {
        if( (*it).first == i ) return it;
    }
    return v.end();
}

}; // namespace

#endif // _Utils_INCLUDE

