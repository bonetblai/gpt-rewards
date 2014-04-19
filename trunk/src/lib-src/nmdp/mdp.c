/* mdp.c

 * Copyright 1995,1996,1997, Brown University, Providence, RI.
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
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "mdp.h"
#include "imm-reward.h"
#include "sparse-matrix.h"

#define MDP_C

#define DOUBLE_DISPLAY_PRECISION                  4

#define EPSILON  0.00001  /* tolerance for sum of probs == 1 */

/* To indicate whether we are using an MDP or POMDP. 
   */
Problem_Type gProblemType = UNKNOWN_problem_type;

/* The discount factor to be used with the problem.  
   */
double gDiscount = DEFAULT_DISCOUNT_FACTOR;

Value_Type gValueType = DEFAULT_VALUE_TYPE;

/* These specify the size of the problem.  The first two are always required.
   */
int gNumStates = 0;
int gNumActions = 0;
int gNumObservations = 0;   /* remains zero for MDPs */

/*  We need two sets of variable for the probabilities and values.  The first
    is an intermediate representation which is filled in as the MDP file
    is parsed, and the other is the final sparse reprsentation which is
    found by converting the interemediate representation.  As aresult, we 
    only need to allocate the intermediate memory while parsing. After parsing
    is completed and we are ready to convert it into the final sparse 
    representation, then we allocate the rest of the memory.
    */

/* Intermediate variables */

I_Matrix *IP;  /* Transition Probabilities */

I_Matrix *IR;  /* Observation Probabilities (POMDP only) */

I_Matrix *IEQ; /* Extended Rewards */
I_Matrix IQ;  /* Immediate action-state pair values (both MDP and POMDP) */

/* Sparse variables */

Matrix *P;  /* Transition Probabilities */

Matrix *R;  /* Observation Probabilities */

Matrix *EQ; /* Extended Rewards: r(s,a,s') */
Matrix Q;  /* Immediate values for state action pairs.  These are
	    expectations computed from immediate values. */

/* Normal variables */

/* Some type of algorithms want a place to start off the problem,
   especially when doing simulation type experiments.  The belief
   state is for POMDPs and the initial state for an MDP */

double *gInitialBelief; 
int gInitialState = INVALID_STATE;

