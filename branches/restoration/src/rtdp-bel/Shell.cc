//  Theseus
//  Shell.cc -- GPT main shell module
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#include "Exception.h"
#include "LookAhead.h"
#include "POMDP.h"
#include "Problem.h"
#include "StandardModel.h"
#include "Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>
#include <fstream>
#include <deque>
#include <map>

#define  PROMPT         "gpt> "
#define  RLC_COMMANDS   0
#define  RLC_FILES      1

#define HISTORY_FILE    "~/.gpt_history"
#define HISTORY_SIZE    1024

static struct
{
  const char *gpthome_;
  const char *cc_;
  const char *ccflags_;
  const char *ld_;
  const char *ldflags_;
  const char *include_;
  const char *lib_;
  const char *entry_;
} sys = { 0, 0, 0, 0, 0, 0, 0 };

static void  fillHelpDB();
static void  setAllDefaultValues();
static void  setDefaultValue( const char* var, bool silent );

class Command {
public:
  const char *name_;
  const char *pattern_;
  int subexprs_;
  regex_t regex_;
  int (*commandFunction_)(int,char**);
  Command( const char* name, const char* pattern, int subexprs, int (*commandFunction)(int,char**) )
    : name_(name), pattern_(pattern), subexprs_(subexprs), commandFunction_(commandFunction)
  {
    // compile the regular expression for command
    if( regcomp(&regex_,pattern_,REG_EXTENDED|REG_ICASE) )
      std::cerr << "Error: compiling pattern '" << pattern_ << "' for command '" << name_ << "'." << std::endl;
  }
  ~Command() { regfree(&regex_); }
};

static void commandRegistration();
static void commandCleanup();
static Command* commandParse( char *input, int &nargs, char** &args );

static void clearLineBuffer();
static void initializeReadline();
static void readlineCompletionMode( int mode );

// Globals
static char* currentProblem = 0;
static bool loadedObject = false;
static bool loadedCore = false;
static std::ostream *terminal = &std::cout;

///////////////////////////////////////////////////////////////////////////////
//
// Main Module
//

static void
help()
{
  std::cerr << "(under construction)" << std::endl;
}

static void
stripWhite( char* str )
{
  // strip prefix
  char *p = str;
  while( whitespace(*p) ) {
    for( char *t = p; *t; *t = *(t+1), ++t );
  }

  // strip others
  for( char *p = str; *p; ++p ) {
    while( whitespace(*p) && whitespace(*(p+1)) ) {
      for( char *t = p; *t; *t = *(t+1), ++t );
    }
  }

  // strip tail
  char *t = str + strlen(str) - 1;
  while( (t > str) && whitespace(*t) ) t--;
  *++t = '\0';
}

static void
evalLoop()
{
  // history initialization
  char *history_file = tilde_expand(HISTORY_FILE);
  using_history();
  read_history(history_file);
  stifle_history(HISTORY_SIZE);

  initializeReadline();
  readlineCompletionMode(RLC_COMMANDS);
  int rv = 1;
  char *line = 0;
  while( rv && (line = readline(PROMPT)) != 0 ) {
    stripWhite(line);
    if( line[0] != '\0' ) {
      int nargs = 0;
      char **args = 0;
      add_history(line);
      Command *cmdPtr = commandParse(line,nargs,args);
      if( cmdPtr != 0 ) {
        try {
          rv = (*cmdPtr->commandFunction_)(nargs,args);
        }
        catch( SignalException& e ) {
          e.print(*terminal);
        }
        catch( UnsupportedModelException& e ) {
          e.print(*terminal);
        }
        catch(...) {
          std::cerr << std::endl << "Error: unexpected exception." << std::endl;
        }

        // clean arguments
        if( nargs > 0 ) {
          for( int arg = 0; arg <= nargs; ++arg )
            if( args[arg] ) delete[] args[arg];
          delete[] args;
        }
      }
      else {
        *terminal << "Unrecognized command '" << line << "'." << std::endl;
      }
    }
    free(line);
  }

  // write history
  write_history(history_file);
  free(history_file);
}

void
setCurrentProblem( const char* str )
{
  char *buff = 0;

  if( (!str && !currentProblem) || (str && (!currentProblem || strcmp(str,currentProblem))) ) {
    if( !str && !currentProblem ) {
      readlineCompletionMode(RLC_FILES);
      buff = readline("Input problem: ");
      while( buff ) {
        stripWhite(buff);
        if( buff[0] != '\0' ) break;
        free(buff);
      }
      readlineCompletionMode(RLC_COMMANDS);
    }
    else { // clean
      if( loadedObject ) {
        PD.clean(Problem::OBJECT);
        PD.freeHandle();
      }
      buff = strdup(str);
    }

    // set new problem
    delete[] currentProblem;
    delete[] PD.problemFile_;
    currentProblem = new char[1+strlen(buff)];
    char *tmp = new char[3+strlen(buff)];
    strcpy(currentProblem,buff);
    if( PD.pddlProblem_ )
      sprintf(tmp,"%s.o",buff);
    else
      strcpy(tmp,buff);
    PD.problemFile_ = tmp;
    free(buff);
    loadedObject = false;
    loadedCore = false;
  }
}

static int
bootstrap()
{
  char *cwd = getcwd(0,1024);
  *terminal << "Bootstraping... " << std::endl;
  try {
    PD.bootstrap(cwd,sys.entry_);
  }
  catch( Exception& ) {
    *terminal << "Bootstraping: Exception!" << std::endl;
    delete[] currentProblem;
    delete[] PD.problemFile_;
    currentProblem = 0;
    PD.problemFile_ = 0;
    free(cwd);
    return(0);
  }
  loadedObject = true;
  loadedCore = false;
  free(cwd);
  return(1);
}

static int
readcore( const char* coreFilename )
{
  *terminal << "Reading core... " << std::flush;
  //if( readCore(coreFilename) ) return(0);
  *terminal << "done!" << std::endl;
  loadedCore = true;
  return(0);
}

void
signalHandler( int s )
{
  if( RL_ISSTATE(RL_STATE_READCMD) ) {
    rl_crlf();
    rl_on_new_line();
    clearLineBuffer();
    rl_redisplay();
  }
  PD.signal_ = s;
  signal(s,signalHandler);
}

