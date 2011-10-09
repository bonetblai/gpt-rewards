//  Theseus
//  SB.cc -- Standard Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "SB.h"

using namespace std;

StandardBelief StandardBelief::bel_a_;
StandardBelief StandardBelief::bel_ao_;
int* StandardBelief::state_heap_ = 0;
int StandardBelief::state_heapsz_ = 0;
double* StandardBelief::state_table_ = 0;

