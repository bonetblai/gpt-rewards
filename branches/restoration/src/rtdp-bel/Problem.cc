//  Theseus
//  Problem.cc -- Problem Implementation
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "LookAhead.h"
#include "Problem.h"
#include "QMDP.h"
#include "Result.h"
#include "StandardPOMDP.h"
#include "Sondik.h"
#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include <errno.h>
#include <iostream>
#include <fstream>

using namespace std;

long long glookups = 0, gfound = 0;

static const double tValues[] = // 101 values for student-t distribution
{ 12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228, 2.201, 2.179, 2.160, 2.145, 2.131,
   2.120, 2.110, 2.101, 2.093, 2.086, 2.080, 2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042,
   2.040, 2.037, 2.035, 2.032, 2.030, 2.028, 2.026, 2.024, 2.023, 2.021, 2.020, 2.018, 2.017, 2.015, 2.014,
   2.013, 2.012, 2.011, 2.010, 2.009, 2.008, 2.007, 2.006, 2.005, 2.004, 2.003, 2.002, 2.002, 2.001, 2.000,
   2.000, 1.999, 1.998, 1.998, 1.997, 1.997, 1.996, 1.995, 1.995, 1.994, 1.994, 1.993, 1.993, 1.993, 1.992,
   1.992, 1.991, 1.991, 1.990, 1.990, 1.990, 1.989, 1.989, 1.989, 1.988, 1.988, 1.988, 1.987, 1.987, 1.987,
   1.986, 1.986, 1.986, 1.986, 1.985, 1.985, 1.985, 1.984, 1.984, 1.984, 1.965, 1.962, 1.960 };
                                                                 // df =   500   1000    inf

Problem PD;

Problem::Problem()
  : softwareRevision_(REVISION), programName_(0), problemFile_(0), outputPrefix_(0),
    outputFilename_(0), outputFile_(&cout), coreFilename_(0), outputLevel_(0),
    pddlProblem_(false), linkmap_(0), verboseLevel_(0), precision_(6), signal_(-1),
    useStopRule_(false), SREpsilon_(0), epsilon_(.000001), epsilonGreedy_(0),
    maxUpdate_(false), cutoff_(100), controlUpdates_(false), sondik_(false),
    sondikMethod_(0), sondikMaxPlanes_(16), sondikIterations_(100), qmethod_(0),
    qlevels_(20), qbase_(0.95), zeroHeuristic_(false), hashAll_(false), lookahead_(0),
    QMDPdiscount_(1), randomTies_(true), randomSeed_(-1), pomdp_(0), model_(0),
    belief_(0), heuristic_(0), baseHeuristic_(0), handle_(0) {
}

Problem::~Problem() {
    delete belief_;
    delete pomdp_;
    delete model_;
    delete baseHeuristic_;
    delete heuristic_;
    StandardBelief::finalize();
}

const char* Problem::readArgument(const char *str, const char *ctx) {
    if( !str ) {
        cerr << "Fatal Error: argument expected after \"" << ctx << "\"" << endl;
        exit(-1);
    } else {
        return str;
    }
}

void Problem::parseArguments(int argc, const char **argv, void (*helpFunction)()) {
    // parse arguments
    ++argv;
    programName_ = *argv;
    while( (argc > 1) && (**argv == '-') ) {
        const char *ctx = *argv;
        if( !strcmp(ctx, "-random-seed") ) {
            randomSeed_ = atoi(*++argv);
            short unsigned seed[3];
            seed[0] = (short unsigned)randomSeed_;
            seed[1] = (short unsigned)randomSeed_;
            seed[2] = (short unsigned)randomSeed_;
            srand48((long)randomSeed_);
            seed48(seed);
            --argc;
        }
        --argc;
        ++argv;
    }

    // if more arguments, error
    if( argc > 2 ) {
        cerr << "usage: " << programName_ << " <options>* [<obj-file>]" << endl;
        exit(-1);
    }

    // What remains in argv is the name of a problem file
    if( *argv ) {
        cout << argc << " " << *argv << endl;
        char *pfile = new char[1 + strlen(*argv)];
        strcpy(pfile, *argv);
        problemFile_ = pfile;
    }

    // set random seed using time
    if( randomSeed_ == -1 ) {
        float time = getTime();
        int *ptr = reinterpret_cast<int*>(&time);
        randomSeed_ = *ptr;
    }
}

