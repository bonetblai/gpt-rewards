//  Theseus
//  Heuristic.h -- Abstarct Belief Heuristic Class
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Heuristic_INCLUDE_
#define _Heuristic_INCLUDE_

#include <iostream>
#include <Serialization.h>

class Belief;

class Heuristic : public Serializable {
public:
  Heuristic() { }
  virtual ~Heuristic() { }
  virtual double value( int state ) const = 0;
  virtual double value( const Belief &belief ) const = 0;

  // serialization
  virtual void write( std::ostream& os ) const { }
  static void read( std::istream& is, Heuristic &heuristic ) { }
};

#endif // _Heuristic_INCLUDE

