//  Theseus
//  Quantization.h -- Belief Quantization
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Quantization_INCLUDE_
#define _Quantization_INCLUDE_

#include "QBelief.h"
#include "SB.h"
#include "Problem.h"

#include <iostream>
#include <math.h>

class Quantization {
protected:
  mutable QBelief qbelief_;
  // for Freudenthal
  int dim_;
  double *X_;
  double *basevec_;
  std::pair<int,double> *delta_;
  double *lambda_;
  double *corner_;
  //double **bary_;
public:
  static double levels_;
  static double base_;
  static double log2rbase_;
public:
  Quantization( double levels = 20, double base = 0.90, int dim = 0 )
  {
    levels_ = levels;
    base_ = base;
    log2rbase_ = 1.0/log2(base_);
    dim_ = dim;
    X_ = new double[dim_];
    basevec_ = new double[dim_];
    delta_ = new std::pair<int,double>[dim_];
    lambda_ = new double[dim_];
    corner_ = new double[dim_];
    //bary_ = new double*[1+dim_];
    //for( int i = 0; i <= dim_; ++i ) bary_[i] = new double[dim_];
  }
  virtual ~Quantization()
  {
    delete[] X_;
    delete[] basevec_;
    delete[] delta_;
    delete[] lambda_;
    delete[] corner_;
  }
  const QBelief& operator()( const StandardBelief &belief ) const;
};

#endif // _Quantization_INCLUDE

