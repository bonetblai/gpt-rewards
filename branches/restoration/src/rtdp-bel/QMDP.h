//  QMDP.h -- The QMDP heuristic for beliefs
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _QMDP_INCLUDE_
#define _QMDP_INCLUDE_

#include "Heuristic.h"
#include "Problem.h"
#include "RtStandard.h"
#include "SB.h"
#include "QBelief.h"
#include "Quantization.h"
#include "Utils.h"

#include <iostream>

class Belief;
class StandardModel;

class QMDPHeuristic : public Heuristic {
  protected:
    const StandardModel *model_;
    double discount_;
    double *table_;
    int *action_table_;
    int size_;

  public:
    QMDPHeuristic(const StandardModel *model = 0, double discount = 1)
      : model_(model), discount_(discount), table_(0), size_(0) {
        compute();
    }
    virtual ~QMDPHeuristic() {
        delete[] table_;
        delete[] action_table_;
    }
    void compute();

    double value(const StandardBelief &belief) const {
        double sum = 0.0;
        for( StandardBelief::const_iterator it = belief.begin(); it != belief.end(); ++it ) {
            double val = value((*it).first);
            sum += (*it).second * val;
        }
        return sum;
    }
    double value(const QBelief &belief) const {
        throw(0);
        return 0;
    }

    virtual int action(int state) const {
        return action_table_[state];
    }
    virtual double value(int state) const {
        return table_[state];
    }
    virtual double value(const Belief &belief) const {
        const StandardBelief *sbel = dynamic_cast<const StandardBelief*>(&belief);
        if( sbel != 0 ) {
            return value(*sbel);
        } else {
            const QBelief *qbel = dynamic_cast<const QBelief*>(&belief);
            if( qbel != 0 )
                return value(*qbel);
            else
                return 0;
        }
    }
};

#endif // _QMDP_INCLUDE