int
main( int argc, const char **argv )
{
#if 0
  // get GPTHOME from env
  if( !(sys.gpthome_ = getenv("GPTHOME")) ) {
    std::cerr << "Fatal Error: need to set variable GPTHOME." << std::endl;
    exit(-1);
  }
#endif

  // parse arguments & initialization
  signal(SIGINT,signalHandler);
  setAllDefaultValues();
  fillHelpDB();
  setDefaultValue(0,true);
  PD.parseArguments(argc,argv,&help);
  //internalInitialization();
  commandRegistration();
  *terminal << "Welcome to GPT, Version " << PD.softwareRevision_ << "." << std::endl;

  // bootstrap problem
  if( PD.problemFile_ ) {
    const char *str = PD.problemFile_;
    PD.problemFile_ = 0;
    setCurrentProblem(str);
    bootstrap();
  }

  // eval loop
  evalLoop();

  // cleanup
  delete[] sys.gpthome_;
  delete[] sys.cc_;
  delete[] sys.ccflags_;
  delete[] sys.ld_;
  delete[] sys.ldflags_;
  delete[] sys.include_;
  delete[] sys.lib_;
  delete[] sys.entry_;
  delete[] currentProblem;
  delete[] PD.problemFile_;
  delete[] PD.linkmap_;
  commandCleanup();
  //internalFinalization();

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////
//
// Default Values
//

struct stringCompareFunction {
  bool operator()( const char* s1, const char* s2 ) const { return(strcmp(s1,s2)<0); }
};

struct stringCaseCompareFunction {
  bool operator()( const char* s1, const char* s2 ) const { return(strcasecmp(s1,s2)<0); }
};

static std::map<const char*,const char*,stringCaseCompareFunction> defaultValues;

static void
setAllDefaultValues()
{
  defaultValues.insert( std::make_pair((const char*)"stoprule",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"epsilon",(const char*)"0.000001") );
  defaultValues.insert( std::make_pair((const char*)"epsilon-greedy",(const char*)"0.0") );
  defaultValues.insert( std::make_pair((const char*)"max-update",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"cutoff",(const char*)"250") );
  defaultValues.insert( std::make_pair((const char*)"control-updates",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"sondik",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"sondik-method",(const char*)"timestamps") );
  defaultValues.insert( std::make_pair((const char*)"sondik-max-planes",(const char*)"16") );
  defaultValues.insert( std::make_pair((const char*)"sondik-iterations",(const char*)"100") );
  defaultValues.insert( std::make_pair((const char*)"pims",(const char*)"") );
  defaultValues.insert( std::make_pair((const char*)"qmethod",(const char*)"plain") );
  defaultValues.insert( std::make_pair((const char*)"qlevels",(const char*)"15") );
  defaultValues.insert( std::make_pair((const char*)"qbase",(const char*)"0.95") );
  defaultValues.insert( std::make_pair((const char*)"qmdp-discount",(const char*)"1.00") );
  defaultValues.insert( std::make_pair((const char*)"heuristic-lookahead",(const char*)"0") );
  defaultValues.insert( std::make_pair((const char*)"zero-heuristic",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"hash-all",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"random-ties",(const char*)"off") );
  defaultValues.insert( std::make_pair((const char*)"random-seed",(const char*)"0") );
  defaultValues.insert( std::make_pair((const char*)"output-level",(const char*)"0") );
  defaultValues.insert( std::make_pair((const char*)"verbose-level",(const char*)"0") );
  defaultValues.insert( std::make_pair((const char*)"precision",(const char*)"6") );
  defaultValues.insert( std::make_pair((const char*)"cc",(const char*)"g++") );
  defaultValues.insert( std::make_pair((const char*)"ccflags",(const char*)"") );
  defaultValues.insert( std::make_pair((const char*)"ld",(const char*)"g++") );
  defaultValues.insert( std::make_pair((const char*)"ldflags",(const char*)"-shared") );
  defaultValues.insert( std::make_pair((const char*)"include-dir",(const char*)"$GPTHOME/include:.") );
  defaultValues.insert( std::make_pair((const char*)"lib-dir",(const char*)"$GPTHOME/lib") );
  defaultValues.insert( std::make_pair((const char*)"entry-point",(const char*)"problemRegister") );
}

static std::map<const char*,const char*,stringCaseCompareFunction> helpDB;
static void
fillHelpDB()
{
#define HELP_MSG(command,msg)     helpDB[command] = msg;
#include "HelpMsg.dat"
#undef HELP_MSG
}

///////////////////////////////////////////////////////////////////////////////
//
// Commands Functions
//

static const char* aboutPattern  = "about";
static int aboutSubexprs = 0;
static int aboutFunction( int nargs, char** args )
{
  *terminal << "(under construction)" << std::endl;
  return(1);
}

static const char* helpPattern  = "help( (.*)){0,1}";
static int helpSubexprs = 2;
static int helpFunction( int nargs, char** args )
{
  if( args[2] != 0 ) {
    const char *helpMsg = helpDB[args[2]];
    if( helpMsg )
      *terminal << helpMsg << std::endl;
    else
      *terminal << "Unrecognized help command." << std::endl;
  }
  else {
    *terminal << helpDB["help"] << std::endl;
  }
  return(1);
}

static const char* clearPattern  = "clear";
static int clearSubexprs = 0;
static int clearFunction( int nargs, char **args  )
{
  rl_clear_screen(0,0);
  rl_crlf();
  rl_on_new_line();
  clearLineBuffer();
  rl_redisplay();
  return(1);
}

static const char* cdPattern  = "cd( ([[:print:]]+)){0,1}";
static int cdSubexprs = 2;
static int cdFunction( int nargs, char**  args )
{
  if( args[1] ) {
    if( chdir(args[2]) )
      std::cerr << "Error: " << args[2] << ": " << strerror(errno) << "." << std::endl;
  }
  else {
    *terminal << getcwd(0,0) << std::endl;
  }
  return(1);
}

static const char* quitPattern  = "quit|exit";
static int quitSubexprs = 0;
static int quitFunction( int nargs, char **args  )
{
  PD.clean(Problem::OBJECT);
  PD.freeHandle();
  *terminal << "Bye." << std::endl;
  return(0);
}

static const char* parsePattern  = "parse( ([[:alnum:]_]+)){0,1}( ([[:print:]]+)){0,1}";
static int parseSubexprs = 4;
static int parseFunction( int nargs, char** args )
{
  if( !args[1] || !args[3] ) {
    *terminal << "Not enough arguments. Use 'parse <problem-name> <pddl-file>+'." << std::endl;
    return(1);
  }

  char *cmdline = new char[15+strlen(sys.gpthome_)+strlen(args[2])+strlen(args[4])];
  sprintf(cmdline,"%s/bin/parser %s %s",sys.gpthome_,args[2],args[4]);

  // parse
  double time1 = getTime();
  *terminal << "-------" << std::endl << "$GPTHOME/bin/parser " << args[2] << " " << args[4] << std::endl;
  int rv = system(cmdline);
  *terminal << "-------" << std::endl;
  if( !rv ) { // print elapsed time
    double time2 = getTime();
    *terminal << time2-time1 << " seconds." << std::endl;
  }

  delete[] cmdline;
  return(1);
}

static const char* printPattern  = "print( (action|action@|belief|observation|state)( (.+)){0,1}){0,1}";
static int printSubexprs = 4;
static int printFunction( int nargs, char** args )
{
  if( !PD.pomdp_ || !PD.model_ ) {
    *terminal << "No model available." << std::endl;
    return(1);
  }

  if( !strcasecmp(args[2],"action") ) {
    if( !args[4] )
      *terminal << "Not enough arguments. Use 'print action <action-id>'." << std::endl;
    else {
      int action = atoi(args[4]);
      if( (action < 0) || (action >= PD.pomdp_->numActions()) )
        *terminal << "Invalid action." << std::endl;
      else if( PD.pddlProblem_ ) {
        (*PD.handle_->printAction)(*terminal,action);
        *terminal << std::endl;
      }
      else
        *terminal << "Name not available." << std::endl;
    }
  }
  else if( !strcasecmp(args[2],"action@") ) {
    if( !args[4] )
      *terminal << "Not enough arguments. Use 'print action@ <belief-id>'." << std::endl;
    else if( ISCONFORMANT(PD.handle_->problemType) || ISPLANNING(PD.handle_->problemType) ) {
      *terminal << "Print request not supported in CONFORMANT/PLANNING models." << std::endl;
    }
    else {
      void *ptr = 0;
      sscanf(args[4],"0x%p",&ptr);
#if 0
      const HashEntry *entry = (const HashEntry*)ptr;
      if( !entry || (entry != PD.pomdp_->getHashEntry((const Belief*)entry->getData())) )
        *terminal << "Invalid belief." << std::endl;
      else {
        int action = PD.pomdp_->getBestAction(entry);
        if( PD.pddlProblem_ ) {
          *terminal << "action " << action << " = ";
          (*PD.handle_->printAction)(*terminal,action);
          *terminal << std::endl;
        }
        else
          *terminal << "action " << action << std::endl;
        }
      }
#endif
    }
  }
  else if( !strcasecmp(args[2],"belief") ) {
    if( !args[4] ) {
      *terminal << "Not enough arguments. "
                << "Use 'print belief [<belief-id>|initial|actions <belief-id>|afteraction <belief-id> <action-id>]'."
                << std::endl;
    }
    else if( ISCONFORMANT(PD.handle_->problemType) || ISPLANNING(PD.handle_->problemType) ) {
      *terminal << "Print request not supported in CONFORMANT/PLANNING models." << std::endl;
    }
    else if( !strcasecmp(args[4],"initial") ) {
#if 0
      const HashEntry *entry = PD.pomdp_->getTheInitialBelief();
      int action = PD.pomdp_->getBestAction(entry);
      if( ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType) )
        ((const Belief*)entry->getData())->checkModelAvailability(PD.model_);
      double hvalue = (!PD.heuristic_?0.0:PD.heuristic_->value((const Belief*)entry->getData()));
      *terminal << "belief-id = " << entry << std::endl
                << "value     = " << entry->getValue() << std::endl
                << "heuristic = " << hvalue << std::endl
                << "goal      = " << PD.pomdp_->isGoal((const Belief*)entry->getData()) << std::endl
                << "solved    = " << (int)entry->getExtra() << std::endl
                << "action    = " << action;
      if( PD.pddlProblem_ ) {
        *terminal << "=";
        (*PD.handle_->printAction)(*terminal,action);
      }
      *terminal << std::endl;
      if( ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType) )
        *terminal << "belief    = " << *(const Belief*)entry->getData() << std::endl;
      else if( ISMDP(PD.handle_->problemType) || ISNDMDP(PD.handle_->problemType) )
        *terminal << "state     = " << (int)entry->getData() << std::endl;
#endif
    }
    else if( !strncasecmp( args[4],"actions",7) ) {
      char *str = 0, *tok = 0;
      str = tok = strdup(&args[4][7]);
      tok = strtok(tok," ");
      if( tok == 0 ) {
        *terminal << "Not enough arguments. Use 'print belief actions <belief-id>'." << std::endl;
      }
      else {
        void *ptr = 0;
        sscanf(tok,"0x%p",&ptr);
#if 0
        const HashEntry *entry = (const HashEntry*)ptr;
        if( !entry || (entry != PD.pomdp_->getHashEntry((const Belief*)entry->getData())) )
          *terminal << "Invalid belief id." << std::endl;
        else {
          if( ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType) )
            ((const Belief*)entry->getData())->checkModelAvailability(PD.model_);
          *terminal << "actions = [ ";
          for( int action = 0; action < PD.pomdp_->numActions(); ++action ) {
            if( PD.pomdp_->applicable((const Belief*)entry->getData(),action) ) {
              *terminal << action;
              if( PD.pddlProblem_ ) {
                *terminal << "=";
                (*PD.handle_->printAction)(*terminal,action);
              }
              *terminal << ", " << std::endl;
            }
          }
          *terminal << "]" << std::endl;
        }
#endif
      }
      free(str);
    }
    else if( !strncasecmp(args[4],"afteraction",11) ) {
      char *str = 0, *tok = 0;
      str = tok = strdup(&args[4][11]);
      tok = strtok(tok," ");
      if( tok == 0 ) {
        err:
          *terminal << "Not enough arguments. Use 'print belief afteraction <belief-id> <action-id>'." << std::endl;
      }
      else {
        void *ptr = 0;
        int action = 0;
        sscanf(tok,"0x%p",&ptr);
        tok = strtok(0," ");
        if( tok == 0 ) goto err;
        sscanf(tok,"%d",&action);
#if 0
        const HashEntry *entry = (const HashEntry*)ptr;
        if( !entry || (entry != PD.pomdp_->getHashEntry((const Belief*)entry->getData())) )
          *terminal << "Invalid belief." << std::endl;
        else if( (action < 0) || (action >= PD.pomdp_->numActions()) )
          *terminal << "Invalid action." << std::endl;
        else if( !PD.pomdp_->applicable((const Belief*)entry->getData(),action) )
          *terminal << "Action not applicable in belief." << std::endl;
        else { // we are ok ..
          std::deque<std::pair<std::pair<int,double>,const HashEntry*> > expansion;
          std::deque<std::pair<std::pair<int,double>,const HashEntry*> >::const_iterator it;
          PD.pomdp_->expandBeliefWithAction(entry,action,expansion);
          *terminal << "cost = " <<PD.pomdp_->cost((const Belief*)entry->getData(),action)<<std::endl;
          for( it = expansion.begin(); it != expansion.end(); ++it ) {
            *terminal << "obs-id = " << (*it).first.first
                      << ", prob. = " << (*it).first.second
                      << ", belief-id = " << (*it).second << std::endl;
          }
        }
#endif
      }
      free(str);
    }
    else {
      void *ptr = 0;
      sscanf(args[4],"0x%p",&ptr);
#if 0
      const HashEntry *entry = (const HashEntry*)ptr;
      if( !entry || (entry != PD.pomdp_->getHashEntry((const Belief*)entry->getData())) )
        *terminal << "Invalid belief id." << std::endl;
      else {
        int action = PD.pomdp_->getBestAction(entry);
        if( ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType) )
          ((const Belief*)entry->getData())->checkModelAvailability(PD.model_);
        double hvalue = (!PD.heuristic_?0.0:PD.heuristic_->value((const Belief*)entry->getData()));
        *terminal << "belief-id = " << entry << std::endl
                  << "value     = " << entry->getValue() << std::endl
                  << "heuristic = " << hvalue << std::endl
                  << "goal      = " << PD.pomdp_->isGoal((const Belief*)entry->getData()) << std::endl
                  << "solved    = " << (int)entry->getExtra() << std::endl
                  << "action    = " << action;
        if( PD.pddlProblem_ ) {
          *terminal << "=";
          (*PD.handle_->printAction)(*terminal,action);
        }
        *terminal << std::endl;
        if( ISPOMDP(PD.handle_->problemType) || ISNDPOMDP(PD.handle_->problemType) )
          *terminal << "belief    = " << *(const Belief*)entry->getData() << std::endl;
        else if( ISMDP(PD.handle_->problemType) || ISNDMDP(PD.handle_->problemType) )
          *terminal << "state     = " << (int)entry->getData() << std::endl;
      }
