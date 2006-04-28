/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    ValueFunction.h
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

#ifndef INCValueFunction_h
#define INCValueFunction_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <string>
#include <vector>
#include <ext/hash_map>

#include "zmdpCommonDefs.h"
#include "zmdpCommonTypes.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

class ValueFunction {
public:
  int numStates;

  ValueFunction(void) : numStates(-1) {}
  virtual ~ValueFunction(void) {}

  virtual ValueInterval getValueAt(const belief_vector& b) const = 0;
  bool consistentWith(const ValueFunction& rhs, int numSamples,
		      bool debug = false) const;
};

}; // namespace zmdp

#endif // INCValueFunction_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/04/04 17:21:49  trey
 * moved ValueFunction from common to bounds
 *
 * Revision 1.5  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.4  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.3  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.2  2005/10/21 20:10:10  trey
 * added namespace zmdp
 *
 * Revision 1.1  2004/11/24 20:48:04  trey
 * moved to common from hsvi
 *
 *
 ***************************************************************************/
