//  Theseus
//  Sondik.h -- Sondik's Method
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Sondik_INCLUDE_
#define _Sondik_INCLUDE_

#include <iostream>
#include <list>
#include <map>
#include <math.h>

#include "StandardModel.h"
#include "SB.h"
#include "QBelief.h"

static int timestampCmp(const void *p1, const void *p2);
static int updateCmp(const void *p1, const void *p2);

class Vector {
  public:
    int dim_;
    int action_;
    double *vec_;

    Vector(const Vector &v)
      : dim_(v.dim_), action_(v.action_), vec_(new double[dim_]) {
        *this = v;
    }
    Vector(int dim, int action = -1)
      : dim_(dim), action_(action), vec_(new double[dim_]) { }
    virtual ~Vector() { delete[] vec_; }

    const Vector& operator=(const Vector& v) {
        action_ = v.action_;
        for( int i = 0; i < dim_; ++i )
            vec_[i] = v.vec_[i];
        return *this;
    }

    double& operator[](int i) { return vec_[i]; }
    const double& operator[](int i) const { return vec_[i]; }

    const Vector& operator+=(const Vector &v) {
        for( int i = 0; i < dim_; ++i )
            vec_[i] += v.vec_[i];
        return *this;
    }
    const Vector& operator*=(double alpha) {
        for( int i = 0; i < dim_; ++i )
            vec_[i] *= alpha;
        return *this;
    }

    double operator*(const Vector &v) const {
        double sum = 0.0;
        for( int i = 0; i < dim_; ++i )
            sum += vec_[i]*v.vec_[i];
        return sum;
    }
    double operator*(const StandardBelief &b) const {
        double sum = 0.0;
        for( StandardBelief::const_iterator bi = b.begin(); bi != b.end(); ++bi )
            sum += (*bi).second * vec_[(*bi).first];
        return sum;
    }

    double l1_dist(const Vector &v) const {
        double dist = 0.0;
        for( int i = 0; i < dim_; ++i )
            dist += fabs(vec_[i] - v.vec_[i]);
        return dist;
    }
};

class VectorSet : public std::vector<const Vector*> {
  public:
    void clear() {
        for( size_t i = 0; i < size(); ++i )
            delete (*this)[i];
        std::vector<const Vector*>::clear();
    }
    bool insert(const Vector *alpha, double epsilon) {
        for( const_iterator it = begin(); it != end(); ++it ) {
            if( (*it)->l1_dist(*alpha) <= epsilon )
                return false;
        }
        push_back(alpha);
        return true;
    }
};

class BeliefSet : public std::list<const StandardBelief*> {
  public:
    void clear() {
        for( const_iterator it = begin(); it != end(); ++it )
            delete *it;
        std::list<const StandardBelief*>::clear();
    }
};

class Sondik {
  protected:
    const StandardPOMDP &pomdp_;
    const StandardModel &model_;
    int numStates_;
    int numActions_;
    int numObs_;

    BeliefSet beliefSet_;
    VectorSet gamma_;
    std::pair<double, int> *best_;

  public:
    Sondik(const StandardPOMDP &pomdp)
      : pomdp_(pomdp), model_(static_cast<const StandardModel&>(*pomdp.model())),
        numStates_(model_.numStates()), numActions_(model_.numActions()),
        numObs_(model_.numObs()), best_(new std::pair<double, int>[numObs_]) { }
    virtual ~Sondik() {
        //clear_belief_set();
        //clear_gamma();
        delete[] best_;
    }

    int numVectors() const { return gamma_.size(); }
    std::pair<double, int> value(const StandardBelief &b) const {
        int imin = -1;
        double min = DBL_MAX;
        for( int i = 0, sz = gamma_.size(); i < sz; ++i ) {
            double d = *gamma_[i] * b;
            if( d < min ) {
                min = d;
                imin = i;
            }
        }
        return std::make_pair(min, gamma_[imin]->action_);
    }

    const Vector* compute_gao(int action, int obs, int index) const {
        Vector *gao = new Vector(numStates_);
        for( int state = 0; state < numStates_; ++state ) {
            (*gao)[state] = 0.0;
            for( int i = 0, sz = model_.transition_[state*numActions_ + action]->size(); i < sz; ++i ) {
                int nstate = (*model_.transition_[state*numActions_ + action])[i].first;
                double prob = (*model_.transition_[state*numActions_ + action])[i].second;
                (*gao)[state] += model_.observation_[nstate*numActions_ + action][obs] * prob * (*gamma_[index])[nstate];
            }
        }
        return gao;
    }
    const Vector* compute_ga(int action, std::pair<double,int> *best) const {
        Vector *ga = new Vector(numStates_, action);
        for( int state = 0; state < numStates_; ++state )
            (*ga)[state] = model_.cost(state, action);
        for( int obs = 0; obs < numObs_; ++obs ) {
            const Vector *gao = compute_gao(action, obs, best[obs].second);
            *ga += *gao;
            delete gao;
        }
        return ga;
    }

