//  Theseus
//  Quantization.h -- Belief Quantization Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include <Quantization.h>

double Quantization::levels_ = 0;
double Quantization::base_ = 0;
double Quantization::log2rbase_ = 0;

static int
paircmp( const void *p1, const void *p2 )
{
  const std::pair<int,double> *q1 = static_cast<const std::pair<int,double>*>(p1);
  const std::pair<int,double> *q2 = static_cast<const std::pair<int,double>*>(p2);
  double delta = q2->second - q1->second;
  return(delta>0.0?1:(delta<0.0?-1:0));
}

inline float
fastlog2( float val )
{
  int *const exp_ptr = reinterpret_cast<int*>(&val);
  int x = *exp_ptr;
  const int log2 = ((x>>23)&255)-128;
  x &= ~(255<<23);
  x += 127<<23;
  *exp_ptr = x;
  val = ((-1.0f/3)*val+2)*val-2.0f/3;
  return(val+log2);
} 

inline float
fastlog( float val )
{
  return(fastlog2(val)*0.69314718f);
}

const QBelief&
Quantization::operator()( const StandardBelief &belief ) const
{
  qbelief_.clear();
  if( PD.qmethod_ == 0 ) { // Plain
    for( StandardBelief::const_iterator it = belief.begin(); it != belief.end(); ++it ) {
      double prob = (*it).second;
      if( prob > PD.epsilon_ ) {
        int rank = (int)ceil(levels_*prob);
        rank = (rank>255?255:rank);
        qbelief_.push_back((*it).first,rank);
      }
    }
  }
  else if( PD.qmethod_ == 1 ) { // Log
    for( StandardBelief::const_iterator it = belief.begin(); it != belief.end(); ++it ) {
      double prob = (*it).second;
      int rank = 0;
      if( prob < 0.5 ) {
        rank = (int)floor(fastlog2(2*prob)*log2rbase_);
        rank = (rank>127?127:rank);
      }
      else {
        rank = (int)floor(fastlog2(2*(1-prob))*log2rbase_);
        rank = (rank>127?127:rank);
        rank = 128+rank;
      }
      qbelief_.push_back((*it).first,rank);
    }
  }
  else if( PD.qmethod_ == 2 ) { // Freudenthal
    // compute X = invMatrixB times bel: worst-case O(n^2)
    bzero(X_,dim_*sizeof(double));
    for( StandardBelief::const_iterator it = belief.begin(); it != belief.end(); ++it ) {
      for( int i = 0; i <= (*it).first; ++i )
        X_[i] += (*it).second*levels_;
    }

    // find simplex S = (V^1, V^2, ..., V^n+1) containing X (sorted): Theta(n) + Theta(nlog(n))
    for( int i = 0; i < dim_; ++i ) {
      basevec_[i] = floor(X_[i]);
      delta_[i].first = i;
      delta_[i].second = X_[i]-basevec_[i];
    }
    qsort(delta_,dim_,sizeof(std::pair<int,double>),&paircmp);

    // compute barycentric coordinates of X inside S: Theta(n)
    lambda_[dim_] = delta_[dim_-1].second;
    lambda_[0] = 1.0-lambda_[dim_];
    double maxlambda = lambda_[dim_];
    int maxi = dim_;
    for( int i = dim_-1; i > 0; --i ) {
      lambda_[i] = delta_[i-1].second - delta_[i].second;
      lambda_[0] -= lambda_[i];
      if( lambda_[i] >= maxlambda ) {
        maxlambda = lambda_[i];
        maxi = i;
      }
    }
    if( lambda_[0] >= maxlambda ) maxi = 0;

#if 0
    // compute simplex corners: Theta(n^2)
    for( int j = 0; j <= dim_; ++j ) {
      bcopy(basevec_,bary_[j],dim_*sizeof(double));
      for( int i = j - 1; i >= 0; --i )
        bary_[j][delta_[i].first] += 1.0;
    }

    // remap S into [0,1]^n: Theta(n^2)
    for( int j = 0; j <= dim_; ++j ) {
      for( int i = 0; i < dim_-1; ++i )
        X_[i] = (bary_[j][i]-bary_[j][i+1])/levels_;
      X_[dim_-1] = bary_[j][dim_-1]/levels_;
      // X_ now contains one simplex corner
    }
#endif

    // copmute selected corner: Theta(n)
    bcopy(basevec_,corner_,dim_*sizeof(double));
    for( int i = maxi-1; i >= 0; --i ) {
      corner_[delta_[i].first] += 1.0;
    }

    // quantize selected corner: Theta(n)
    for( int i = 0; i < dim_; ++i ) {
      if( corner_[i] > 0 ) qbelief_.push_back(i,(int)ceil(corner_[i]));
    }
  }
  return(qbelief_);
}