/***************************************************************************/
double *newBeliefState(  ) {

  return( (double *) calloc( gNumStates, sizeof( double )));
}  /* *newBeliefState */
/***************************************************************************/
int transformBeliefState( double *pi, double *pi_hat, int a, int obs ) {
  double denom;
  int i, j, z, cur_state, next_state;

  if( gProblemType != POMDP_problem_type ) return(0);

  /* zero out all elements since we will acumulate probabilities as we loop */
  for( i = 0; i < gNumStates; i++ ) pi_hat[i] = 0.0;

  for( cur_state = 0; cur_state < gNumStates; cur_state++ ) {
    for( j = P[a]->row_start[cur_state]; j < P[a]->row_start[cur_state]+P[a]->row_length[cur_state]; j++ ) {
      next_state = P[a]->col[j];
      pi_hat[next_state] += pi[cur_state] * P[a]->mat_val[j] * getEntryMatrix( R[a], next_state, obs );
    } /* for j */
  }  /* for i */

  /* Normalize */
  denom = 0.0;
  for( i = 0; i < gNumStates; i++ ) denom += pi_hat[i];
  if( IS_ZERO( denom )) return(0);
  for( i = 0; i < gNumStates; i++ ) pi_hat[i] /= denom;
  return(1);
}  /* transformBeliefState */
/**********************************************************************/
void copyBeliefState( double *copy, double *pi ) {
/*
*/
   int i;

   if(( pi == NULL) || (copy == NULL ))
      return;

   for( i = 0; i < gNumStates; i++ )
      copy[i] = pi[i];

}  /* copyBeliefState */
/**********************************************************************/
void displayBeliefState( FILE *file, double *pi ) {
   int i;
   
   fprintf( file, "[%.*lf", DOUBLE_DISPLAY_PRECISION, pi[0] );
   for(i = 1; i < gNumStates; i++) {
      fprintf(file, " ");
      fprintf( file, "%.*lf", DOUBLE_DISPLAY_PRECISION, pi[i] );
   }  /* for i */
      fprintf(file, "]");
}  /* displayBeliefState */
/***************************************************************************/
int readMDP( char *filename ) {
/*
   This routine returns 1 if the file is successfully parsed and 0 if not.
*/

   FILE *file;
   extern int readMDPFile( FILE * );

   if( filename == NULL ) {
      fprintf( stderr, "<NULL> MDP filename: %s.\n", filename );
      return( 0 );
   }

   if(( file = fopen( filename, "r" )) == NULL ) {
      fprintf( stderr, "Cannot open the MDP file: %s.\n", filename );
      return( 0 );
   }

   if( readMDPFile( file ) == 0 ) {
      fprintf( stderr, 
              "MDP file '%s' was not successfully parsed!\n", filename );
      return( 0 );
   }

   fclose( file );

   /* After the file has been parsed, we should have everything we need
      in the final representation.  
      */
   return( 1 );
}  /* readMDP */
/**********************************************************************/
void allocateIntermediateMDP() {
/*
   Assumes that the gProblemType has been set and that the variables
   gNumStates, gNumActions, and gNumObservation have the appropriate 
   values.  It will allocate the memory that will be needed to store
   the problem.  This allocates the space for the intermediate 
   representation representation for the transitions and observations,
   the latter for POMDPs only.
*/

  int a;

  /* We need an intermediate matrix for transition probs. for each
     action.  */
  IP = (I_Matrix *) malloc( gNumActions * sizeof( *IP ));
  
  for( a = 0; a < gNumActions; a++ )
    IP[a] = newIMatrix( gNumStates );

  /* Only need observation probabilities if it is a POMDP */
  if( gProblemType == POMDP_problem_type ) {
   
    /* We need an intermediate matrix for observation probs. for each
       action.  */
    IR = (I_Matrix *) malloc( gNumActions * sizeof( *IR ));
  
    for( a = 0; a < gNumActions; a++ )
      IR[a] = newIMatrix( gNumStates );

    /* Note that the immediate values are stored in a special way, so
       we do not need to allocate anything at this time. */

    /* For POMDPs, we will keep a starting belief state, since many */
    /* type of algorithms use a simulation approach and would want to */
    /* start it in a particular place. This is not kept in a sparse */
    /* way, so it is just a vector of the number of states. We */
    /* initialize it to be all zeroes.  */

    gInitialBelief = (double *) calloc( gNumStates, sizeof( double ));

  }  /* if POMDP */

  /* Regardless of whether there is an MDP or POMDP, the immediate
     rewards for action-state pairs will always exist as an expectation
     over the next states and possibly actions.  These will be computed
     after parsing from the special immediate reward representation.
     */

  IQ = newIMatrix( gNumActions );

  IEQ = (I_Matrix*)malloc(gNumActions*sizeof(*IEQ));
  for( a = 0; a < gNumActions; ++a ) IEQ[a] = newIMatrix(gNumStates);
  
} /* allocateIntermediateMDP */
/************************************************************************/
int verifyIntermediateMDP() {
/*
   This routine will make sure that the intermediate form for the MDP
   is valid.  It will check to make sure that the transition and
   observation matrices do indeed specify probabilities.

   There is a similar routine in the parser.y file, which is nicer
   when parsing a POMDP file, but this routine is needed when we 
   are creating the POMDP through a program.  In this case there
   will be no parsing and thus no logging of errors.
*/
   int a,i,j,obs;
   double sum;
   
   for( a = 0; a < gNumActions; a++ )
      for( i = 0; i < gNumStates; i++ ) {
	 sum = sumIMatrixRowValues( IP[a], i );
         if((sum < ( 1.0 - EPSILON)) || (sum > (1.0 + EPSILON))) {
	   return( 0 );
         }
      } /* for i */

   if( gProblemType == POMDP_problem_type )
     for( a = 0; a < gNumActions; a++ )
       for( j = 0; j < gNumStates; j++ ) {
	 sum = sumIMatrixRowValues( IR[a], j );
         if((sum < ( 1.0 - EPSILON)) || (sum > (1.0 + EPSILON))) {
	   return( 0 );
         } /* if sum not == 1 */
       }  /* for j */

  return( 1 );
}  /* verifyIntermediateMDP */
/************************************************************************/
void deallocateIntermediateMDP() {
/*
   This routine is made available in case something goes wrong
   before converting the matrices from the intermediate form
   to the final form.  Normally the conversion routine convertMatrices()
   will deallocate the intermediate matrices, but it might be desirable
   to get rid of them before converting (especially if something
   has gone wrong) so that things can be started over.
*/
  int a;

  for( a = 0; a < gNumActions; a++ ) {

    destroyIMatrix( IP[a] );

    if( gProblemType == POMDP_problem_type ) {
      destroyIMatrix( IR[a] );
    }

  }

  free( IP );
  
  if( gProblemType == POMDP_problem_type ) {
    free( IR );
    free( gInitialBelief );
  }

  destroyIMatrix( IQ );

  for( a = 0; a < gNumActions; ++a )
    destroyIMatrix(IEQ[a]);
  free(IEQ);

}  /* deallocateIntermediateMDP */
/**********************************************************************/
void computeRewards() {
  int a, i, j, z, next_state, obs;
  double sum, inner_sum, rew;

  /* Now do the expectation thing for action-state reward values */

  for( a = 0; a < gNumActions; a++ )
    for( i = 0; i < gNumStates; i++ ) {
      sum = 0.0;
      /* Note: 'j' is not a state. It is an index into an array */
      for( j = P[a]->row_start[i]; j < P[a]->row_start[i]+P[a]->row_length[i]; j++ ) {
	next_state = P[a]->col[j];
	if( gProblemType == POMDP_problem_type ) {
	  inner_sum = 0.0;
	  /* Note: 'z' is not a state. It is an index into an array */
	  for( z = R[a]->row_start[next_state]; z < R[a]->row_start[next_state]+R[a]->row_length[next_state]; z++ ) {
	    obs = R[a]->col[z];
            rew = getImmediateReward(a,i,next_state,obs);
	    inner_sum += R[a]->mat_val[z] * rew;
	  }  /* for z */
	}  /* if POMDP */
	else { /* it is an MDP */
	  inner_sum = getImmediateReward(a,i,next_state,0);
        }
	sum += P[a]->mat_val[j] * inner_sum;

        /* set extended reward r(s,a,s') */ 
        if( inner_sum != 0.0 ) {
          addEntryToIMatrix(IEQ[a],i,next_state,inner_sum);
        }
      }  /* for j */
      addEntryToIMatrix( IQ, a, i, sum );
    }  /* for i */
  
}  /* computeRewards */
/************************************************************************/
void convertMatrices() {
/*
   This routine is called after the parsing has been succesfully done.
   It will assume that the intermediate representations for the transition
   and observation matrices have been allocated and had their values set.
   It also assumes that the special immediate reward representation
   has been set.  

   This routine will do two functions.  It will convert the intermediate
   sparse representations for the transitions and observations to the 
   actual true sparse representation.  It will also compute the action-state
   immeidate reward pairs as an expectation over next states and possibly
   observations from the special immediate reward representation.  This
   will be the final step toward the use of the MDP/POMDP model in 
   computation.
   */

  int a;

  /* Allocate room for each action */
  P = (Matrix *) malloc( gNumActions * sizeof( *P ) );
  R = (Matrix *) malloc( gNumActions * sizeof( *R ) );

  /* First convert the intermediate sparse matrices for trans. and obs. */

  for( a = 0; a < gNumActions; a++ ) {

    P[a] = transformIMatrix( IP[a] );
    destroyIMatrix( IP[a] );

    if( gProblemType == POMDP_problem_type ) {
      R[a] = transformIMatrix( IR[a] );
      destroyIMatrix( IR[a] );
    }

  }

  free( IP );
  
  if( gProblemType == POMDP_problem_type )
    free( IR );

  /* allocare space for extended rewards */
  EQ = (Matrix*)malloc(gNumActions*sizeof(*EQ));

  /* Calculate expected immediate rewards for action-state pairs, but
     do it in the sparse matrix representation to eliminate zeroes */

  computeRewards();

  /* Then convert it into the real representation */
  Q = transformIMatrix( IQ );
  destroyIMatrix( IQ );

  for( a = 0; a < gNumActions; ++a ) {
    EQ[a] = transformIMatrix(IEQ[a]);
    destroyIMatrix(IEQ[a]);
  }

}