    double compute_dot(const StandardBelief &ba, int action, int obs, int index) const {
        double dot_value = 0.0;
        for( StandardBelief::const_iterator bi = ba.begin(); bi != ba.end(); ++bi ) {
            int nstate = (*bi).first;
            double p = model_.observation_[nstate*numActions_ + action][obs] * (*gamma_[index])[nstate] * (*bi).second;
            dot_value += p;
        }
        return dot_value;
    }
    double compute_dot(const StandardBelief &ba, int action) const {
        double dot_value = 0.0;
        for( int obs = 0; obs < numObs_; ++obs ) {
            best_[obs] = std::make_pair(DBL_MAX, 0);
            for( int index = 0, sz = gamma_.size(); index < sz; ++index ) {
                double d = compute_dot(ba, action, obs, index);
                if( d < best_[obs].first ) {
                    best_[obs].first = d;
                    best_[obs].second = index;
                }
            }
            dot_value += best_[obs].first;
        }
        return dot_value;
    }

    const Vector* backup(const StandardBelief &b) const {
        // calculate best action
        std::pair<double, int> *best = new std::pair<double, int>[numObs_];
        double best_value = DBL_MAX;
        int best_action = 0;
        for( int action = 0; action < numActions_; ++action ) {
            const StandardBelief &ba = static_cast<const StandardBelief&>(b.update(&model_, action));
            double value = pomdp_.cost(b, action) + compute_dot(ba, action);
            if( value < best_value ) {
                best_value = value;
                best_action = action;
                bcopy(best_, best, sizeof(std::pair<double, int>)*numObs_);
            }
        }

        // calculate best vector
        const Vector *alpha = compute_ga(best_action, best);
        delete[] best;
        return alpha;
    }

#if 0
    void bootstrap(const QBeliefHash &hash) {
        beliefSet_.push_back(static_cast<const StandardBelief*>(model_.getInitialBelief()->clone()));
        double upper_bound = model_.minCost_ / (1.0 - model_.underlyingDiscount_);
        Vector *alpha = new Vector(numStates_);
        for( int state = 0; state < numStates_-1; ++state ) {
            if( state != model_.absorbing_ ) (*alpha)[state] = upper_bound;
        }
        (*alpha)[model_.absorbing_] = 0;
        gamma_.push_back(alpha);
    }
#endif

    void bootstrap(unsigned size, int method) {
        // insert beliefs
        beliefSet_.push_back(static_cast<const StandardBelief*>(model_.getInitialBelief()->clone()));

        const QBeliefHash &hash = static_cast<const QBeliefHash&>(*pomdp_.beliefHash());
        const Hash<const QBelief,BeliefHash::Data>::Entry **sortable = new const Hash<const QBelief, BeliefHash::Data>::Entry*[hash.nentries()];
        unsigned index = 0;
        for( QBeliefHash::const_iterator hi = hash.begin(); hi != hash.end(); ++hi ) {
            sortable[index++] = *hi;
        }
        assert(index == hash.nentries());
        qsort(&sortable[0], hash.nentries(), sizeof(void*), (method == 0 ? &timestampCmp : &updateCmp));
        for( unsigned i = 0; (i < size) && (i < hash.nentries()); ++i ) {
            //std::cout << "rank[" << i << "]=" << (method == 0 ? sortable[i]->data_.timestamp_ : sortable[i]->data_.updates_) << std::endl;
          const QBelief *qbelief = sortable[i]->key_;
          StandardBelief *belief = new StandardBelief;
          for( QBelief::const_iterator bi = qbelief->begin(); bi != qbelief->end(); ++bi ) {
              int state = (*bi) >> 8;
              int rank = (*bi) & 0xFF;
              belief->push_back(state, (double)rank);
          }
          belief->normalize();
          beliefSet_.push_back(belief);
        }
        delete[] sortable;

        // initial alpha vectors (just one)
        double upper_bound = model_.minCost_ / (1.0 - model_.underlyingDiscount_);
        Vector *alpha = new Vector(numStates_);
        for( int state = 0; state < numStates_-1; ++state ) {
            if( state != model_.absorbing_ ) (*alpha)[state] = upper_bound;
        }
        (*alpha)[model_.absorbing_] = 0;
        gamma_.push_back(alpha);
    }

    void update(double epsilon) {
        // perform point-based updates over all beliefs in beliefSet
        VectorSet ngamma;
        for( BeliefSet::const_iterator li = beliefSet_.begin(); li != beliefSet_.end(); ++li ) {
            const Vector *alpha = backup(**li);
            //if( !ngamma.insert(alpha, epsilon) ) delete alpha;
            ngamma.push_back(alpha);
        }
        gamma_.clear();
        gamma_ = ngamma;
    }
};

static int timestampCmp(const void *p1, const void *p2) {
    const Hash<const QBelief, BeliefHash::Data>::Entry **e1, **e2;
    e1 = reinterpret_cast<const Hash<const QBelief, BeliefHash::Data>::Entry**>((const void**)p1);
    e2 = reinterpret_cast<const Hash<const QBelief, BeliefHash::Data>::Entry**>((const void**)p2);
    int t1 = (*e1)->data_.timestamp_, t2 = (*e2)->data_.timestamp_;
    return t2 - t1;
}

static int updateCmp(const void *p1, const void *p2 ) {
    const Hash<const QBelief, BeliefHash::Data>::Entry **e1, **e2;
    e1 = reinterpret_cast<const Hash<const QBelief, BeliefHash::Data>::Entry**>((const void**)p1);
    e2 = reinterpret_cast<const Hash<const QBelief, BeliefHash::Data>::Entry**>((const void**)p2);
    int t1 = (*e1)->data_.updates_, t2 = (*e2)->data_.updates_;
    return t2 - t1;
}

#endif // _Sondik_INCLUDE