void Problem::print(ostream &os, const char *prefix) const {
    os << prefix << "problem \"" << problemFile_ << "\"" << endl
       << prefix << "pddlProblem " << (pddlProblem_ ? "true" : "false") << endl

       << prefix << "software-revision " << softwareRevision_ << endl
       << prefix << "version original (fixed)" << endl
       << prefix << "epsilon " << epsilon_ << endl
       << prefix << "pims";
    for( int pim = 0; pim < (int)pims_.size(); ++pim ) {
        os << " [" << pims_[pim].first << "," << pims_[pim].second.first << "," << pims_[pim].second.second << "]";
    }
    os << endl
       << prefix << "cutoff " << cutoff_ << endl
       << prefix << "qmdp-discount " << QMDPdiscount_ << endl
       << prefix << "heuristic-lookahead " << lookahead_ << endl
       << prefix << "zero-heuristic " << zeroHeuristic_ << endl
       << prefix << "hash-all " << (hashAll_ ? "on" : "off") << endl
       << prefix << "random-ties " << (randomTies_ ? "on" : "off") << endl
       << prefix << "random-seed " << randomSeed_ << endl
       << prefix << "verbose-level " << verboseLevel_ << endl
       << prefix << "precision " << precision_ << endl
       << prefix << "output-level " << outputLevel_ << endl
       << prefix << "max-update " << (maxUpdate_ ? "on" : "off") << endl
       << prefix << "stoprule ";
    if( useStopRule_ )
        os << SREpsilon_ << endl;
    else
        os << "off" << endl;
    os << prefix << "epsilon-greedy " << epsilonGreedy_ << endl
       << prefix << "control-updates " << (controlUpdates_ ? "on" : "off") << endl
       << prefix << "sondik " << (sondik_?"on":"off") << endl
       << prefix << "sondik-method " << (sondikMethod_ == 0 ? "timestamps" : "updates") << endl
       << prefix << "sondik-max-planes " << sondikMaxPlanes_ << endl
       << prefix << "sondik-iterations " << sondikIterations_ << endl
       << prefix << "qmethod " << (qmethod_ == 0 ? "plain" : (qmethod_ == 1 ? "log" : "freudenthal")) << endl
       << prefix << "qlevels " << qlevels_ << endl
       << prefix << "qbase " << qbase_ << endl;
}

#if 0
static void
classRegistration()
{
  // registration of all serializable classes

  // for POMDP
  StandardPOMDP::checkIn();
  StandardBeliefHash::checkIn();
  StandardBelief::checkIn();
  StandardModel::checkIn();
  QMDPHeuristic::checkIn();
  DynHeuristic::checkIn();
  LookAheadHeuristic::checkIn();

  // for MDP
  MDP::checkIn();
  SimpleBelief::checkIn();
  SimpleBeliefHash::checkIn();

  // for PLANNING
  PlanningPOMDP::checkIn();
  PlanningBeliefHash::checkIn();
  PlanningBelief::checkIn();
  PlanningModel::checkIn();

  // for CONFORMANT (search)
  SearchPOMDP::checkIn();
  SetBeliefHash::checkIn();
  SetBelief::checkIn();
  NonDetModel::checkIn();
}

static void
classCleanup()
{
  // registration of all serializable classes
  POMDP::cleanup();
  Belief::cleanup();
  Model::cleanup();
  Heuristic::cleanup();
  Hash::cleanup();
}

