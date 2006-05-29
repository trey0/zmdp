/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-05-29 04:06:02 $
   
 @file    decision-tree.h
 @brief   No brief

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

typedef struct DTNodeStruct DTNode;
typedef struct DTTableStruct DTTable;

extern void dtInit(int numActions, int numStates, int numObservations);
extern void dtAdd(int action, int cur_state, int next_state, int obs, double val);
extern double dtGet(int action, int cur_state, int next_state, int obs);
extern void dtDeallocate(void);

extern void dtDebugPrint(const char* header);

#endif // INCdecision_tree_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
