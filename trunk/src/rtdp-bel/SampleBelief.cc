//  SampleBelief.cc -- Implementation of belief states based on samples.
//  Blai Bonet, Hector Geffner (c)

#include "SampleBelief.h"

using namespace std;

int SampleBelief::num_particles_ = 0;
vector<int> SampleBelief::particles_;
vector<double> SampleBelief::weights_;
SampleBelief SampleBelief::bel_a_;
SampleBelief SampleBelief::bel_ao_;

int HistoryAndSampleBelief::num_particles_ = 0;
vector<int> HistoryAndSampleBelief::particles_;
vector<double> HistoryAndSampleBelief::weights_;
HistoryAndSampleBelief HistoryAndSampleBelief::bel_a_;
HistoryAndSampleBelief HistoryAndSampleBelief::bel_ao_;

