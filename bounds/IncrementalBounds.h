/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-04-27 22:58:37 $
   
 @file    IncrementalBounds.h
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

#ifndef INCIncrementalBounds_h
#define INCIncrementalBounds_h

#include <iostream>
#include <string>
#include <vector>

#include "MDPCache.h"

using namespace sla;

namespace zmdp {

typedef void (*GetNodeHandler)(MDPNode& cn, void* callbackData);

struct IncrementalBounds {
  int numStatesTouched;
  int numStatesExpanded;
  int numBackups;
  GetNodeHandler getNodeHandler;
  void* handlerData;

  virtual ~IncrementalBounds(void) {}

  virtual void initialize(const MDP* _problem,
			  double _targetPrecision) = 0;

  virtual MDPNode* getRootNode(void) = 0;
  virtual MDPNode* getNode(const state_vector& s) = 0;
  virtual void expand(MDPNode& cn) = 0;
  virtual void update(MDPNode& cn, int* maxUBActionP) = 0;
  virtual int chooseAction(const state_vector& s) const = 0;
  virtual ValueInterval getValueAt(const state_vector& s) const = 0;

  virtual bool getSupportsPolicyOutput(void) const { return false; }
  virtual void writePolicy(const std::string& outFileName) { assert(0); }

  void setGetNodeHandler(GetNodeHandler getNodeHandler, void* handlerData);

  // relies on correct cached Q values!
  static int getMaxUBAction(MDPNode& cn);

  static int getSimulatedOutcome(MDPNode& cn, int a);
};

}; // namespace zmdp

#endif // INCIncrementalBounds_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/04/06 20:33:51  trey
 * moved setGetNodeHandler() implementation from PointBounds -> IncrementalBounds
 *
 * Revision 1.2  2006/04/05 21:33:07  trey
 * made some functions static
 *
 * Revision 1.1  2006/04/04 17:22:51  trey
 * initial check-in
 *
 *
 ***************************************************************************/