#endif
    }
  }
  else if( !strcasecmp(args[2],"observation") ) {
    if( !args[4] )
      *terminal << "Not enough arguments. Use 'print observation <obs-id>'." << std::endl;
    else {
#if 0
    int obs = atoi(args[4]);
    PD.model_->printObservation(obs,*terminal,0);
#endif
    }
  }
  else if( !strcasecmp(args[2],"state") ) {
    if( !args[4] ) {
      *terminal << "Not enough arguments. "
                << "Use 'print state [<state-id>|actions <state-id>|afteraction <state-id> <action-id>]'." << std::endl;
    }
    else if( !strncasecmp(args[4],"actions",7) ) {
      char *str = 0, *tok = 0;
      str = tok = strdup(&args[4][7]);
      tok = strtok(tok," ");
      if( tok == 0 ) {
        *terminal << "Not enough arguments. Use 'print state actions <state-id>'." << std::endl;
      }
      else {
        int state = atoi(tok);
        *terminal << "actions = [ ";
        for( int action = 0; action < PD.pomdp_->numActions(); ++action ) {
          if( PD.model_->applicable(state,action) ) {
            *terminal << action;
            if( PD.pddlProblem_ ) {
              *terminal << "=";
              (*PD.handle_->printAction)(*terminal,action);
            }
            *terminal << ", " << std::endl;
          }
        }
        *terminal << "]" << std::endl;
      }
      free(str);
    }
    else if( !strncasecmp(args[4],"afteraction",11) ) {
      char *str = 0, *tok = 0;
      str = tok = strdup(&args[4][11]);
      tok = strtok(tok," ");
      if( tok == 0 ) {
        err2:
          *terminal << "Not enough arguments. Use 'print state afteraction <belief-id> <action-id>'." << std::endl;
      }
      else {
        int state = atoi(tok);
        tok = strtok(0," ");
        if( tok == 0 ) goto err2;
        int action = atoi(tok);
        if( (action < 0) || (action >= PD.pomdp_->numActions()) )
          *terminal << "Invalid action." << std::endl;
        else if( !PD.model_->applicable(state,action) )
          *terminal << "Action not applicable in state." << std::endl;
        else { // we are ok ..
#if 0
          int state = PD.model_->nextState(state,action);
          PD.model_->checkModelFor(state);
          double hvalue = (!PD.heuristic_?0.0:PD.heuristic_->value(state));
          *terminal << "state     = " << state << endl
                    << "goal      = " << PD.model_->isGoal(state) << std::endl
                    << "heuristic = " << hvalue << std::endl
                    << "state:" << std::endl;
          PD.model_->printState(state,*terminal,2);
#endif
        }
      }
      free(str);
    }
    else {
      int state = atoi(args[4]);
      //PD.model_->checkModelFor(state);
      double hvalue = (!PD.heuristic_?0.0:PD.heuristic_->value(state));
      *terminal << "goal      = " << PD.model_->isGoal(state) << std::endl
                << "heuristic = " << hvalue << std::endl
                << "state:" << std::endl;
      //PD.model_->printState(state,*terminal,2);
    }
  }
  return(1);
}

