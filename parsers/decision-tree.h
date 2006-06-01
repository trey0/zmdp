/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-06-01 16:48:36 $
   
 @file    decision-tree.h
 @brief   Efficient decision tree data structure for MDP/POMDP immediate
          rewards.

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

#ifndef INCdecision_tree_h
#define INCdecision_tree_h

/**********************************************************************
   See an example of how to use this library in testDecisionTree.c.

   The decision-tree library efficiently stores a mapping [a,s,s',o] ->
   val, where a is an action, s and s' are states, o is an observation
   (all integers), and val is an immediate reward value (real).

   * Construct the immediate reward mapping by calling dtInit() and
     repeatedly calling dtAdd(), one time for each "entry" in the mapping.

   * Query immediate rewards by calling dtGet().

   * When finished, call dtDeallocate().

   The decision-tree library is intended to be used only by the
   imm-reward library.  (The dtGet() function hides behind the
   getImmediateReward() function in imm-reward.c).
 **********************************************************************/

/* Initialize the decision-tree library--dimensionality of the model
   must be specified so that tables in the decision tree can be
   allocated appropriately later. */
extern void dtInit(int numActions, int numStates, int numObservations);

/* Adds an entry to the decision tree.  Any of the first four arguments
   can be given the value -1 (== WILDCARD_SPEC), indicating a wildcard.
   Later calls to dtAdd() overwrite earlier calls. */
extern void dtAdd(int action, int cur_state, int next_state, int obs, double val);

/* Returns the immediate reward for a particular [a,s,s',o] tuple. */
extern double dtGet(int action, int cur_state, int next_state, int obs);

/* Cleans up all decision tree data structures on the heap. */
extern void dtDeallocate(void);

/* Print a textual representation of the decision tree data structure to
   stdout.  Intended for debugging. */
extern void dtDebugPrint(const char* header);

#endif // INCdecision_tree_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/06/01 15:59:55  trey
 * no longer publish unnecessary typedefs in header
 *
 * Revision 1.1  2006/05/29 04:06:02  trey
 * initial check-in
 *
 *
 ***************************************************************************/
