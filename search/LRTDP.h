/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.6 $  $Author: trey $  $Date: 2006-04-04 17:24:52 $
   
 @file    LRTDP.h
 @brief   Implementation of Bonet and Geffner's LRTDP algorithm.

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

#ifndef INCLRTDP_h
#define INCLRTDP_h

#include "RTDPCore.h"

namespace zmdp {

struct LRTDPExtraNodeData {
  bool isSolved;
};

struct LRTDP : public RTDPCore {
  LRTDP(AbstractBound* _initUpperBound);

  void getNodeHandler(MDPNode& cn);
  static void staticGetNodeHandler(MDPNode& cn, void* handlerData);
  static bool& getIsSolved(const MDPNode& cn);
  void cacheQ(MDPNode& cn);
  double residual(MDPNode& cn);
  bool checkSolved(MDPNode& cn);

  bool getUseLowerBound(void) const { return false; }
  void updateInternal(MDPNode& cn);
  bool trialRecurse(MDPNode& cn, int depth);
  bool doTrial(MDPNode& cn);
  void derivedClassInit(void);
};

}; // namespace zmdp

#endif /* INCLRTDP_h */

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.5  2006/02/27 20:12:36  trey
 * cleaned up meta-information in header
 *
 * Revision 1.4  2006/02/19 18:35:09  trey
 * targetPrecision now stared as a field rather than passed around recursively
 *
 * Revision 1.3  2006/02/14 19:34:34  trey
 * now use targetPrecision properly
 *
 * Revision 1.2  2006/02/13 20:20:33  trey
 * refactored some common code from RTDP and LRTDP
 *
 * Revision 1.1  2006/02/13 19:09:24  trey
 * initial check-in
 *
 *
 ***************************************************************************/