static const char* linkmapPattern  = "linkmap( (\\+|-)){0,1}( ([[:print:]]+)){0,1}";
static int linkmapSubexprs = 4;
static int linkmapFunction( int nargs, char** args )
{
  if( args[4] ) {
    if( !args[2] || (args[2][0] == '+') ) {
      char *tmp = new char[2+(PD.linkmap_?strlen(PD.linkmap_):0)+strlen(args[4])];
      sprintf(tmp,"%s%s%s",(PD.linkmap_?PD.linkmap_:""),(PD.linkmap_?" ":""),args[4]);
      delete[] PD.linkmap_;
      PD.linkmap_ = tmp;
    }
    else {
      char *tmp = strdup(PD.linkmap_);
      delete[] PD.linkmap_;
      PD.linkmap_ = 0;
      for( char *ptr = strtok(tmp," "); ptr; ptr = strtok(0," ") ) {
        if( strcmp(ptr,args[4]) ) {
          char *ptr2 = new char[2+(PD.linkmap_?strlen(PD.linkmap_):0)+strlen(ptr)];
          sprintf(ptr2,"%s%s%s",(PD.linkmap_?PD.linkmap_:""),(PD.linkmap_?" ":""),ptr);
          delete[] PD.linkmap_;
          PD.linkmap_ = ptr2;
        }
      }
      free(tmp);
    }
  }
  *terminal << "linkmap: " << PD.linkmap_ << std::endl;
  return(1);
}

static const char* loadPattern  = "load (core|problem)( ([[:print:]]+)){0,1}";
static int loadSubexprs = 3;
static int loadFunction( int nargs, char** args )
{
  if( !strcasecmp(args[1],"core") ) {
    // read problem name and core name (if necessary)
    char *coreFilename = 0;
    setCurrentProblem(0);

    // set corefile
    readlineCompletionMode(RLC_FILES);
    char *tmp = readline("Input corefile: ");
    while( tmp ) {
      stripWhite(tmp);
      if( tmp[0] != '\0' ) break;
      free(tmp);
    }
    readlineCompletionMode(RLC_COMMANDS );
    coreFilename = new char[1+strlen(tmp)];
    strcpy(coreFilename,tmp);
    free(tmp);

    // bootstrap problem
    if( !loadedObject && !bootstrap() ) {
      delete[] coreFilename;
      return(1);
    }

    // load core
    readcore(coreFilename);
    delete[] coreFilename;
  }
  else if( !strcasecmp(args[1],"problem") ) {
    setCurrentProblem(0);
    bootstrap();
  }
  return(1);
}

#define COMPILE_CMD "make -f Makefile.%s CC=\"%s\" LD=\"%s\" CCFLAGS=\"%s\" LDFLAGS=\"%s\" INCLUDE=\"%s\" LIB=\"%s\" LINKMAP=\"%s\""
static const char* compilePattern  = "compile( ([[:alnum:]_]+)){0,1}";
static int compileSubexprs = 2;
static int compileFunction( int nargs, char** args )
{
  setCurrentProblem(args[2]);

  // generate cmdline
  char *include = 0;
  if( sys.include_ ) {
    char *tmp = strdup(sys.include_);
    for( char *ptr = strtok(tmp,":"); ptr; ptr = strtok(0,":") ) {
      char *ptr2 = new char[4+(include?strlen(include):0)+strlen(ptr)];
      sprintf(ptr2,"%s%s-I%s",(include?include:""),(include?" ":""),ptr);
      delete[] include;
      include = ptr2;
    }
    free(tmp);
  }
  else
    include = (char*)"";

  char *cmdline = new char[1+strlen(COMPILE_CMD)+strlen(currentProblem)+strlen(sys.cc_)+
		           strlen(sys.ld_)+strlen(sys.ccflags_)+strlen(sys.ldflags_)+ 
		           strlen(include)+strlen(sys.lib_)+(PD.linkmap_?strlen(PD.linkmap_):0)];
  sprintf(cmdline,COMPILE_CMD,currentProblem,sys.cc_,sys.ld_,sys.ccflags_,sys.ldflags_,include,sys.lib_,(PD.linkmap_?PD.linkmap_:""));

  // compile
  double time1 = getTime();
  *terminal << "-------" << std::endl << cmdline << std::endl;
  int rv = system(cmdline);
  *terminal << "-------" << std::endl;
  if( !rv )
    *terminal << "Successful compilation." << std::endl;
  else
    std::cerr << "Error." << std::endl;

  // register final time & print elapsed time
  double time2 = getTime();
  *terminal << time2-time1 << " seconds." << std::endl;

  // clean & return
  delete[] cmdline;
  if( include[0] != '\0' ) delete[] include;
  return(1);
}

static const char* solvePattern  = "solve( ([^[:blank:]]+)){0,1}( ([^[:blank:]]+)){0,1}";
static int solveSubexprs = 4;
static int solveFunction( int nargs, char** args )
{
  // reset signals and register initial time
  PD.signal_ = -1;

  // set problem
  if( !loadedObject || (args[2] && strcmp(args[2],currentProblem)) )
    setCurrentProblem(args[2]);

  // open outputFile
  bool stdOutput = false;
  if( args[4] ) {
    if( !strcasecmp(args[4],"-") )
      stdOutput = true;
    else {
      char *tmp = new char[1+strlen(args[4])];
      strcpy(tmp,args[4]);
      PD.outputFilename_ = tmp;
    }
  }
  else {
    char *tmp = new char[8+strlen(currentProblem)];
    sprintf(tmp,"%s.output",currentProblem);
    PD.outputFilename_ = tmp;
  }

  if( !stdOutput ) {
    PD.outputFile_ = new std::ofstream(PD.outputFilename_,std::ios_base::out|std::ios_base::app);
    if( !(*PD.outputFile_) ) {
      std::cerr << "Error: " << PD.outputFilename_ << ": " << strerror(errno) << "." << std::endl;
      return(1);
    }
  }
  else {
    PD.outputFile_ = terminal;
  }
  PD.setOutputFormat();

  // print some data
  *terminal << "Output redirected to ";
  if( stdOutput )
    *terminal << "<stdout>." << std::endl;
  else
    *terminal << "'" << PD.outputFilename_ << "'." << std::endl;

  // bootstrap it
  double time1 = getTime();
  if( !loadedObject && !bootstrap() ) return(1);
  PD.print(*PD.outputFile_,"%call ");
  *terminal << "Solving... " << std::endl;

  // solve problem
  loadedCore = false;
  try {
    PD.solveProblem();
  } 
  catch( Exception& ) {
    if( !stdOutput ) {
      ((std::ofstream*)PD.outputFile_)->close();
      delete PD.outputFile_;
      delete[] PD.outputFilename_;
      PD.outputFile_ = 0;
      PD.outputFilename_ = 0;
    }
    PD.pomdp_->cleanHash();
    throw;
  }
  double time2 = getTime();

  // print the model data
  //if( !stdOutput ) PD.model_->printData(*PD.outputFile_);
  //PD.model_->printData(*terminal);

  // register final time & print elapsed time
  *PD.outputFile_ << "%time " << time2-time1 << std::endl;
  if( !stdOutput ) *terminal << time2-time1 << " seconds" << std::endl;

  // close outputFile
  if( !stdOutput ) {
    ((std::ofstream*)PD.outputFile_)->close();
    delete PD.outputFile_;
    delete[] PD.outputFilename_;
    PD.outputFile_ = 0;
    PD.outputFilename_ = 0;
    }

  // check if we have a non-empty core
  if( !PD.pomdp_->emptyBeliefHash() ) loadedCore = true;

  return(1);
}

