/*  imm-reward.c

 * Copyright 1996,1997, Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

For a sparse representation, the main thing we are trying to avoid is
keeping a matrix that is NxN where N is the number of rows and columns
(states in the MDP case).  This is easily accomplished with the
transition matrices and observation matrices.  However, the most
general form for the specification of an immediate rewards in a POMDP
conditions the reward upon the actions, current state, next state and
observation.  This requires a matrix for each combination of action
and state and the size of these matrices will be NxO where N is the
numer of states and O the number of observations.  Although we can
keep the individual matrices in a sparse representation where we have
a row for each state, we will require as many of these as there are
states, effectively requiring a matrix that is at least NxN.  We
choose not to limit the file format's reward specification as
previously defined, yet want to keep a short internal repressntation
for the rewards.

Since the general rewards are used for computing the immediate rewards
for each state-action pair at the start of the program, we do not need
to be very concerned with efficiency.  We will keep the shorthand
representation around, in case individual elements are needed (e.g.,
in a simulation).  However, getting the immediate reward from this
representation will be slower than getting the other problem
parameters.

Note that for MDPs we do not have to worry about this issue.  We
cannot condition the immediate rewards upon the observation (there
is not a notion of observation in MDPs) so we merely need a sparse NxN
matrix for each action.  This will require us to keep track of the
type of problem we have loaded.

However, even for the MDP case we would like to take advantage of any 
wildcard character shortcuts, so we use this module for both the MDP 
and POMDP case.  However, things will be slightly different depending 
upon the type of problem being parsed (in gProblemType).

Here's how this module interacts with the parser: When the Parser sees
a line that begins with R: it will call the newImmReward() routine.
This will create an Imm_Reward_List node and fill in the proper
information.  If necessary, it will then initialize the intermediate
sparse matrix representation (i.e., next_state and obs not specified
for a POMDP or cur_state and next_state not specified for an MDP).
The node is not added to the list at this time.  The parser will then
deliver each value for this 'R:' entry individually through the
routine enterImmReward(). As the actual values are parsed, one of
three things could happen: It could require only a single value, it
could require a vector of values or it could be a matrix of values.
With a single value we just set it in the node.  With the vector we
set the proper entry each time a value is passed in.  Finally, if it
is an entire matrix, we enter it into the sparse representation.  When
the current 'R:' line is finished, the parser will call
doneImmReward() which will first, if necessary, transform the
intermediate sparse matrix in to a sparse matrix.  Then it will put
this into the node and add the node to the list.

Note that the semantics of the file is such that sequentially later
values override earlier valus.  This means that for a particular
combination of action, cur_state, next state and obs could have more
than one value.  The last value in the file is the one that is
correct.  Therefore we need to keep the linked list in order from
oldest to newest.  Then when a particular value is desired, we must
run through the entire list, setting the value each time we see a
specification for it.  In this way we will be left with the last value
that was specified in the file.  

*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mdp.h"
#include "sparse-matrix.h"
#include "imm-reward.h"

#include <list>

/* As we parse the file, we will encounter only one R : * : *.... line
   at a time, so we will keep the intermediate matrix as a global
   variable.  When we start to enter a line we will initial it and
   when we are finished we will convert it and store the sparse matrix
   into the node of the linked list.  */ 
I_Matrix gCurIMatrix = NULL;

/* We will have most of the information we need when we first start to
  parse the line, so we will create the node and put that information
  there.  After we have read all of the values, we will put it into
  the linked list.  */
Imm_Reward *gCurImmReward = 0;

/* This is the actual list of immediate reward lines */
//Imm_Reward_List gImmRewardList = NULL;

std::list<Imm_Reward*> gAllRewards;
std::list<Imm_Reward*> *gImmRewards;

void initializeImmRewards()
{
  gImmRewards = new std::list<Imm_Reward*>[gNumActions*gNumStates];
}

void destroyImmRewards()
{
  for( std::list<Imm_Reward*>::const_iterator li = gAllRewards.begin(); li != gAllRewards.end(); ++li ) {
    switch( (*li)->type ) {
      case ir_vector:
        free( (*li)->rep.vector );
        break;
      case ir_matrix:
        destroyMatrix( (*li)->rep.matrix );
        break;
      case ir_value:
      default:
        break;
    }
    delete (*li);
  }
  gAllRewards.clear();

  for( int a = 0; a < gNumActions; ++a ) {
    for( int cs = 0; cs < gNumStates; ++cs )
      gImmRewards[cs*gNumActions+a].clear();
  }
}

