//  HistoryBelief.cc -- Implementation of belief states based on histories
//                      of actions and observations.
//
//  Blai Bonet, Hector Geffner (c)

#include "HistoryBelief.h"

using namespace std;

HistoryBelief HistoryBelief::bel_a_;
HistoryBelief HistoryBelief::bel_ao_;

int HistoryAndSampleBelief::num_particles_ = 0;
vector<int> HistoryAndSampleBelief::particles_;
vector<double> HistoryAndSampleBelief::weights_;
HistoryAndSampleBelief HistoryAndSampleBelief::bel_a_;
HistoryAndSampleBelief HistoryAndSampleBelief::bel_ao_;

