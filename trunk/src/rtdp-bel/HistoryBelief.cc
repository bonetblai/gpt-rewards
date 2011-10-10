//  Theseus
//  HistoryBelief.cc -- History Belief Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "HistoryBelief.h"

using namespace std;

int HistoryBelief::num_particles_ = 0;
int* HistoryBelief::particles_ = 0;
double* HistoryBelief::weights_ = 0;
HistoryBelief HistoryBelief::bel_a_;
HistoryBelief HistoryBelief::bel_ao_;

