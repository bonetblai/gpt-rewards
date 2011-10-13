//  Heuristic.h -- Abstarct heuristics
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Heuristic_INCLUDE_
#define _Heuristic_INCLUDE_

#include "Serialization.h"

#include <iostream>

class Belief;

class Heuristic : public Serializable {
  public:
    Heuristic() { }
    virtual ~Heuristic() { }
    virtual int action(int state) const = 0;
    virtual double value(int state) const = 0;
    virtual double value(const Belief &belief) const = 0;

    // serialization
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, Heuristic &heuristic) { }
};

#endif // _Heuristic_INCLUDE