static const char* setPattern  = "set( (defaults|problem|stoprule|epsilon|epsilon-greedy|max-update|cutoff|control-updates|sondik|sondik-method|sondik-max-planes|sondik-iterations|pims|qmdp-discount|heuristic-lookahead|qmethod|qlevels|qbase|zero-heuristic|hash-all|random-ties|random-seed|output-level|verbose-level|precision|action-cache|obs-cache|other-cache|cc|ccflags|ld|ldflags|include-dir|lib-dir|entry-point)( (.+)){0,1}){0,1}";
static int setSubexprs = 4;
static void setValue( const char* var, const char* value );
static void invalidSetValue( const char* var, const char* value );
static void setPimValue( char* value );
static int setFunction( int nargs, char** args )
{
  if( !args[1] ) {
    terminal->precision(20);
    *terminal << std::endl
              << "Problem:" << std::endl
              << "  current               = " << (currentProblem?currentProblem:"(null)") << std::endl
              << "  loaded obj            = " << (loadedObject?"true":"false") << std::endl
              << "  loaded core           = " << (loadedCore?"true":"false") << std::endl
              << "  linkmap               = " << (PD.linkmap_?PD.linkmap_:"(null)") << std::endl
              << std::endl;
    *terminal << "General options:" << std::endl
	      << "  software-revision     = " << PD.softwareRevision_ << std::endl
              << "  version               = original [fixed]" << std::endl
              << "  epsilon               = " << PD.epsilon_ << std::endl
              << "  pims                  =" << (PD.pims_.empty()?" <empty>":"");
    for( int pim = 0; pim < (int)PD.pims_.size(); ++pim )
      *terminal << " [" << PD.pims_[pim].first << "," << PD.pims_[pim].second.first << "," << PD.pims_[pim].second.second << "]";
    *terminal << std::endl;
    *terminal << "  cutoff                = " << PD.cutoff_ << std::endl
              << "  qmdp-discount         = " << PD.QMDPdiscount_ << std::endl
              << "  heuristic-lookahead   = " << PD.lookahead_ << std::endl
              << "  zero-heuristic        = " << (PD.zeroHeuristic_?"on":"off") << std::endl
	      << "  hash-all              = " << (PD.hashAll_?"on":"off") << std::endl
              << "  random-ties           = " << (PD.randomTies_?"on":"off") << std::endl
              << "  random-seed           = " << PD.randomSeed_ << std::endl
              << "  verbose-level         = " << PD.verboseLevel_ << std::endl
              << "  precision             = " << PD.precision_ << std::endl
              << "  output-level          = " << PD.outputLevel_ << std::endl
	      << std::endl;
    *terminal << "Solver options:" << std::endl
	      << "  max-update            = " << (PD.maxUpdate_?"on":"off") << std::endl
              << "  stoprule              = ";
    if( PD.useStopRule_ )
      *terminal << PD.SREpsilon_ << std::endl;
    else
      *terminal << "off" << std::endl;
    *terminal << "  epsilon-greedy        = " << PD.epsilonGreedy_ << std::endl
	      << std::endl;
    *terminal << "Control options:" << std::endl
	      << "  control-updates       = " << (PD.controlUpdates_?"on":"off") << std::endl
              << "  sondik                = " << (PD.sondik_?"on":"off") << std::endl
              << "  sondik-method         = " << (PD.sondikMethod_==0?"timestamps":"updates") << std::endl
              << "  sondik-max-planes     = " << PD.sondikMaxPlanes_ << std::endl
              << "  sondik-iterations     = " << PD.sondikIterations_ << std::endl
	      << std::endl;
    *terminal << "Discretization options:" << std::endl
              << "  qmethod               = " << (PD.qmethod_==0?"plain":(PD.qmethod_==1?"log":"freudenthal")) << std::endl
              << "  qlevels               = " << PD.qlevels_ << std::endl
              << "  qbase                 = " << PD.qbase_ << " (only used by the 'log' method)" << std::endl
	      << std::endl;
    *terminal << "System:" << std::endl
              << "  cc                    = " << sys.cc_ << std::endl
              << "  ccflags               = " << sys.ccflags_ << std::endl
              << "  ld                    = " << sys.ld_ << std::endl
              << "  ldflags               = " << sys.ldflags_ << std::endl
              << "  include-dir           = " << sys.include_ << std::endl
              << "  lib-dir               = " << sys.lib_ << std::endl
              << "  entry-point           = " << sys.entry_ << std::endl
	      << std::endl;
    terminal->precision(PD.precision_);
  }
  else if( !args[3] ) {
    if( !strcasecmp(args[2],"defaults") )
      setDefaultValue(0,false);
    else
      setDefaultValue(args[2],false);
  }
  else {
    setValue(args[2],args[4]);
  }
  return(1);
}

