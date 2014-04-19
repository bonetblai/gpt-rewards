//  SB.cc -- Standard beliefs
//
//  Blai Bonet, Hector Geffner (c)

#include "SB.h"

using namespace std;

StandardBelief StandardBelief::bel_a_;
StandardBelief StandardBelief::bel_ao_;
int* StandardBelief::state_heap_ = 0;
int StandardBelief::state_heapsz_ = 0;
double* StandardBelief::state_table_ = 0;