void newImmReward( int action, int cur_state, int next_state, int obs )
{
  /* First we will allocate a new node for this entry */
  gCurImmReward = new Imm_Reward;
  gCurImmReward->action = action;
  gCurImmReward->cur_state = cur_state;
  gCurImmReward->next_state = next_state;
  gCurImmReward->obs = obs;

  switch( gProblemType ) {
  case POMDP_problem_type:
    if( obs == NOT_PRESENT) {
      if( next_state == NOT_PRESENT ) {
        /* This is the situation where we will need to keep a sparse 
           matrix, so let us initialize the global I_Matrix variable */
        gCurIMatrix = newIMatrix( gNumStates );
        gCurImmReward->rep.matrix = NULL;
        gCurImmReward->type = ir_matrix;
      }
      else { /* we will need a vector of numbers, not a matrix */
        gCurImmReward->rep.vector = (double *) calloc( gNumObservations, sizeof(double));
        gCurImmReward->type = ir_vector;
      }
    }
    else { /* We only need a single value, so let us just initialize it to zero */
      gCurImmReward->rep.value = 0.0;
      gCurImmReward->type = ir_value;
    }
    break;
  case MDP_problem_type:
    /* for this case we completely ignor 'obs' parameters */
    if( next_state == NOT_PRESENT ) {
      if( cur_state == NOT_PRESENT ) {
        /* This is the situation where we will need to keep a sparse 
           matrix, so let us initialize the global I_Matrix variable.  */
        gCurIMatrix = newIMatrix( gNumStates );
        gCurImmReward->rep.matrix = NULL;
        gCurImmReward->type = ir_matrix;
      }
      else { /* we will need a vector of numbers, not a matrix */
        gCurImmReward->rep.vector = (double *) calloc( gNumStates, sizeof(double));
        gCurImmReward->type = ir_vector;
      }
    }
    else { /* We only need a single value, so let us just initialize it to zero */
      gCurImmReward->rep.value = 0.0;
      gCurImmReward->type = ir_value;
    }
    break;
  default:
    fprintf( stderr, "**ERR** newImmReward: Unreckognised problem type.\n");
    exit( -1 );
    break;
  }
}

void enterImmReward( int cur_state, int next_state, int obs, double value )
{
/* cur_state is ignored for a POMDP, and obs is ignored for an MDP */
  assert( gCurImmReward != NULL );

  switch( gCurImmReward->type ) {
    case ir_value:
      gCurImmReward->rep.value = value;
      break;
    case ir_vector:
      if( gProblemType == POMDP_problem_type )
        gCurImmReward->rep.vector[obs] = value;
      else
        gCurImmReward->rep.vector[next_state] = value;
      break;
    case ir_matrix:
      if( gProblemType == POMDP_problem_type )
        addEntryToIMatrix( gCurIMatrix, next_state, obs, value );
      else
        addEntryToIMatrix( gCurIMatrix, cur_state, next_state, value );
      break;
    default:
      fprintf( stderr, "** ERR ** Unreckognized IR_Type in enterImmReward().\n");
      exit( -1 );
      break;
  }
}

void insertImmReward( Imm_Reward *r )
{
  int a = r->action, cs = r->cur_state;
  if( (a == WILDCARD_SPEC) && (cs == WILDCARD_SPEC) ) {
    for( a = 0; a < gNumActions; ++a ) {
      for( int cs = 0; cs < gNumStates; ++cs )
        gImmRewards[cs*gNumActions+a].push_back(r);
    }
  }
  else if( a == WILDCARD_SPEC ) {
    for( a = 0; a < gNumActions; ++a )
      gImmRewards[cs*gNumActions+a].push_back(r);
  }
  else if( cs == WILDCARD_SPEC ) {
    for( int cs = 0; cs < gNumStates; ++cs )
      gImmRewards[cs*gNumActions+a].push_back(r);
  }
  else {
    gImmRewards[cs*gNumActions+a].push_back(r);
  }
  gAllRewards.push_back(r);
}

void doneImmReward()
{
  if( gCurImmReward == NULL ) return;
  switch( gCurImmReward->type ) {
    case ir_value:
    case ir_vector:
      /* Do nothing for these cases */
      break;
    case ir_matrix:
      gCurImmReward->rep.matrix = transformIMatrix( gCurIMatrix );
      destroyIMatrix( gCurIMatrix );
      gCurIMatrix = NULL;
      break;
    default:
      fprintf( stderr, "** ERR ** Unreckognized IR_Type in doneImmReward().\n");
      exit( -1 );
      break;
  }
  insertImmReward(gCurImmReward);
  //gImmRewardList = appendImmRewardList( gImmRewardList, gCurImmRewardNode );
  gCurImmReward = 0;
}

