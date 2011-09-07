//  Theseus
//  RtStandard.h -- RunTime Standard Support
//
//  Blai Bonet, Hector Geffner
//  Universidad Simon Bolivar (c) 1998-2008

#ifndef _RtStandard_INCLUDE
#define _RtStandard_INCLUDE


class stateClass;
class stateListClass;
class observationListClass;
class Model;
class Belief;
class Heuristic;


class stateClass
{
public:
  struct node_t
  {
    int value;
    int depth;
    node_t*  parent;
    node_t** children;
    node_t() { value = 0; depth = 0; parent = NULL; children = NULL; }
  };

  // members
  int valid;

  // static members
  static int stateSize;

  // contructors
  stateClass() { }
  virtual ~stateClass() { }

  // virtual operators
  virtual stateClass& operator=( const stateClass& s ) = 0;
  virtual bool operator==( const stateClass& s ) const = 0;

  // virtual methods
  virtual stateClass *clone( void ) const = 0;
  virtual void applyRamifications( void ) = 0;
  virtual bool goal( void ) const = 0;
  virtual stateListClass *applyAllActions( void ) const = 0;
  virtual void print( std::ostream& os, int indent ) const = 0;
  virtual const stateClass::node_t* lookup( const node_t *node ) const = 0;
  virtual const stateClass::node_t* insert( node_t *node, int index ) const = 0;
  virtual void recover( const node_t *node ) = 0;
};

class observationClass
{
public:
  // contructors
  observationClass() { }
  virtual ~observationClass() { }

  // virtual operators
  virtual observationClass& operator=( const observationClass& s ) = 0;
  virtual bool operator==( const observationClass& s ) const = 0;

  // virtual methods
  virtual void setValue( int, int ) = 0;
  virtual observationClass *clone( void ) const = 0;
  virtual void print( std::ostream& os, int indent ) const = 0;
};

class stateListClass
{
public:
  // members
  int action;
  double cost;
  double probability;
  stateClass *state;
  observationListClass *observations;

  // usually this list is treated like an array except 
  // for initial situation when we use these pointers
  stateListClass *prev;
  stateListClass *next;
  stateListClass *last;

  // contructors
  stateListClass()
    {
      action = -1;
      cost = -1;
      probability = -1;
      state = NULL;
      observations = NULL;
      prev = NULL;
      next = NULL;
      last = this;
    }
  virtual ~stateListClass()
    {
      // only delete state, observations are deleted elsewhere
      delete state;
    }

  // methods
  stateListClass *concat( stateListClass *sl );

  // virtual methods
  virtual stateListClass *alloc( int size ) = 0;
  virtual void print( std::ostream& os, int indent ) const;
};

class observationListClass
{
public:
  // members
  double probability;
  observationClass *observation;

  // contructors
  observationListClass()
    {
      probability = -1;
      observation = NULL;
    }
  virtual ~observationListClass() { }

  // virtual methods
  virtual void print( std::ostream& os, int indent ) const;
};

class ProblemHandle
{
public:
  // members
  void*               dlHandle;
  int                 problemType;
  int                 hookType;
  int                 numActions;
  int                 stateSize;
  int                 observationSize;
  observationClass*   nullObservation;
  stateListClass*     (*bootstrap)( void );
  void                (*cleanUp)( void );
  void                (*printAction)( std::ostream& os, int action );
  void                (*stateAllocFunction)( int number, stateClass **pool );
  void                (*stateDeallocFunction)( stateClass *pool );
  void                (*initializeFunction)( void );
  unsigned            otherInfo;

  Model*         modelHook;
  Belief*        beliefHook;
  Heuristic*     heuristicHook;

  // constants
  enum { PROBLEM_PLANNING, 
	 PROBLEM_CONFORMANT1, PROBLEM_CONFORMANT2, 
	 PROBLEM_MDP, 
	 PROBLEM_ND_MDP, 
	 PROBLEM_POMDP1, PROBLEM_POMDP2, 
	 PROBLEM_ND_POMDP1, PROBLEM_ND_POMDP2 };
  enum { HOOK_NONE = 0, 
	 HOOK_HEURISTIC = 1, 
	 HOOK_BELIEF = 2, 
	 HOOK_MODEL = 4 };
  enum { CERTAINTY_GOAL = 0 };

  // destructor
  ~ProblemHandle() { delete nullObservation; }
};


class planningHeuristicClass
{
public:
  // contructors
  planningHeuristicClass();
  virtual ~planningHeuristicClass();

  // virtual methods
  virtual int planningHeuristic( stateClass& s ) = 0;
};


// utilities

//stateClass::node_t* 
void recover( const stateClass::node_t *node, stateClass *s );
void printNodeTree( const stateClass::node_t *node );
void concatLists( stateListClass*& list1, stateListClass* list2 );
void removeElement( stateListClass*& list, stateListClass* elem );
void printBits( std::ostream &os, unsigned pack );

std::ostream& operator<<( std::ostream& os, const stateClass& s );
std::ostream& operator<<( std::ostream& os, const observationClass& s );
std::ostream& operator<<( std::ostream& os, const stateListClass& s );
std::ostream& operator<<( std::ostream& os, const observationListClass& s );

inline bool
bitValue( const unsigned *ptr, unsigned offset )
{
  const unsigned *p = ptr + (offset / (8*sizeof(unsigned)));
  const unsigned o = offset % (8*sizeof(unsigned));
  return( (bool)((*p >> o) & 1) );
}

inline void
setBit( unsigned *ptr, unsigned offset, bool value )
{
  unsigned *p = ptr + (offset / (8*sizeof(unsigned)));
  unsigned o = offset % (8*sizeof(unsigned));
  if( value )
    *p = *p | (1 << o);
  else
    *p = *p & ~(1 << o);
}

#endif  // _RtStandard_INCLUDE
