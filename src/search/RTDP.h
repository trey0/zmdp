/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.11 $  $Author: trey $  $Date: 2006-10-20 04:56:15 $
   
 @file    RTDP.h
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

#ifndef INCRTDP_h
#define INCRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct RTDP : public RTDPCore {
  RTDP(void);

  void trialRecurse(MDPNode& cn, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.10  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.9  2006/04/07 19:42:42  trey
 * removed initUpperBound argument to constructor
 *
 * Revision 1.8  2006/04/03 21:39:24  trey
 * updated to use IncrementalBounds
 *
 * Revision 1.7  2006/02/19 18:33:47  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.6  2006/02/14 19:34:43  trey
 * now use targetPrecision properly
 *
 * Revision 1.5  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.4  2006/02/11 22:38:10  trey
 * moved much of the RTDP implementation into RTDPCore, where it can be shared by many RTDP variants
 *
 * Revision 1.3  2006/02/10 20:14:33  trey
 * standardized fields in bounds.plot
 *
 * Revision 1.2  2006/02/10 19:33:54  trey
 * added numTrials, useful in debug output
 *
 * Revision 1.1  2006/02/09 21:59:04  trey
 * initial check-in
 *
 *
 ***************************************************************************/