double getImmediateReward( int action, int cur_state, int next_state, int obs )
{
  assert( (action >= 0) && (action < gNumActions) && (cur_state >= 0) &&
          (cur_state < gNumStates) && (next_state >= 0) && (next_state < gNumStates) );

  double return_value = 0.0;
  int index = cur_state*gNumActions+action;
  for( std::list<Imm_Reward*>::const_iterator li = gImmRewards[index].begin(); li != gImmRewards[index].end(); ++li ) {
    assert( ((*li)->action == WILDCARD_SPEC) || ((*li)->action == action) );
    if( ((*li)->action == WILDCARD_SPEC) || ((*li)->action == action) ) {
      switch( (*li)->type ) {
      case ir_value:
	if( gProblemType == POMDP_problem_type ) {
	  if( (((*li)->next_state == WILDCARD_SPEC) || ((*li)->next_state == next_state)) && 
              (((*li)->obs == WILDCARD_SPEC) || ((*li)->obs == obs)) &&
              (((*li)->cur_state == WILDCARD_SPEC) || ((*li)->cur_state == cur_state)) ) {
	    return_value = (*li)->rep.value;
	  }  /* if we have a match */
	}  /* if POMDP */
	else {  /* then it is an MDP */
	  if( (((*li)->cur_state == WILDCARD_SPEC) || ((*li)->cur_state == cur_state)) &&
              (((*li)->next_state == WILDCARD_SPEC) || ((*li)->next_state == next_state)) ) {
	    return_value = (*li)->rep.value;
	  }  /* if we have a match */
	}
	break;
      case ir_vector:
	if( gProblemType == POMDP_problem_type ) {
	  if( (((*li)->next_state == WILDCARD_SPEC) || ( (*li)->next_state == next_state)) &&
              (((*li)->cur_state == WILDCARD_SPEC) || ((*li)->cur_state == cur_state)) ) {
	    return_value = (*li)->rep.vector[obs];
	  }
	}  /* if POMDP */
	else {  /* it is an MDP */
	  if( ((*li)->cur_state == WILDCARD_SPEC) || ((*li)->cur_state == cur_state) ) {
	    return_value = (*li)->rep.vector[next_state];
	  }
	}
	break;
      case ir_matrix:
	if( gProblemType == POMDP_problem_type )  {
	  if( ((*li)->cur_state == WILDCARD_SPEC) || ((*li)->cur_state == cur_state))
	    return_value = getEntryMatrix((*li)->rep.matrix,next_state,obs);
	}
	else
	  return_value = getEntryMatrix((*li)->rep.matrix,cur_state,next_state);
	break;
      default:
	fprintf(stderr,"** ERR ** Unreckognized IR_Type in getImmediateReward().\n");
	exit(-1);
	break;
      }
    }
  }
  return(return_value);
}

void writeImmReward( std::ostream &os, int action, int cur_state, int next_state, int obs, double reward )
{
  os << "R: ";

  if( action == -1 )
    os << "* : ";
  else
    os << action << " : ";

  if( cur_state == -1 )
    os << "* : ";
  else
    os << cur_state << " : ";

  if( next_state == -1 )
    os << "* : ";
  else
    os << next_state << " : ";

  if( obs == -1 )
    os << "* ";
  else
    os << obs << " : ";

  os.precision(6);
  os << reward << std::endl;
}

int writeImmRewards( std::ostream &os, double shift )
{
  for( std::list<Imm_Reward*>::const_iterator li = gAllRewards.begin(); li != gAllRewards.end(); ++li ) {
    int action = (*li)->action;
    int cur_state = (*li)->cur_state;
    int next_state = (*li)->next_state;
    int obs = (*li)->obs;
    switch( (*li)->type ) {
    case ir_value:
      writeImmReward(os,action,cur_state,next_state,obs,(*li)->rep.value+shift);
      break;
    case ir_vector:
      for( int obs = 0; obs < gNumObservations; ++obs ) {
        double reward = (*li)->rep.vector[obs];
        if( reward != 0.0 ) writeImmReward(os,action,cur_state,next_state,obs,reward+shift);
      }
      break;
    case ir_matrix:
      for( int next_state = 0; next_state < gNumStates; ++next_state ) {
        for( int obs = 0; obs < gNumObservations; ++obs ) {
          double reward = getEntryMatrix((*li)->rep.matrix,next_state,obs);
          if( reward != 0.0 ) writeImmReward(os,action,cur_state,next_state,obs,reward+shift);
        }
      }
      break;
    default:
      std::cerr << "** ERR ** Unreckognized IR_Type in getImmediateReward()." << std::endl;
      return(0);
    }
  }
  return(1);
}

