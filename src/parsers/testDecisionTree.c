/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-09-21 16:32:28 $
   
 @file    testDecisionTree.c
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

#include <stdio.h>
#include <assert.h>

#include "decision-tree.h"

void testOnce(void)
{
  double result;

  /* set up table */
  dtInit(5, 5, 5);

  dtAdd(0, 1, 2, 3, 0.5);
  dtAdd(0, 1, 2, 3, 0.7);

  dtAdd(0, 1, 3, -1, -10.1);
  dtAdd(0, 1, 3, -1, -10.3);

  dtAdd(1, 0, 0,  -1, 6.0);
  dtAdd(1, 0, -1, -1, 7.0);

  dtAdd(-1, 2, -1, -1, 2.7);

  /* do a few queries */
  result = dtGet(0, 1, 2, 3);
  printf("expecting: result=%lf\n", 0.7);
  printf("got:       result=%lf\n", result);

  result = dtGet(0, 1, 3, 1);
  printf("expecting: result=%lf\n", -10.3);
  printf("got:       result=%lf\n", result);

  result = dtGet(1, 0, 0, 0);
  printf("expecting: result=%lf\n", 7.0);
  printf("got:       result=%lf\n", result);

  result = dtGet(3, 2, 0, 0);
  printf("expecting: result=%lf\n", 2.7);
  printf("got:       result=%lf\n", result);

  /* clean up */
  dtDeallocate();
}

int main(int argc, char** argv)
{
  testOnce();
  testOnce();

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/05/29 05:48:11  trey
 * added more tests
 *
 * Revision 1.1  2006/05/29 05:02:52  trey
 * initial check-in
 *
 *
 ***************************************************************************/
