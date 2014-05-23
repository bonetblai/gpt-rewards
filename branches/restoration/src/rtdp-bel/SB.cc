//  Theseus
//  SB.cc -- Standard Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "SB.h"

StandardBelief StandardBelief::bel_a_;
StandardBelief StandardBelief::bel_ao_;
int* StandardBelief::state_heap_ = 0;
int StandardBelief::state_heapsz_ = 0;
double* StandardBelief::state_table_ = 0;

void
StandardBelief::initialize( int numStates )
{
  state_heap_ = new int[numStates];
  state_table_ = new double[numStates];
  bzero(state_table_,numStates*sizeof(double));
}

void
StandardBelief::finalize()
{
  delete[] state_table_;
  delete[] state_heap_;
  state_heap_ = 0;
  state_table_ = 0;
}