static void
setValue( const char* var, const char* value )
{
  if( !strcasecmp(var,"problem") ) {
    setCurrentProblem(value);
  }
  else if( !strcasecmp(var,"stoprule") ) {
    if( !strcasecmp(value,"off") )
      PD.useStopRule_ = false;
    else {
      double farg = atof(value);
      if( farg < 0.0 )
        invalidSetValue(var,value);
      else {
        PD.SREpsilon_ = farg;
        PD.useStopRule_ = true;
      }
    }
  }
  else if( !strcasecmp(var,"epsilon") ) {
    double farg = atof(value);
    if( farg < 0.0 )
      invalidSetValue(var,value);
    else
      PD.epsilon_ = farg;
  }
  else if( !strcasecmp(var,"epsilon-greedy") ) {
    double arg = atof(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.epsilonGreedy_ = arg;
    if( PD.pomdp_ ) PD.pomdp_->setEpsilonGreedy(PD.epsilonGreedy_);
  }
  else if( !strcasecmp(var,"max-update") ) {
    if( !strcasecmp(value,"on") || !strcasecmp(value,"1") )
      PD.maxUpdate_ = true;
    else if( !strcasecmp(value,"off") || !strcasecmp(value,"0") )
      PD.maxUpdate_ = false;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"cutoff") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.cutoff_ = arg;
    if( PD.pomdp_ ) PD.pomdp_->setCutoff(PD.cutoff_);
  }
  else if( !strcasecmp(var,"control-updates") ) {
    if( !strcasecmp(value,"on") || !strcasecmp(value,"1") )
      PD.controlUpdates_ = true;
    else if( !strcasecmp(value,"off") || !strcasecmp(value,"0") )
      PD.controlUpdates_ = false;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"sondik") ) {
    if( !strcasecmp(value,"on") || !strcasecmp(value,"1") )
      PD.sondik_ = true;
    else if( !strcasecmp(value,"off") || !strcasecmp(value,"0") )
      PD.sondik_ = false;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"sondik-method") ) {
    if( !strcasecmp(value,"timestamps") )
      PD.sondikMethod_ = 0;
    else if( !strcasecmp(value,"updates") )
      PD.sondikMethod_ = 1;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"sondik-max-planes") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.sondikMaxPlanes_ = arg;
  }
  else if( !strcasecmp(var,"sondik-iterations") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.sondikIterations_ = arg;
  }
  else if( !strcasecmp(var,"pims") ) {
    PD.useStopRule_ = false;
    char *s = strdup(value);
    setPimValue(s);
    free(s);
  }
  else if( !strcasecmp(var,"qmdp-discount") ) {
    double farg = atof(value);
    if( (farg < 0.0) || (farg > 1.0) )
      invalidSetValue(var,value);
    else
      PD.QMDPdiscount_ = farg;
  }
  else if( !strcasecmp(var,"heuristic-lookahead") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.lookahead_ = arg;
  }
  else if( !strcasecmp(var,"qmethod") ) {
    if( !strcasecmp(value,"plain") )
      PD.qmethod_ = 0;
    else if( !strcasecmp(value,"log") )
      PD.qmethod_ = 1;
    else if( !strcasecmp(value,"freudenthal") )
      PD.qmethod_ = 2;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"qlevels") ) {
    int arg = atoi(value);
    if( (arg <= 0) || (arg > 255) )
      invalidSetValue(var,value);
    else {
      if( (arg != PD.qlevels_) && loadedCore ) {
        loadedCore = false;
        PD.clean(Problem::HASH);
        *terminal << "Hash cleared." << std::endl;
      }
      PD.qlevels_ = arg;
    }
  }
  else if( !strcasecmp(var,"qbase") ) {
    double arg = atof(value);
    if( arg <= 0 )
      invalidSetValue(var,value);
    else {
      if( (arg != PD.qbase_) && loadedCore ) {
        loadedCore = false;
        PD.clean(Problem::HASH);
        *terminal << "Hash cleared." << std::endl;
      }
      PD.qbase_ = arg;
    }
  }
  else if( !strcasecmp(var,"zero-heuristic") ) {
    if( !strcasecmp(value,"on") || !strcasecmp(value,"1") )
      PD.zeroHeuristic_ = true;
    else if( !strcasecmp(value,"off") || !strcasecmp(value,"0") )
      PD.zeroHeuristic_ = false;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"hash-all") ) {
    if( !strcasecmp(value,"on") || !strcasecmp(value,"1") )
      PD.hashAll_ = true;
    else if( !strcasecmp(value,"off") || !strcasecmp(value,"0") )
      PD.hashAll_ = false;
    else
      invalidSetValue(var,value);
  }
  else if( !strcasecmp(var,"random-seed") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else {
      PD.randomSeed_ = arg;
      short unsigned seed[3];
      seed[0] = (short unsigned)PD.randomSeed_;
      seed[1] = (short unsigned)PD.randomSeed_;
      seed[2] = (short unsigned)PD.randomSeed_;
      srand48((long)PD.randomSeed_);
      seed48(seed);
    }
  }
  else if( !strcasecmp( var, "random-ties" ) )
    {
      if( !strcasecmp( value, "on" ) || !strcasecmp( value, "1" ) )
	PD.randomTies_ = true;
      else if( !strcasecmp( value, "off" ) || !strcasecmp( value, "0" ) )
	PD.randomTies_ = false;
      else
	invalidSetValue( var, value );

      if( PD.pomdp_ )
	PD.pomdp_->setRandomTies( PD.randomTies_ );
    }
  else if( !strcasecmp(var,"output-level") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.outputLevel_ = arg;
  }
  else if( !strcasecmp(var,"verbose-level") ) {
    int arg = atoi( value );
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.verboseLevel_ = arg;
  }
  else if( !strcasecmp(var,"precision") ) {
    int arg = atoi(value);
    if( arg < 0 )
      invalidSetValue(var,value);
    else
      PD.precision_ = arg;
  }
  else if( !strcasecmp(var,"cc") ) {
    delete[] sys.cc_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.cc_ = tmp;
  }
  else if( !strcasecmp(var,"ccflags") ) {
    delete[] sys.ccflags_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.ccflags_ = tmp;
  }
  else if( !strcasecmp(var,"ld") ) {
    delete[] sys.ld_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.ld_ = tmp;
  }
  else if( !strcasecmp(var,"ldflags") ) {
    delete[] sys.ldflags_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.ldflags_ = tmp;
  }
  else if( !strcasecmp(var,"include-dir") ) {
    delete[] sys.include_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.include_ = tmp;
  }
  else if( !strcasecmp(var,"lib-dir") ) {
    delete[] sys.lib_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.lib_ = tmp;
  }
  else if( !strcasecmp(var,"entry-point") ) {
    delete[] sys.entry_;
    char *tmp = new char[1+strlen(value)];
    strcpy(tmp,value);
    sys.entry_ = tmp;
  }
  else {
    *terminal << "Unrecognized variable '" << var << "'." << std::endl;
  }
}

static void
setDefaultValue( const char* var, bool silent )
{
  std::map<const char*,const char*,stringCaseCompareFunction>::const_iterator it;
  if( var != 0 ) {
    it = defaultValues.find(var);
    if( it == defaultValues.end() )
      *terminal << "No default value for " << var << "." << std::endl;
    else {
      setValue( var, (*it).second );
      if( !silent ) *terminal << var << " is now " << (*it).second << "." << std::endl;
    }
  }
  else {
    for( it = defaultValues.begin(); it != defaultValues.end(); ++it )
      setValue((*it).first,(*it).second);
    if( !silent ) *terminal << "default values restored for all variables." << std::endl;
  }
}

static void
invalidSetValue( const char* var, const char* value )
{
  *terminal << "Invalid value '" << value << "' for: " << var << "." << std::endl;
}

static void
setPimValue( char* value )
{
  PD.pims_.clear();
  char *token = strtok(value," ");
  while( token ) {
    int s = -1, l = -1, c = -1;
    sscanf(token,"[%d,%d,%d]",&s,&l,&c);
    if( (s < 0) || (l < 0) || (c < 0) )
      *terminal << "Invalid pim value: " << token << "." << std::endl;
    else
      PD.pims_.push_back(std::make_pair(s,std::make_pair(l,c)));
    token = strtok(0," ");
  }
}

#if 0
static void
setCacheValue( int cacheId, const char* value )
{
  int s = -1, c = -1;
  sscanf(value,"[%d,%d]",&s,&c);
  if( (s < 0) || (c < 0) || ((s > 0) && (c == 0)) || ((c > 0) && (s % c != 0)) )
    *terminal << "Invalid cache values: " << value << "." << std::endl;
  else {
    PD.cacheSize_[cacheId] = s;
    PD.clusterSize_[cacheId] = c;
  }
}
#endif

static const char* cleanPattern  = "clean( (problem|object|hash|linkmap)){0,1}";
static int cleanSubexprs = 2;
static int cleanFunction( int nargs, char** args )
{
  if( args[1] ) {
    if( !strcasecmp(args[2],"problem") ) {
      delete[] currentProblem;
      currentProblem = 0;
      delete[] PD.linkmap_;
      PD.linkmap_ = 0;
      loadedObject = false;
      loadedCore = false;
      PD.clean(Problem::OBJECT);
    }
    else if( !strcasecmp(args[2],"object") ) {
      loadedObject = false;
      loadedCore = false;
      PD.clean(Problem::OBJECT);
    }
    else if( !strcasecmp(args[2],"hash") ) {
      loadedCore = false;
      PD.clean(Problem::HASH);
    }
    else if( !strcasecmp(args[2],"linkmap") ) {
      delete[] PD.linkmap_;
      PD.linkmap_ = 0;
    }
  }
  else { // default is 'clean hash'
    loadedCore = false;
    PD.clean(Problem::HASH);
  }
  return(1);
}

