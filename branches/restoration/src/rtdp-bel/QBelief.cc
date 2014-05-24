//  QBelief.h -- Quantization of standard beliefs
//
//  Blai Bonet, Hector Geffner (c)

#include "QBelief.h"
#include "Quantization.h"

using namespace std;

unsigned QBeliefHash::timestamp_ = 0;

pair<const Belief*, BeliefHash::Data> QBeliefHash::lookup(const Belief &belief, bool quantizied, bool insert) {
    if( quantizied ) {
        const QBelief &qbelief = static_cast<const QBelief&>(belief);
        const HashType::Entry *entry = HashType::lookup(qbelief);
        if( entry ) {
            return make_pair(&belief, entry->data_);
        } else {
            BeliefHash::Data data(0, false);
            if( insert ) {
                data.timestamp_ = timestamp_++;
                HashType::insert(qbelief, data);
            }
            return make_pair(&belief, data);
        }
    } else {
        const QBelief &qbelief = (*quantization_)(static_cast<const StandardBelief&>(belief));
        const HashType::Entry *entry = HashType::lookup(qbelief);
        if( entry ) {
            return make_pair(&qbelief, entry->data_);
        } else {
            BeliefHash::Data data(heuristic(static_cast<const StandardBelief&>(belief)), false);
            if( insert ) {
                data.timestamp_ = timestamp_++;
                HashType::insert(qbelief, data);
            }
            return make_pair(&qbelief, data);
        }
    }
}
 
