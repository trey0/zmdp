/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-15 16:24:27 $
   
 @file    FRTDP.h
 @brief   No brief

 Copyright (c) 2006, Trey Smith. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * The software may not be sold or incorporated into a commercial
   product without specific prior written permission.
 * The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 ***************************************************************************/

#ifndef INCFRTDP_h
#define INCFRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct FRTDP : public RTDPCore {
  FRTDP(AbstractBound* _initUpperBound);

  int getMaxPrioOutcome(MDPNode& cn, int a,
			double* maxPrioP = NULL,
			double* secondBestPrioP = NULL) const;

  bool getUseLowerBound(void) const { return true; }
  void updateInternal(MDPNode& cn);
  void trialRecurse(MDPNode& cn, double occ, double altPrio, int depth);
  bool doTrial(MDPNode& cn, double pTarget);
};

}; // namespace zmdp

#endif /* INCRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.1  2006/02/13 21:46:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/
