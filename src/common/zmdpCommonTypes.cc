/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    commonTypes.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>

#include "zmdpCommonTypes.h"

using namespace std;

namespace zmdp {

ostream& operator<<(ostream& out, const ValueInterval& v) {
  out << "[" << v.l << " .. " << v.u << "]";
  return out;
}

};

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.1  2005/10/21 20:11:52  trey
 * renamed to avoid potential conflicts with other projects
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 *
 ***************************************************************************/
