//  Theseus
//  History.cc -- History Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "History.h"

using namespace std;

History* History::pool_ = 0;

int History::num_bits_per_act_ = 0;
int History::num_act_per_wrd_ = 0;
unsigned History::act_mask_ = 0;

int History::num_bits_per_obs_ = 0;
int History::num_obs_per_wrd_ = 0;
unsigned History::obs_mask_ = 0;