static const char* generatePattern  = "generate( (core|graph|hash|pomdp|table)){0,1}( ([[:print:]]+)){0,1}";
static int generateSubexprs = 5;
static int generateFunction( int nargs, char** args )
{
  if( !args[1] ) {
    *terminal << "Unrecognized command. Use 'generate (core|graph|pomdp|table) [<file>]'." << std::endl;
    return(1);
  }

  if( !strcasecmp(args[2],"core") ) {
    if( !loadedObject || !loadedCore )
      std::cerr << "Error: no current or empty core." << std::endl;
    else {
      char *filename = 0;
      if( args[4] != 0 ) {
        filename = new char[1+strlen(args[4])];
        strcpy(filename,args[4]);
      }
      else {
        filename = new char[6+strlen(currentProblem)];
        sprintf(filename,"%s.core",currentProblem);
      }
      std::ofstream outputFile(filename,std::ios::out);
      if( !outputFile ) {
        std::cerr << "Error: " << filename << ": " << strerror(errno) << "." << std::endl;
        return(1);
      }
      delete[] filename;
      //writeCore(outputFile);
      outputFile.close();
    }
  }
  else if( !strcasecmp(args[2],"pomdp") ) {
    std::ofstream *outputFile = 0;
    if( args[3] != 0 ) {
      outputFile = new std::ofstream(args[4],std::ios::out);
      if( !outputFile ) {
        std::cerr << "Error: " << args[4] << ": " << strerror(errno) << "." << std::endl;
        return(1);
      }
    }
    static_cast<const StandardModel*>(PD.model_)->outputCASSANDRA(*outputFile);
  }
  else if( !strcasecmp(args[2],"graph") || !strcasecmp(args[2],"hash") || !strcasecmp(args[2],"table") ) {
    // read problem name and core name (if necessary)
    char *filename = 0, *tmp = 0;
    std::ofstream *outputFile = 0;

    setCurrentProblem(0);
    if( !loadedCore ) {
      readlineCompletionMode(RLC_FILES);
      while( (tmp = readline("Input corefile: ")) ) {
        stripWhite(tmp);
        if( tmp[0] != '\0' ) break;
        free(tmp);
      }
      readlineCompletionMode(RLC_COMMANDS);
      filename = new char[1+strlen(tmp)];
      strcpy(filename,tmp);
      free(tmp);
    }

    // check that the model support generation
    if( !strcasecmp(args[2],"graph") || !strcasecmp(args[2],"table") ) {
      if( PD.pddlProblem_ && !ISMDP(PD.handle_->problemType) && 
          !ISNDMDP(PD.handle_->problemType) && !ISPOMDP(PD.handle_->problemType) &&
          !ISNDPOMDP(PD.handle_->problemType) ) {
        std::cerr << "Error: this function is only available for MDP/POMDP models." << std::endl;
        return(1);
      }
    }

    // open output file
    if( args[3] != 0 ) {
      outputFile = new std::ofstream(args[4],std::ios::out);
      if( !outputFile ) {
        std::cerr << "Error: " << args[4] << ": " << strerror( errno ) << "." << std::endl;
        delete[] filename;
        return(1);
      }
    }

    // bootstrap problem
    if( !loadedObject && !bootstrap() ) {
      delete[] filename;
      delete outputFile;
      return(1);
    }

    // load core
    if( !loadedCore ) {
      if( readcore(filename) ) {
        delete[] filename;
        delete outputFile;
        return(1);
      }
      delete[] filename;
    }

    // generate output
    if( !strcasecmp(args[2],"graph") ) {
#if 0
      PolicyGraph graph;
      graph.generatePolicyGraph();
      if( outputFile != 0 )
        graph.outputPolicyGraph(*outputFile);
      else
        graph.outputPolicyGraph(std::cout);
      *terminal << "Graph generated." << std::endl;
#endif
    }
    else if( !strcasecmp(args[2],"table") ) {
#if 0
      PolicyTable table;
      table.generatePolicyTable();
      if( outputFile != 0 )
        table.outputPolicyTable(*outputFile);
      else
        table.outputPolicyTable(std::cout);
      *terminal << "Policy generated." << std::endl;
#endif
    }
    else {
      if( outputFile != 0 )
        PD.pomdp_->printHash(*outputFile);
      else
        PD.pomdp_->printHash(std::cout);
      *terminal << "Hash generated." << std::endl;
    }

    // close output file
    if( outputFile != 0 ) {
      outputFile->close();
      delete outputFile;
    }
  }
  return(1);
}

#define SHELL "ksh"
static const char* shellPattern  = "(shell |!)([[:print:]]+){0,1}";
static int shellSubexprs = 2;
static int shellFunction( int nargs, char** args )
{
  int rv = 0;
  if( args[2] ) {
    char *cmdline = new char[1+strlen(args[2])];
    sprintf(cmdline,"%s",args[2]);
    rv = system(cmdline);
    delete[] cmdline;
  }
  else {
    rv = system(SHELL);
  }
  return(rv);
}

static const char* viewerPattern  = "viewer( ([[:print:]]+)){0,1}";
static int viewerSubexprs = 2;
static int viewerFunction( int nargs, char** args )
{
  char *cmdline = new char[14+strlen(sys.gpthome_)+(args[2]?1+strlen(args[2]):0)];
  sprintf(cmdline,"%s/bin/viewer%s%s &",sys.gpthome_,(args[2]?" ":""),(args[2]?args[2]:""));
  int rv = system(cmdline);
  delete[] cmdline;
  return(rv);
}

///////////////////////////////////////////////////////////////////////////////
//
// Commands Definitions
//

static int registeredCommandsSize = 0;
static int registeredCommandsEntries = 0;
static Command** registeredCommands = 0;

static void
commandRegister( Command* command )
{
  if( registeredCommandsEntries == registeredCommandsSize ) {
    registeredCommandsSize = (!registeredCommandsSize ? 2 : 2*registeredCommandsSize);
    registeredCommands = (Command**)realloc(registeredCommands,registeredCommandsSize*sizeof(Command*));
  }
  registeredCommands[registeredCommandsEntries++] = command;
}

static void
commandRegistration()
{
  commandRegister( new Command("about",aboutPattern,aboutSubexprs,&aboutFunction) );
  commandRegister( new Command("help",helpPattern,helpSubexprs,&helpFunction) );
  commandRegister( new Command("cd",cdPattern,cdSubexprs,&cdFunction) );
  commandRegister( new Command("clear",clearPattern,clearSubexprs,&clearFunction) );
  commandRegister( new Command("quit",quitPattern,quitSubexprs,&quitFunction) );
  commandRegister( new Command("exit",quitPattern,quitSubexprs,&quitFunction) );
  commandRegister( new Command("parse",parsePattern,parseSubexprs,&parseFunction) );
  commandRegister( new Command("print",printPattern,printSubexprs,&printFunction) );
  commandRegister( new Command("linkmap",linkmapPattern,linkmapSubexprs,&linkmapFunction) );
  commandRegister( new Command("load",loadPattern,loadSubexprs,&loadFunction) );
  commandRegister( new Command("compile",compilePattern,compileSubexprs,&compileFunction) );
  commandRegister( new Command("solve",solvePattern,solveSubexprs,&solveFunction) );
  commandRegister( new Command("set",setPattern,setSubexprs,&setFunction) );
  commandRegister( new Command("clean",cleanPattern,cleanSubexprs,&cleanFunction) );
  commandRegister( new Command("generate",generatePattern,generateSubexprs,&generateFunction) );
  commandRegister( new Command("shell",shellPattern,shellSubexprs,&shellFunction) );
  commandRegister( new Command("viewer",viewerPattern,viewerSubexprs,&viewerFunction) );
}

static void
commandCleanup()
{
  for( int i = 0; i < registeredCommandsEntries; ++i )
    delete registeredCommands[i];
  free(registeredCommands);
}

static Command*
commandParse( char *input, int &nargs, char** &args )
{
  regmatch_t pmatch[32];
  nargs = 0;
  args = 0;
  for( Command **ptr = registeredCommands; ptr - registeredCommands < registeredCommandsEntries; ++ptr ) {
    if( !regexec(&(*ptr)->regex_,input,1+(*ptr)->subexprs_,pmatch,0) && !pmatch[0].rm_so && !input[pmatch[0].rm_eo] ) {
      // fill-in command arguments
      if( (nargs = (*ptr)->subexprs_) ) {
        args = new char*[nargs+1];
        args[0] = 0;
        for( int arg = 1; arg <= nargs; ++arg )
          if( (pmatch[arg].rm_so != -1) && (pmatch[arg].rm_eo != -1) ) {
            args[arg] = new char[pmatch[arg].rm_eo-pmatch[arg].rm_so+1];
            strncpy(args[arg],&input[pmatch[arg].rm_so],pmatch[arg].rm_eo-pmatch[arg].rm_so);
            args[arg][pmatch[arg].rm_eo-pmatch[arg].rm_so] = '\0';
         }
          else
            args[arg] = 0;
      }
      return(*ptr);
    }
  }
  return(0);
}