void
internalInitialization()
{
  // register all known classes
  classRegistration();

  // initialization
  //xxxx  memoryMgmt = new MemoryMgmt;
  setOutputFormat(*PD.outputFile);
}

void
internalFinalization()
{
  classCleanup();
  //xxxx  delete MemoryMgmt;
}

static int
bootstrapPLANNING( ProblemHandle *handle )
{
  unsigned long secs;

  gettimeofday(&t1,NULL);
  if( (handle->beliefHook != NULL) || (handle->modelHook != NULL) ) {
    cerr << "Error: model/belief hook not yet supported" << endl;
    return(-1);
  }
  else {
    // model creation & setup
    PD.model = new PlanningModel;
    PD.belief = new StandardBelief;
    PD.model->setup(handle,PD.belief->getConstructor());
    PD.model->discountFactor = 1.0;
    PD.belief->setModel(PD.model);
    PD.baseHeuristic = handle->heuristicHook;
    PD.heuristic = NULL;

    // POMDP creation & setup
    PD.pomdp = new PlanningPOMDP;
    PD.pomdp->setModel(PD.model);
    PD.pomdp->setParameters(PD.model->numActions,1,PD.cutoff,PD.randomTies,PD.model->discountFactor);
    PD.pomdp->setTheInitialBelief(PD.model->getTheInitialBelief());
  }

  // heuristic setup
  if( !PD.zeroHeuristic && !PD.baseHeuristic ) {
    PD.heuristic = new LookAheadHeuristic(PD.pomdp,PD.baseHeuristic);
    PD.heuristic->setLookahead(PD.lookahead);
    PD.belief->setHeuristic(PD.heuristic);
  }

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot setupTime " << (float)secs + (float)usecs / 1000000.0 << endl;
  return(0);
}

static int
bootstrapMDP( ProblemHandle *handle )
{
  StandardBelief *belief;

  int initialState;
  int action;
  map <int,float>::const_iterator it;
  const map <int,float> *initialBelief;

  gettimeofday(&t1,NULL);
  if( (handle->beliefHook != NULL) || (handle->modelHook != NULL) ) {
    cerr << "Error: model/belief hook not yet supported" << endl;
    return(-1);
  }
  else {
    // model creation
    PD.model = new StandardModel;
    PD.belief = new SimpleBelief;
    PD.baseHeuristic = handle->heuristicHook;
    PD.heuristic = NULL;

    // POMDP creation
    PD.pomdp = new MDP;
    PD.pomdp->setHash(new SimpleBeliefHash);
  }

  // model setup
  belief = new StandardBelief;
  PD.model->setup(handle,belief->getConstructor());
  PD.model->discountFactor = PD.discountFactor;
  PD.belief->setModel(PD.model);
  delete belief;

  // generate extra single initial state leading to the true initial states, 
  // setup model for the extra state
  initialState = PD.model->newState();
  action = 0;                       // the INIT action geenrated by the parser
  initialBelief = PD.model->theInitialBelief->cast();
  for( it = initialBelief->begin(); it != initialBelief->end(); ++it ) {
    PD.model->newTransition(initialState,action,(*it).first,(*it).second);
  }
  ((StandardModel*)PD.model)->setAvailability(initialState);

  // POMDP setup (no cache)
  PD.pomdp->setTheInitialBelief((Belief*)initialState);
  PD.pomdp->setModel(PD.model);
  PD.pomdp->setParameters(PD.model->numActions,1,PD.cutoff,PD.randomTies,PD.model->discountFactor);

  // heuristic
  if( PD.baseHeuristic != NULL ) {
    PD.heuristic = new LookAheadHeuristic(PD.pomdp,PD.baseHeuristic);
    PD.heuristic->setLookahead(PD.lookahead);
    PD.belief->setHeuristic(PD.heuristic);
  }

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot setupTime " << (float)secs + (float)usecs / 1000000.0 << endl;
  return(0);
}

