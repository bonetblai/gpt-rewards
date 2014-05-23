//  Theseus
//  Problem.h -- Problem
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _Problem_INCLUDE_
#define _Problem_INCLUDE_

#include "Serialization.h"

#include <iostream>
#include <iomanip>
#include <vector>

class POMDP;
class Model;
class Belief;
class LookAheadHeuristic;
class Heuristic;
struct ProblemHandle;

#define ISCONFORMANT1(id) ((id)==ProblemHandle::PROBLEM_CONFORMANT1)
#define ISCONFORMANT2(id) ((id)==ProblemHandle::PROBLEM_CONFORMANT2)
#define ISCONFORMANT(id)  (ISCONFORMANT1(id) || ISCONFORMANT2(id))
#define ISPOMDP1(id)      ((id)==ProblemHandle::PROBLEM_POMDP1)
#define ISPOMDP2(id)      ((id)==ProblemHandle::PROBLEM_POMDP2)
#define ISPOMDP(id)       (ISPOMDP1(id) || ISPOMDP2(id))
#define ISNDPOMDP1(id)    ((id)==ProblemHandle::PROBLEM_ND_POMDP1)
#define ISNDPOMDP2(id)    ((id)==ProblemHandle::PROBLEM_ND_POMDP2)
#define ISNDPOMDP(id)     (ISNDPOMDP1(id) || ISNDPOMDP2(id))
#define ISMDP(id)         ((id)==ProblemHandle::PROBLEM_MDP)
#define ISNDMDP(id)       ((id)==ProblemHandle::PROBLEM_ND_MDP)
#define ISPLANNING(id)    ((id)==ProblemHandle::PROBLEM_PLANNING)

class Problem : public Serializable {
  public:
    enum CleanType { OBJECT, CORE, HASH };

  public:
    const char *softwareRevision_;
    const char *programName_;
    const char *problemFile_;

    const char *outputPrefix_;
    const char *outputFilename_;
    std::ostream *outputFile_;
    const char *coreFilename_;
    int outputLevel_;

    bool pddlProblem_;
    const char *linkmap_;
    int verboseLevel_;
    int precision_;
    int signal_;

    bool useStopRule_;
    double SREpsilon_;
    double epsilon_;
    double epsilonGreedy_;
    std::vector<std::pair<int, std::pair<int, int> > > pims_;
    bool maxUpdate_;
    int cutoff_;
    bool controlUpdates_;
    bool sondik_;
    int sondikMethod_;
    int sondikMaxPlanes_;
    int sondikIterations_;
    int qmethod_;
    double qlevels_;
    double qbase_;

    bool zeroHeuristic_;
    bool hashAll_;
    int lookahead_;
    double QMDPdiscount_;
    bool randomTies_;
    int randomSeed_;

    POMDP *pomdp_;
    const Model *model_;
    const Belief *belief_;
    const LookAheadHeuristic *heuristic_;
    const Heuristic *baseHeuristic_;
    const ProblemHandle *handle_;

  public:
    Problem();
    virtual ~Problem();

    void clean(CleanType type) {
        switch( type ) {
            case OBJECT:
                break;
            case CORE:
                break;
            case HASH:
                break;
        }
    }

    static const char* readArgument(const char *str, const char *ctx);
    void parseArguments(int argc, const char **argv, void (*helpFunction)());
    void print(std::ostream &os, const char *prefix) const;
    void setOutputFormat() const {
        if( outputFile_ )
            outputFile_->setf(std::ios::fixed | std::ios::showpoint);
        outputFile_->precision(precision_);
    }

    void bootstrapPDDL() { }
    void bootstrapCASSANDRA();
    void bootstrap(const char *workingDir, const char *entryPoint);

    void solvePDDL() { }
    void solveCASSANDRA();
    void solveProblem() {
        if( pddlProblem_ )
            solvePDDL();
        else
            solveCASSANDRA();
    }

    void freeHandle() {
        if( handle_ ) {
        }
    }
    void getHandle(const char *filename, const char *cwd, const char *entry) { }

    // serialization
    virtual void write(std::ostream &os) const { }
    static void read(std::istream &is, Problem &problem) { }
};

extern Problem PD;
extern void internalInitialization();
extern void internalFinalization();

#endif // _Problem_INCLUDE

