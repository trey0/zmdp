/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2006-02-20 00:04:49 $
   
 @file    HDP.h
 @brief   Implementation of Bonet and Geffner's HDP algorithm
          (as presented at IJCAI in 2003).

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

#ifndef INCHDP_h
#define INCHDP_h

#include "RTDPCore.h"

namespace zmdp {

struct HDP : public RTDPCore {
  int index;
  NodeStack nodeStack;
  std::stack<MDPNode*> visited;

  HDP(AbstractBound* _initUpperBound);

  void cacheQ(MDPNode& cn);
  double residual(MDPNode& cn);

#if USE_HDP_LOWER_BOUND
  bool getUseLowerBound(void) const { return true; }
#else
  bool getUseLowerBound(void) const { return false; }
#endif
  void updateInternal(MDPNode& cn);
  bool trialRecurse(MDPNode& cn, int depth);
  bool doTrial(MDPNode& cn);
};

}; // namespace zmdp

#endif /* INCHDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/02/19 18:33:35  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.1  2006/02/17 18:20:55  trey
 * initial check-in
 *
 *
 ***************************************************************************/