static int
bootstrapPOMDP( ProblemHandle *handle )
{
  gettimeofday(&t1,NULL);
  if( handle->modelHook != NULL ) {
    // model creation
    PD.model = handle->modelHook;
    PD.belief = handle->beliefHook;
    PD.baseHeuristic = handle->heuristicHook;
    PD.heuristic = NULL;

    // POMDP creation
    PD.pomdp = new StandardPOMDP;
    //PD.pomdp->setHash(new HookedBeliefHash);
  }
  else if( handle->beliefHook != NULL ) {
    cerr << "Error: belief hook not yet supported" << endl;
    return(-1);
  }
  else {
    if( ISPOMDP(handle->problemType) ) {
      PD.model = new StandardModel;
      PD.baseHeuristic = handle->heuristicHook;
      PD.heuristic = NULL;
      PD.pomdp = new StandardPOMDP;
      PD.belief = new StandardBelief;
      PD.pomdp->setHash(new StandardBeliefHash);
    }
    else {
      PD.model = new NonDetModel;
      PD.baseHeuristic = handle->heuristicHook;
      PD.heuristic = NULL;
      PD.pomdp = new StandardPOMDP;
      PD.belief = new SetBelief;
      PD.pomdp->setHash(new SetBeliefHash);
    }
  }

  // model and belief setup
  PD.model->setup(handle,PD.belief->getConstructor());
  PD.model->discountFactor = PD.discountFactor;
  PD.belief->setModel(PD.model);

  // POMDP setup
  PD.pomdp->setTheInitialBelief(PD.model->getTheInitialBelief());
  if( ISPOMDP(handle->problemType) ) {
    PD.pomdp->setCache(PD.cacheSize[0],PD.clusterSize[0],PD.cacheSize[1],PD.clusterSize[1],
                        (Belief*(*)())&StandardBelief::constructor);
  }
  else {
    PD.pomdp->setCache(PD.cacheSize[0],PD.clusterSize[0],PD.cacheSize[1],PD.clusterSize[1],
                        (Belief*(*)())&SetBelief::constructor);
  }
  PD.pomdp->setModel(PD.model);
  PD.pomdp->setParameters(PD.model->numActions,1,PD.cutoff,PD.randomTies,PD.model->discountFactor);

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot setupTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  // compute model
  if( !PD.incrementalMode ) {
    try {
      ((StandardModel*)PD.model)->computeWholeModelFrom(PD.model->getTheInitialBelief());
    } catch( Exception& ) {
      PD.clean(Problem::CLEAN_OBJECT);
      throw;
    }
  }

  // timing
  gettimeofday(&t1,NULL);
  diffTime(secs,usecs,t2,t1);
  *PD.outputFile << "%boot computeModelTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  // heuristic setup
  if( !PD.zeroHeuristic ) {
    if( !PD.baseHeuristic ) {
      if( !PD.incrementalMode ) {
        try {
          PD.baseHeuristic = new QMDPHeuristic(PD.MDPdiscountFactor,PD.model->numActions,
                                                     ((StandardModel *)PD.model)->stateIndex,
                                                     (StandardPOMDP*)PD.pomdp);
        } catch( Exception& ) {
          PD.clean(Problem::CLEAN_OBJECT);
          throw;
        }
      }
      else if( ISPOMDP1(handle->problemType) || ISNDPOMDP1(handle->problemType) ) {
        PD.baseHeuristic = new DynHeuristic(PD.model);
      }
    }
    if( PD.baseHeuristic ) {
      PD.heuristic = new LookAheadHeuristic(PD.pomdp,PD.baseHeuristic);
      PD.heuristic->setLookahead(PD.lookahead);
    }
  }

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot computeHeuristicTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  if( PD.heuristic ) PD.belief->setHeuristic(PD.heuristic);

  // others setup
  StandardPOMDP::setup(((StandardModel *)PD.model)->stateIndex);
  //Quantization::initialize(PD.discretizationLevels,((StandardModel *)PD.model)->stateIndex);
  if( ISPOMDP(handle->problemType) ) {
    //Quantization::setCache(PD.cacheSize[2],PD.clusterSize[2], (Belief*(*)())&StandardBelief::constructor);
  }

  // check singal & end
  if( PD.signal >= 0 ) {
    int s = PD.signal;
    PD.signal = -1;
    throw(SignalException(s));
  }
  else
    return(0);
}

