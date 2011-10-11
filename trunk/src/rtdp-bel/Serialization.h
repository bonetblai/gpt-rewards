//  Register.h -- Serialization
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Serialization_INCLUDE_
#define _Serialization_INCLUDE_

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct Serializable { };

struct Serialize {
    static Serializable* read(std::istream &is) { return 0; }
    static void write(const Serializable *object, std::ostream &is) { }
    static void safeRead(void *buff, const unsigned size, const unsigned num, std::istream &is) {
        is.read((char*)buff, size * num);
        if( !is ) {
            std::cerr << "Fatal Error: safeRead: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
    static void safeWrite(const void *buff, const unsigned size, const unsigned num, std::ostream &os) {
        os.write((char*)buff, size * num);
        if( !os ) {
            std::cerr << "Fatal Error: safeWrite: " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
};

#endif // _Serialization_INCLUDE_

