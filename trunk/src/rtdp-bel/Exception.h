//  Exceptions.h -- Exceptions 
//
//  Blai Bonet, Hector Geffner (c)

#ifndef _Exceptions_INCLUDE_
#define _Exceptions_INCLUDE_

#include "RtStandard.h"

#include <iostream>

class Exception {
  public:
    Exception() { }
    virtual ~Exception() { }
    virtual void print(std::ostream &os) = 0;
};

class SignalException : public Exception {
  public:
    int signal_;
    SignalException( int signal ) : signal_(signal) { }
    virtual ~SignalException() { }
    virtual void print(std::ostream &os) {
#ifdef SOLARIS
        char buff[SIG2STR_MAX];
        sig2str(signal_, buff);
        os << std::endl << "Error: interrupted by signal SIG" << buff << "(" << signal_ << ")." << std::endl;
#else
        os << std::endl << "Error: interrupted by signal SIG" << signal_ << "." << std::endl;
#endif
    }
};

class UnsupportedModelException : public Exception {
  public:
    int model_;
    UnsupportedModelException(int model) : model_(model) { }
    virtual ~UnsupportedModelException() { }
    virtual void print(std::ostream &os) {
        os << std::endl << "Error: unsupported model:";
        switch( model_ ) {
            case ProblemHandle::PROBLEM_PLANNING:
                os << "PLANNING";
                break;
            case ProblemHandle::PROBLEM_MDP:
                os << "MDP";
                break;
            case ProblemHandle::PROBLEM_ND_MDP:
                os << "NON-DETERMINISTIC MDP";
                break;
            case ProblemHandle::PROBLEM_POMDP1:
            case ProblemHandle::PROBLEM_POMDP2:
                os << "POMDP";
                break;
            case ProblemHandle::PROBLEM_ND_POMDP1:
            case ProblemHandle::PROBLEM_ND_POMDP2:
                os << "NON-DETERMINISTIC POMDP";
                break;
            case ProblemHandle::PROBLEM_CONFORMANT1:
            case ProblemHandle::PROBLEM_CONFORMANT2:
                os << "CONFORMANT";
                break;
        }
        os << std::endl;
    }
};

#endif // _Exceptions_INCLUDE

