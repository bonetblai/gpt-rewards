//  HistoryBelief.cc -- Implementation of belief states based on histories
//                      of actions and observations.
//
//  Blai Bonet, Hector Geffner (c)

#include "HistoryBelief.h"

using namespace std;

int HistoryBelief::num_particles_ = 0;
int* HistoryBelief::particles_ = 0;
double* HistoryBelief::weights_ = 0;
HistoryBelief HistoryBelief::bel_a_;
HistoryBelief HistoryBelief::bel_ao_;