int writeMDP( char *filename ) {
  FILE *file = fopen(filename,"w");
  if( file == NULL ) return(0);

  fprintf(file,"discount: %.6lf\n",gDiscount);

  if( gValueType == COST_value_type )
    fprintf(file,"values: cost\n");
  else
    fprintf(file,"values: reward\n");

  fprintf(file,"states: %d\n",gNumStates);
  fprintf(file,"actions: %d\n",gNumActions);

  if( gProblemType == POMDP_problem_type )
    fprintf(file,"observations: %d\n",gNumObservations);
  
  for( int a = 0; a < gNumActions; a++ ) {
    for( int i = 0; i < gNumStates; i++ ) {
      for( int j = P[a]->row_start[i]; j < P[a]->row_start[i] +  P[a]->row_length[i]; j++ ) {
	fprintf(file,"T: %d : %d : %d %.6lf\n",a,i,P[a]->col[j],P[a]->mat_val[j]);
      }
    }
  }
  
  if( gProblemType == POMDP_problem_type ) {
    for( int a = 0; a < gNumActions; a++ ) {
      for( int j = 0; j < gNumStates; j++ ) {
	for( int obs = R[a]->row_start[j]; obs < R[a]->row_start[j] +  R[a]->row_length[j]; obs++ ) {
	  fprintf(file,"O: %d : %d : %d %.6lf\n",a,j,R[a]->col[obs],R[a]->mat_val[obs]);
        }
      }
    }
  }
 
  if( gProblemType == POMDP_problem_type ) {
    for( int a = 0; a < gNumActions; a++ ) {
      for( int i = Q->row_start[a]; i < Q->row_start[a] +  Q->row_length[a]; i++ ) {
	fprintf(file,"R: %d : %d : * : * %.6lf\n",a,Q->col[i],Q->mat_val[i]);
      }
    }
  }
  else {
    for( int a = 0; a < gNumActions; a++ ) {
      for( int i = Q->row_start[a]; i < Q->row_start[a] +  Q->row_length[a]; i++ ) {
	fprintf(file,"R: %d : %d : * %.6lf\n",a,Q->col[i],Q->mat_val[i]);
      }
    }
  }

  fclose(file);
  return(1);
}

