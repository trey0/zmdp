/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2006-04-07 19:38:49 $
   
 @file    PointBounds.h
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

#ifndef INCPointBounds_h
#define INCPointBounds_h

#include <iostream>
#include <string>
#include <vector>

#include "IncrementalBounds.h"

using namespace sla;

namespace zmdp {

struct PointBounds : public IncrementalBounds {
  const MDP* problem;
  AbstractBound* initLowerBound;
  AbstractBound* initUpperBound;
  double targetPrecision;
  MDPNode* root;
  MDPHash* lookup;

  PointBounds(void);

  // helper functions
  void updateValuesUB(MDPNode& cn, int* maxUBActionP);
  void updateValuesBoth(MDPNode& cn, int* maxUBActionP);

  // must be called before initialize()
  void setBounds(AbstractBound* _initLowerBound,
		 AbstractBound* _initUpperBound);

  // implementations of virtual functions declared in IncrementalBounds
  void initialize(const MDP* _problem,
		  double _targetPrecision);

  MDPNode* getRootNode(void);
  MDPNode* getNode(const state_vector& s);
  void expand(MDPNode& cn);
  void update(MDPNode& cn, int* maxUBActionP);
  int chooseAction(const state_vector& s) const;
  ValueInterval getValueAt(const state_vector& s) const;
};

}; // namespace zmdp

#endif // INCPointBounds_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/04/06 20:33:51  trey
 * moved setGetNodeHandler() implementation from PointBounds -> IncrementalBounds
 *
 * Revision 1.2  2006/04/05 21:34:40  trey
 * changed initialization to match new IncrementalBounds API
 *
 * Revision 1.1  2006/04/04 17:22:51  trey
 * initial check-in
 *
 *
 ***************************************************************************/

