/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    testPQueue.cc
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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
#include "PQueue.h"

using namespace std;
using namespace zmdp;

typedef PQueue<int, double, std::greater<double> > QType;

void fill(QType& q) {
  q.setPriority(0, 0.5);
  q.setPriority(1, 4.5);
  q.setPriority(2, 1.5);
  q.setPriority(3, 6.5);
  q.setPriority(4, 2.5);
  q.setPriority(5, 3.5);
  q.setPriority(6, 8.5);
  q.setPriority(7, 5.5);
  q.setPriority(8, 7.5);
}

int main(int argc, char** argv) {
  QType q;
  fill(q);

  printf("--1\n");
  while (!q.empty()) {
    printf("v=%d prio=%f\n", q.top(), q.getTopPriority());
    q.pop();
  }
		
  fill(q);
  q.setPriority(1, 1.7);
  q.setPriority(3, 2.7);
  q.erase(5);

  printf("--2\n");
  while (!q.empty()) {
    printf("v=%d prio=%f\n", q.top(), q.getTopPriority());
    q.pop();
  }

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.3  2005/12/06 20:28:44  trey
 * now test specifying the comparison operator
 *
 * Revision 1.2  2005/12/05 16:06:16  trey
 * added more tests
 *
 * Revision 1.1  2005/11/29 04:43:19  trey
 * initial check-in
 *
 *
 ***************************************************************************/
