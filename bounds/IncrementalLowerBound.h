/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2007-03-22 23:57:16 $
   
 @file    IncrementalLowerBound.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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

#ifndef INCIncrementalLowerBound_h
#define INCIncrementalLowerBound_h

#include <iostream>
#include <string>
#include <vector>

#include "AbstractBound.h"
#include "MDPCache.h"

using namespace sla;

namespace zmdp {

struct IncrementalLowerBound : public AbstractBound {
  virtual void initNodeBound(MDPNode& cn) = 0;
  virtual void update(MDPNode& cn) = 0;
  virtual int chooseAction(const state_vector& s) {
    // signal to fall back to default implementation if derived class
    // does not implement chooseAction()
    return -1;
  }
};

}; // namespace zmdp

#endif // INCIncrementalLowerBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/10/24 02:06:35  trey
 * initial check-in
 *
 *
 ***************************************************************************/

