//  Heuristic.h -- Abstarct heuristics
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Heuristic_INCLUDE_
#define _Heuristic_INCLUDE_

#include <iostream>

class Belief;

class Heuristic {
  public:
    Heuristic() { }
    virtual ~Heuristic() { }
    virtual int action(int state) const = 0;
    virtual double value(int state) const = 0;
    virtual double value(const Belief &belief) const = 0;
};

#endif // _Heuristic_INCLUDE

