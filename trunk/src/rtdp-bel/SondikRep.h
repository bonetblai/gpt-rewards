//  Theseus
//  SondikRep.h -- Sondik's Method
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _SondikRep_INCLUDE_
#define _SondikRep_INCLUDE_

#include <iostream>
#include <list>
#include <math.h>

#include <StandardModel.h>
#include <StandardBelief.h>
#include <QBelief.h>

class SondikRep {
protected:
  std::list<const QBelief*> candidates_;
  std::list<const StandardBelief*> beliefSet_;
  std::list<double*> *gamma_;
public:
  SondikRep() : gamma_(0) { }
  virtual ~SondikRep() { purge_set(); purge_gamma(); }

  void purge_set()
  {
    for( std::list<const StandardBelief*>::const_iterator li = beliefSet_->begin(); li != beliefSet_->end(); ++li )
      delete *li;
    beliefSet_.clear();
  }
  void purge_gamma()
  {
    for( std::list<double*>::const_iterator li = gamma_->begin(); li != gamma_->end(); ++li )
      delete[] *li;
    delete gamma_;
    gamma_ = 0;
  }

  double dot( const StandardBelief &b, double *alpha ) const
  {
    double sum = 0.0;
    for( StandardBelief::const_iterator bi = b.begin(); bi != b.end(); ++bi )
      sum += (*bi).second * alpha[(*bi).first];
    return(sum);
  }
  int value( const StandardBelief &b ) const
  {
    double min = DBL_MAX;
    for( std::list<double*>::const_iterator li = gamma_->begin(); li != gamma_->end(); ++li ) {
      double dotp = dot(b,*li);
      if( dotp < min ) min = dotp;
    }
    return(argmin);
  }

  void bootstrap( const BeliefHash &hash, int method )
  {
    // some stuff
  }
  void pb_update()
  {
    // some stuff
    std::list<double*> *tmp = 0, *ngamma = new std::list<double*>;
    for( std::list<const StandardBelief*>::const_iterator li = beliefSet_.begin(); li != beliefSet_.end(); ++li ) {
      //gamma_->push_back( alpha_b );
    }
    // some stuff
    purge_gamma();
    gamma_ = ngamma;
  }
  void expand( int method )
  {
    // some stuff
  }
};

#endif // _SondikRep_INCLUDE