static int
bootstrapCONFORMANT( ProblemHandle *handle )
{
  gettimeofday(&t1,NULL);
  if( (handle->beliefHook != NULL) || (handle->modelHook != NULL) ) {
    cerr << "Error: model/belief hook not yet supported" << endl;
    return( -1 );
  }
  else {
    if( ISCONFORMANT2(handle->problemType) ) {
      PD.model = new StandardModel;
      PD.baseHeuristic = handle->heuristicHook;
      PD.heuristic = NULL;
      PD.pomdp = new SearchPOMDP;
      PD.belief = new StandardBelief;
      PD.pomdp->setHash( new StandardBeliefHash );
    }
    else {
      PD.model = new NonDetModel;
      PD.baseHeuristic = handle->heuristicHook;
      PD.heuristic = NULL;
      PD.pomdp = new SearchPOMDP;
      PD.belief = new SetBelief;
      PD.pomdp->setHash( new SetBeliefHash );
    }
  }

  // model and belief setup
  PD.model->setup( handle, PD.belief->getConstructor() );
  PD.model->discountFactor = PD.discountFactor;
  PD.belief->setModel( PD.model );

  // POMDP setup
  PD.pomdp->setTheInitialBelief( PD.model->getTheInitialBelief() );
  if( ISCONFORMANT2(handle->problemType) ) {
    PD.pomdp->setCache( PD.cacheSize[0], PD.clusterSize[0], PD.cacheSize[1], PD.clusterSize[1],
                        (Belief*(*)())&StandardBelief::constructor );
  }
  else {
    PD.pomdp->setCache( PD.cacheSize[0], PD.clusterSize[0], PD.cacheSize[1], PD.clusterSize[1],
                        (Belief*(*)())&SetBelief::constructor );
  }
  PD.pomdp->setModel( PD.model );
  PD.pomdp->setParameters( PD.model->numActions, 1, PD.cutoff, PD.randomTies, PD.model->discountFactor );

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot setupTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  // compute model
  if( !PD.incrementalMode ) {
    try {
      ((StandardModel*)PD.model)->computeWholeModelFrom( PD.model->getTheInitialBelief() );
    } catch( Exception& ) {
      PD.clean( Problem::CLEAN_OBJECT );
      throw;
    }
  }

  // timing
  gettimeofday(&t1,NULL);
  diffTime(secs,usecs,t2,t1);
  *PD.outputFile << "%boot computeModelTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  // heuristic setup
  if( !PD.zeroHeuristic ) {
    if( !PD.baseHeuristic ) {
      if( !PD.incrementalMode ) {
        try {
          PD.baseHeuristic = new QMDPHeuristic( PD.MDPdiscountFactor, PD.model->numActions,
                                                     ((StandardModel *)PD.model)->stateIndex,
                                                     (StandardPOMDP*)PD.pomdp );
        } catch( Exception& ) {
          PD.clean( Problem::CLEAN_OBJECT );
          throw;
        }
      }
      else {
        PD.baseHeuristic = new DynHeuristic( PD.model );
      }
    }

    if( PD.baseHeuristic ) {
      PD.heuristic = new LookAheadHeuristic( PD.pomdp, PD.baseHeuristic );
      PD.heuristic->setLookahead( PD.lookahead );
    }
  }

  // timing
  gettimeofday(&t2,NULL);
  diffTime(secs,usecs,t1,t2);
  *PD.outputFile << "%boot computeHeuristicTime " << (float)secs + (float)usecs / 1000000.0 << endl;

  if( !PD.zeroHeuristic && PD.heuristic ) PD.belief->setHeuristic( PD.heuristic );

  // check singal & end
  if( PD.signal >= 0 ) {
    int s = PD.signal;
    PD.signal = -1;
    throw( SignalException( s ) );
  }
  else
    return( 0 );
}
#endif