///////////////////////////////////////////////////////////////////////////////
//
// readline completion functions (taken from readline distribution examples)
//

static char* command_generator( const char*, int );
static char** fileman_completion( const char*, int, int );
static int rlcMode;

typedef struct { const char* prefix; int pfxsz; const char** comp; } completionInfo_t;

static const char* fullList[] = { "about", "cd", "clean", "clear", "compile", "exit", "generate", 
				  "help", "linkmap", "load", "parse", "print", "quit", "set",
				  "shell", "solve", "viewer", 0 };
static const char* cleanList[] = { "problem", "object", "hash", "linkmap", 0 };
static const char* generateList[] = { "core", "graph", "hash", "pomdp", "table", 0 };
static const char* loadList[] = { "core", "problem", 0 };
static const char* linkmapList[] = { "+", "-", 0 };
static const char* setList[] = { "defaults", "problem", "stoprule", "epsilon", "epsilon-greedy",
                                 "max-update", "cutoff", "control-updates", 
                                 "sondik", "sondik-method", "sondik-max-planes", "sondik-iterations",
                                 "pims", "qmdp-discount",
				 "heuristic-lookahead", "qmethod", "qlevels", "qbase", "zero-heuristic",
                                 "hash-all", "random-ties", "random-seed", "output-level",
				 "verbose-level", "precision", "action-cache", "obs-cache",
				 "other-cache", "cc", "ccflags", "ld", "ldflags", "include-dir",
				 "lib-dir", "entry-point", 0 };
static const char* printList[] = { "action", "action@", "belief", "observation", "state", 0 };
static const char* beliefList[] = { "actions","afteraction", "initial", 0 };
static const char* stateList[] = { "actions", "afteraction", 0 };
static const char* onOffList[] = { "on", "off", 0 };
static const char* sondikList[] = { "sondik", "sondik-method", "sondik-max-planes", "sondik-iterations", 0 };
static const char* emptyList[] = { 0 };

static const char** possibleCtxCompletions = 0;
static completionInfo_t completionInfo[] = { 
  { "about", 0, 0 },
  { "cd", 0, 0 },
  { "clean compile", 0, emptyList },
  { "clean object", 0, emptyList },
  { "clean hash", 0, emptyList },
  { "clean linkmap", 0, emptyList },
  { "clean", 0, cleanList },
  { "clear", 0, emptyList },
  { "compile", 0, 0 },
  { "exit", 0, emptyList },
  { "generate core", 0, 0 },
  { "generate graph", 0, 0 },
  { "generate hash", 0, 0 },
  { "generate pomdp", 0, 0 },
  { "generate table", 0, 0 },
  { "generate", 0, generateList },
  { "help clean", 0, emptyList },
  { "help clear", 0, emptyList },
  { "help compile", 0, emptyList },
  { "help exit", 0, emptyList },
  { "help generate", 0, emptyList },
  { "help help", 0, emptyList },
  { "help linkmap", 0, emptyList },
  { "help parse", 0, emptyList },
  { "help quit", 0, emptyList },
  { "help set", 0, emptyList },
  { "help shell", 0, emptyList },
  { "help solve", 0, emptyList },
  { "help", 0, fullList },
  { "linkmap +", 0, 0 },
  { "linkmap -", 0, 0 },
  { "linkmap", 0, linkmapList },
  { "load core", 0, 0 },
  { "load problem", 0, 0 },
  { "load", 0, loadList },
  { "parse", 0, 0 },
  { "print action", 0, emptyList },
  { "print action@", 0, emptyList },
  { "print belief initial", 0, emptyList },
  { "print belief", 0, beliefList },
  { "print observation", 0, emptyList },
  { "print state", 0, stateList },
  { "print", 0, printList },
  { "quit", 0, emptyList },
  { "set defaults", 0, emptyList },
  { "set problem", 0, 0 },
  { "set stoprule", 0, emptyList },
  { "set epsilon", 0, emptyList },
  { "set epsilon-greedy", 0, emptyList },
  { "set max-update", 0, emptyList },
  { "set cutoff", 0, emptyList },
  { "set control-updates", 0, emptyList },
  { "set sondik", 0, sondikList },
  { "set sondik-method", 0, emptyList },
  { "set sondik-max-planes", 0, emptyList },
  { "set sondik-iterations", 0, emptyList },
  { "set pims", 0, emptyList },
  { "set qmdp-discount", 0, emptyList },
  { "set heuristic-lookahead", 0, emptyList },
  { "set qmethod", 0, emptyList },
  { "set qlevels", 0, emptyList },
  { "set qbase", 0, emptyList },
  { "set zero-heuristic", 0, emptyList },
  { "set hash-all", 0, emptyList },
  { "set random-ties", 0, onOffList },
  { "set random-seed", 0, emptyList },
  { "set output-level", 0, emptyList },
  { "set verbose-level", 0, emptyList },
  { "set precision", 0, emptyList },
  { "set action-cache", 0, emptyList },
  { "set obs-cache", 0, emptyList },
  { "set other-cache", 0, emptyList },
  { "set cc", 0, emptyList },
  { "set ccflags", 0, emptyList },
  { "set ld", 0, emptyList },
  { "set ldflags", 0, emptyList },
  { "set include-dir", 0, 0 },
  { "set lib-dir", 0, 0 },
  { "set entry-point", 0, 0 },
  { "set", 0, setList },
  { "shell", 0, 0 },
  { "solve", 0, 0 },
  { "viewer", 0, 0 },
  { "", 0, fullList }, 
  { 0, 0, 0 }
};

static void
readlineCompletionMode( int mode )
{
  rlcMode = mode;
}

static void
clearLineBuffer()
{
  rl_point = 0;
  rl_end = 0;
  rl_line_buffer[0] = '\0';
}

// Tell the GNU Readline library how to complete.  We want to try to complete
// on command names if this is the first word in the line, or on filenames
// if not.
static void
initializeReadline()
{
  // Allow conditional parsing of the ~/.inputrc file
  rl_readline_name = "GPT";

  // Tell the completer that we want a crack first
  rl_attempted_completion_function = fileman_completion;

  // fill completion info
  for( completionInfo_t *info = completionInfo; info->prefix; ++info )
    info->pfxsz = strlen(info->prefix);
}

// Attempt to complete on the contents of TEXT.  START and END bound the
// region of rl_line_buffer that contains the word to complete.  TEXT is
// the word to complete.  We can use the entire contents of rl_line_buffer
// in case we want to do some simple parsing.  Return the array of matches,
// or NULL if there aren't any.
static char** 
fileman_completion( const char* text, int start, int end )
{
  completionInfo_t *info = 0;

  // Use appropriate mode.
  switch( rlcMode ) {
  case RLC_COMMANDS:
    // find context; if valid make completion on it, else go for a filename
    for( info = completionInfo; info->prefix != 0; ++info )
      if( !strncasecmp(info->prefix,rl_line_buffer,info->pfxsz) )
    break;
    if( info->comp == 0 )
      return(0);
    else
      possibleCtxCompletions = info->comp;
    return( rl_completion_matches(text,command_generator) );
  case RLC_FILES:
    return(0);
  }
  return(0);
}

// Generator function for command completion.  STATE lets us know whether
// to start from scratch; without any state (i.e. STATE == 0), then we
// start at the top of the list.
static char*
command_generator( const char* text, int state )
{
  static const char **ptr;
  static int len;

  // If this is a new word to complete, initialize now.  This includes
  // saving the length of TEXT for efficiency, and initializing the index
  // variable to 0.
  if( !state ) {
    len = strlen(text);
    ptr = possibleCtxCompletions;
  }

  // look in command list.
  // it is NECESSARY to allocate memory here since readline will
  // attempt to free it.
  while( *ptr ) {
    if( !strncasecmp(*ptr++,text,len) )
      return( strdup(*(ptr-1)) );
  }
  return(0);
}

