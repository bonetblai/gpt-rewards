//  HashFunction.h -- General hash functions
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _HashFunction_INCLUDE_
#define _HashFunction_INCLUDE_

#include <cassert>
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

namespace HashFunction {

inline size_t hash(register const unsigned *seq, register unsigned len) {
    register unsigned a, b, c;
    a = b = c = 0xdeadbeef + (len<<2) + 0;
    if( len == 0 ) return c;

    while( len > 3 ) {
        a += *seq++;
        b += *seq++;
        c += *seq++;
        HASH_MIX(a, b, c);
        len -= 3;
    }
    assert((len == 3) || (len == 2) || (len == 1));

    switch( len ) {
        case 3: c += seq[2];
        case 2: b += seq[1];
        case 1: a += seq[0];
            HASH_FINAL(a, b, c);
    }
    return c; 
}

inline size_t hash(register const std::pair<int, double> *seq, register unsigned len) {
    register unsigned a, b, c;
    a = b = c = 0xdeadbeef + (len<<3) + 0;
    if( len == 0 ) return c;

    len = len << 1;
    register unsigned i = 0, *ptr = 0;
    while( len > 6 ) {
        ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
        a += (unsigned)seq[i].first;
        b += ptr[0] + ptr[1];
        ++i;
        c += (unsigned)seq[i].first;
        HASH_MIX(a, b, c);
        ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
        a += ptr[0] + ptr[1];
        ++i;
        b += (unsigned)seq[i].first;
        ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
        c += ptr[0] + ptr[1];
        HASH_MIX(a, b, c);
        ++i;
        len -= 6;
    }
    assert((len == 6) || (len == 4) || (len == 2));

    ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
    a += (unsigned)seq[i].first;
    b += ptr[0] + ptr[1];
    if( len == 2 ) goto final;

    ++i;
    c += (unsigned)seq[i].first;
    HASH_MIX(a, b, c);
    ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
    a += ptr[0] + ptr[1];
    if( len == 4 ) goto final;

    ++i;
    ptr = reinterpret_cast<unsigned*>(const_cast<double*>(&seq[i].second));
    b += (unsigned)seq[i].first;
    c += ptr[0] + ptr[1];

  final:
    HASH_FINAL(a, b, c);
    return c; 
}

}; // namespace

#endif // _HashFunction_INCLUDE