void Problem::bootstrapCASSANDRA() {
    // model creation: use setup for cassandra's format
    double time1 = getTime();
    model_ = new StandardModel(problemFile_);
    belief_ = new StandardBelief;

    // POMDP creation & setup
    pomdp_ = new StandardPOMDP(static_cast<const StandardModel*>(model_), qlevels_, qbase_);
    pomdp_->setEpsilonGreedy(epsilonGreedy_);
    pomdp_->setCutoff(cutoff_);
    StandardBelief::initialize(model_->numStates_);

    // timing
    double time2 = getTime();
    *outputFile_ << "%boot setupTime " << time2 - time1 << endl;
    time1 = time2;

    // heuristic setup
    if( !zeroHeuristic_ ) {
        baseHeuristic_ = new QMDPHeuristic(static_cast<const StandardModel*>(model_), QMDPdiscount_);
        heuristic_ = new LookAheadHeuristic(pomdp_, baseHeuristic_, lookahead_);
        pomdp_->setHeuristic(heuristic_);
    }

    // timing
    time2 = getTime();
    *outputFile_ << "%boot computeHeuristicTime " << time2 - time1 << endl;
    time1 = time2;
}

void Problem::bootstrap(const char *workingDir, const char *entryPoint) {
    clean(OBJECT);
    if( pddlProblem_ ) {
        // get handle and initialize problem
        //if( !(handle_ = getHandle(problemFile_, workingDir, entryPoint)) ) {
        //    return;
        //}
        (*handle_->initializeFunction)();

        switch( handle_->problemType ) {
            case ProblemHandle::PROBLEM_PLANNING:
                //PlanningBelief::initialize(handle_);
                //bootstrapPLANNING(handle_);
                break;
            case ProblemHandle::PROBLEM_MDP:
            case ProblemHandle::PROBLEM_ND_MDP:
                //bootstrapMDP(handle_);
                break;
            case ProblemHandle::PROBLEM_POMDP1:
            case ProblemHandle::PROBLEM_POMDP2:
            case ProblemHandle::PROBLEM_ND_POMDP1:
            case ProblemHandle::PROBLEM_ND_POMDP2:
                //bootstrapPOMDP(handle_);
                break;
            case ProblemHandle::PROBLEM_CONFORMANT1:
            case ProblemHandle::PROBLEM_CONFORMANT2:
                //bootstrapCONFORMANT(handle_);
                break;
            default:
                cerr << endl << "Error: undefined problem model" << endl;
        }
    } else {
        bootstrapCASSANDRA();
    }
}