int writeTransPOMDP( std::ostream &os, double reward_shift )
{
  int absorbing = gNumStates;
  int abs_obs = gNumObservations;
  extern int writeImmRewards(std::ostream&,double);

  os.precision(6);
  //os << "discount: " << gDiscount << std::endl;
  os << "discount: 1.0" << std::endl;
  os << "values: reward" << std::endl;
  os << "states: " << 1+gNumStates << std::endl;
  os << "actions: " << gNumActions << std::endl;
  os << "observations: " << 1+gNumObservations << std::endl;
 
  os << std::endl << "# Transitions" << std::endl;
  for( int a = 0; a < gNumActions; a++ ) {
    for( int i = 0; i < gNumStates; i++ ) {
      for( int j = P[a]->row_start[i]; j < P[a]->row_start[i] +  P[a]->row_length[i]; j++ ) {
        double p = P[a]->mat_val[j]*gDiscount;
        os << "T: " << a << " : " << i << " : " << P[a]->col[j] << " " << p << std::endl;
      }
      os << "T: " << a << " : " << i << " : " << absorbing << " " << 1.0-gDiscount << std::endl;
    }
    os << "T: " << a << " : " << absorbing << " : " << absorbing << " " << 1.0 << std::endl;
  }
  
  os << std::endl << "# Observations" << std::endl;
  for( int a = 0; a < gNumActions; a++ ) {
    for( int j = 0; j < gNumStates; j++ ) {
      for( int obs = R[a]->row_start[j]; obs < R[a]->row_start[j] +  R[a]->row_length[j]; obs++ ) {
        os << "O: " << a << " : " << j << " : " << R[a]->col[obs] << " " << R[a]->mat_val[obs] << std::endl;
      }
    }
    os << "O: " << a << " : " << absorbing << " : " << abs_obs << " " << 1.0 << std::endl;
  }

  os << std::endl << "# Rewards" << std::endl;
  os << "R: * : * : * : * " << reward_shift << std::endl;
  writeImmRewards(os,reward_shift);
  os << "R: * : " << absorbing << " : * : * 0.0" << std::endl;

  return(1);
}

void deallocateMDP() {
  int a;

  for( a = 0; a < gNumActions; a++ ) {
    destroyMatrix( P[a] );
    
    if( gProblemType == POMDP_problem_type )
      destroyMatrix( R[a] );
  }  /* for a */

  free( P );

  if( gProblemType == POMDP_problem_type ) {
    free( R );
    free( gInitialBelief );
  }

  destroyMatrix( Q );

  destroyImmRewards();

}  /* deallocateMDP */
/**********************************************************************/
void displayMDPSlice( int state ) {
/*
   Shows the transition and observation probabilites (and rewards) for
   the given state.
*/
   int a, j, obs;

   if(( state < 0 ) || ( state >= gNumStates ) || ( gNumStates < 1 ))
      return;

   printf( "MDP slice for state: %d\n", state );

   for( a = 0; a < gNumActions; a++ )
      for( j = P[a]->row_start[state]; 
          j < P[a]->row_start[state] +  P[a]->row_length[state];
          j++ ) 
         printf( "\tP( s=%d | s=%d, a=%d ) = %.6lf\n",
                P[a]->col[j], state, a, P[a]->mat_val[j] );
  
   if( gProblemType == POMDP_problem_type )
      for( a = 0; a < gNumActions; a++ )
         for( obs = R[a]->row_start[state]; 
             obs < R[a]->row_start[state] +  R[a]->row_length[state];
             obs++ ) 
            printf( "\tP( o=%d | s=%d, a=%d ) = %.6lf\n",
                   R[a]->col[obs], state, a, R[a]->mat_val[obs] );
   
   for( a = 0; a < gNumActions; a++ )
      printf( "\tQ( s=%d, a=%d ) = %5.6lf\n",
             state, a, getEntryMatrix( Q, a, state ));
   
}  /* displayMDPSlice */
/**********************************************************************/


