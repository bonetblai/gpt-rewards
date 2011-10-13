//  Utils.h -- General utilities
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Utils_INCLUDE_
#define _Utils_INCLUDE_

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
    return os;
}

template <class T1,class T2, class Comp, class Alloc>
std::ostream& operator<<(std::ostream &os, const std::map<T1, T2, Comp, Alloc> &m) {
    return os;
}

template<class T, class Alloc>
std::ostream& operator<<(std::ostream &os, const std::vector<T,Alloc> &v) {
    return os;
}

template<class T, class Comp, class Alloc>
std::istream& operator>>(std::istream &is, std::set<T, Comp, Alloc> &s) {
    return is;
}

template <class T1,class T2, class Comp, class Alloc>
std::istream& operator>>(std::istream &is, std::map<T1, T2, Comp, Alloc> &m) {
    return is;
}

template<class T, class Alloc>
std::istream& operator>>(std::istream &is, std::vector<T, Alloc> &v) {
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