void Problem::solveCASSANDRA() {
    Result *result = new StandardResult;
    for( int pim = 0; pim < (int)pims_.size(); ++pim ) { // perform pim step
        for( int i = 0; i < pims_[pim].first; ++i  ) {
            double lratio = -1, cratio = -1;

            // learning trials
            if( pims_[pim].second.first > 0 ) {
                int ntrials = pims_[pim].second.first;
                double ivalue = 0;
                for( int trial = 0; (trial < ntrials) || useStopRule_; ++trial ) {
                    try {
                        pomdp_->learnAlgorithm(*result);
                    } catch( Exception& ) {
                        result->clean();
                        delete result;
                        throw;
                    }
                    pomdp_->incLearningTime(result->elapsedTime());
                    result->print(*outputFile_, outputLevel_, handle_);
                    ivalue = result->initialValue_;
                    result->clean();
                }
                ivalue = (1+model_->maxReward_) / (1-model_->underlyingDiscount()) - ivalue;
                lratio = 100 * (double)gfound / (double)glookups;
                glookups = 0;
                gfound = 0;
                *outputFile_ << "%learningStats learningTime " << pomdp_->learningTime()
                             << " initialValue " << ivalue
                             << " hashRatio " << lratio
                             << endl;
            }

            Sondik *sondik = 0;
            double sondik_time = 0;
            if( sondik_ ) {
                double start_time = getTime();
                // For point-based operation need:
                //   1. collect last used beliefs
                //   2. transform collection of belief/values into Sondik's representation
                //   3. perform point-based updates over them
                sondik = new Sondik(static_cast<const StandardPOMDP&>(*pomdp_));
                sondik->bootstrap(sondikMaxPlanes_, sondikMethod_);
                for( int i = 0; i < sondikIterations_; ++i ) {
                    sondik->update(epsilon_);
                }
                sondik_time = getTime() - start_time;
            }
            pomdp_->incLearningTime(sondik_time);

            // control trials
            if( pims_[pim].second.second > 0 ) {
                int ntrials = pims_[pim].second.second;
                double ivalue = 0, ngoals = 0, totalSumDisCost = 0, totalSumDisCost2 = 0;
                for( int trial = 0; trial < ntrials; ++trial ) {
                    try {
                        pomdp_->controlAlgorithm(*result, sondik);
                    } catch( Exception& ) {
                        result->clean();
                        delete result;
                        throw;
                    }
                    pomdp_->incControlTime(result->elapsedTime());
                    result->print(*outputFile_, outputLevel_, handle_);
                    ivalue = result->initialValue_;
                    ngoals += result->goalReached_ ? 1 : 0;
                    totalSumDisCost += result->accDisCost_;
                    totalSumDisCost2 += result->accDisCost_ * result->accDisCost_;
                    result->clean();
                }
                ivalue = (1+model_->maxReward_) / (1-model_->underlyingDiscount()) - ivalue;
                double avg = totalSumDisCost / (double)ntrials, dev = 0, conf = 0, tv = 0;
                if( ntrials >= 2 ) {
                    dev = sqrt((totalSumDisCost2 / (ntrials-1)) - (totalSumDisCost*totalSumDisCost / (ntrials*(ntrials-1))));
                    if( ntrials <= 101 ) { // for degrees of freedom (df) <= 100 use exact t value
                        tv = tValues[ntrials-2];
                    } else if( (ntrials > 101) && (ntrials <= 501) ) { // for 100 < df <= 500, interpolate
                        tv = tValues[99] - (ntrials-101) * (tValues[99]-tValues[100]) / (double)(500-100);
                    } else if( (ntrials > 501) && (ntrials <= 1001) ) { // for 500 < df <= 1000, interpolate
                        tv = tValues[100] - (ntrials-501) * (tValues[100]-tValues[101]) / (double)(1000-500);
                    } else { // for df > 1000, use infinity value
                        tv = tValues[102];
                    }
                }
                conf = tv * dev / sqrt(ntrials);
                cratio = 100 * (double)gfound / (double)glookups;
                glookups = 0;
                gfound = 0;
                *outputFile_ << "%controlStats  learningTime " << pomdp_->learningTime()
                             << " initialValue " << ivalue;
                if( model_->numGoals() > 0 ) {
                    *outputFile_ << " goalRatio " << ngoals / (double)ntrials;
                }
                *outputFile_ << " rewardAvg " << avg
                             << " confInterval " << conf
                             << " hashRatio " << cratio;
                if( sondik_ ) {
                    *outputFile_ << " numplanes " << sondik->numVectors();
                }
                *outputFile_ << endl;
            }
            pomdp_->incLearningTime(-sondik_time);
        }
    }

    // check abortion
    if( signal_ >= 0 ) {
        int s = signal_;
        signal_ = -1;
        result->clean();
        delete result;
        throw(SignalException(s));
    }

    // statistics and clean
    pomdp_->statistics(*outputFile_);
    delete result;
}

