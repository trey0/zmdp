/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2007-03-06 06:57:08 $
   
 @file    PomdpExec.h
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

#ifndef INCPomdpExec_h
#define INCPomdpExec_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

#include "Pomdp.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace zmdp {

struct PomdpExecCore {
  virtual ~PomdpExecCore(void) {}

  virtual void setToInitialBelief(void) = 0;
  virtual int chooseAction(void) = 0;
  virtual void advanceToNextBelief(int a, int o) = 0;
};

// PomdpExec adds some default class members to PomdpExecCore,
// makes inheritance easier in some cases.
struct PomdpExec : public PomdpExecCore {
  Pomdp* pomdp;
  bool currentBeliefInitialized;
  belief_vector currentBelief;

  PomdpExec(void);

  // default implementation
  bool getBeliefIsTerminal(void) const;

  // helpful for debugging
  belief_vector& getBelief(belief_vector& b) const;
  int getRandomObservation(int a) const;
};

}; // namespace zmdp

#endif // INCPomdpExec_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2006/06/27 18:20:18  trey
 * turned PomdpExec into an abstract class; most of the original implementation is now in the derived class MaxPlanesLowerBoundExec
 *
 * Revision 1.1  2006/06/24 16:25:28  trey
 * initial check-in
 *
 *
 ***************************************************************************/
