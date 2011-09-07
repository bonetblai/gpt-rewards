//  Theseus
//  Result.cc -- Result Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Result_INCLUDE_
#define _Result_INCLUDE_

#include <sys/time.h>
#include <list>
#include <RtStandard.h>
#include <Utils.h>

class Result {
public:
  int runType_;
  int numSteps_;
  double accCost_;
  double accDisCost_;
  double initialValue_;
  bool solved_;
  bool goalReached_;
  double startTime_;
  double stopTime_;

  Result() : runType_(0), numSteps_(0), accCost_(0), accDisCost_(0), initialValue_(0), solved_(false), goalReached_(true), startTime_(0), stopTime_(0) { }
  virtual ~Result() { }

  void startTimer() { startTime_ = getTime(); }
  void stopTimer() { stopTime_ = getTime(); }
  double elapsedTime() const { return(stopTime_-startTime_); }

  virtual void clean() = 0;
  virtual void print( std::ostream& os, int outputLevel, const ProblemHandle *handle ) = 0;
  virtual void push_back( int state, int action, int observation ) = 0;
  virtual void push_front( int state, int action, int observation ) = 0;
  virtual void pop_back() = 0;
  virtual void pop_front() = 0;
};

class StandardResult : public Result {
protected:
  struct Step {
    int state_;
    int action_;
    int observation_;
    Step( int state, int action, int observation ) : state_(state), action_(action), observation_(observation) { }
  };

  std::list<Step> steps_;

public:
  StandardResult() { }
  virtual ~StandardResult() { clean(); }
  virtual void clean() { startTime_ = 0; stopTime_ = 0; steps_.clear(); }
  virtual void push_back( int state, int action, int observation )
  {
    Step step(state,action,observation);
    steps_.push_back(step);
    ++numSteps_;
  }
  virtual void pop_back() { steps_.pop_back(); --numSteps_; }
  virtual void push_front( int state, int action, int observation )
  {
    Step step(state,action,observation);
    steps_.push_front(step);
    ++numSteps_;
  }
  virtual void pop_front() { steps_.pop_front(); --numSteps_; }

  virtual void print( std::ostream& os, int outputLevel, const ProblemHandle *handle )
  {
    if( outputLevel > 0 ) {
      os << "<e=" << elapsedTime() << ","
         << "t=" << (runType_==1?"l":"c") << ","
         << "v=" << initialValue_ << ","
         << "l=" << (solved_?"s":"u") << ","
         << "i=" << steps_.front().state_ << ","
         << "f=" << steps_.back().state_ << ","
         << "g=" << (goalReached_?1:0) << ","
         << "n=" << numSteps_;
    
      if( runType_ == 0 ) {
        os << ",c=" << accCost_ << ",d=" << accDisCost_;
        if( outputLevel > 1 ) {
          os << ",trajectory=[";
          if( numSteps_ != -1 ) {
            for( std::list<Step>::const_iterator it = steps_.begin(); it != steps_.end(); ) {
              os << "(";
              if( handle != 0 ) {
                (*handle->printAction)(os,(*it).action_);
                os << ",obs" << (*it).observation_;
              }
              else {
                os << "act" << (*it).action_ << ",obs" << (*it).observation_;
              }
              ++it;
              os << (it!=steps_.end()?"),":")");
            }
          }
          else
            os << "null";
          os << "]";
        }
      }
        os << ">" << std::endl;
    }
  }
};

#endif // _Result_INCLUDE

