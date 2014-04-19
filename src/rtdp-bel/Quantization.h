//  Quantization.h -- Quantization of standard beliefs.
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Quantization_INCLUDE_
#define _Quantization_INCLUDE_

#include "QBelief.h"
#include "SB.h"
#include "Problem.h"

#include <iostream>
#include <math.h>

//#define BARYCENTRIC_COORDINATES

class Quantization {
  public:
    static double levels_;
    static double base_;
    static double log2rbase_;

  protected:
    mutable QBelief qbelief_;

    // for Freudenthal
    int dim_;
    double *X_;
    double *basevec_;
    std::pair<int, double> *delta_;
    double *lambda_;
    double *corner_;
#ifdef BARYCENTRIC_COORDINATES
    double **bary_;
#endif

  public:
    Quantization(double levels = 20, double base = 0.90, int dim = 0) {
        levels_ = levels;
        base_ = base;
        dim_ = dim;
        log2rbase_ = 1.0/log2(base_);
        X_ = new double[dim_];
        basevec_ = new double[dim_];
        delta_ = new std::pair<int, double>[dim_];
        lambda_ = new double[dim_];
        corner_ = new double[dim_];
#ifdef BARYCENTRIC_COORDINATES
        bary_ = new double*[1 + dim_];
        for( int i = 0; i <= dim_; ++i )
            bary_[i] = new double[dim_];
#endif
    }
    virtual ~Quantization() {
        delete[] X_;
        delete[] basevec_;
        delete[] delta_;
        delete[] lambda_;
        delete[] corner_;
    }
    const QBelief& operator()(const StandardBelief &belief) const;
};

#endif // _Quantization_INCLUDE

