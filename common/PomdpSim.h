/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.7 $  $Author: trey $  $Date: 2006-01-28 03:04:39 $
   
 @file    PomdpSim.h
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

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

#ifndef INCPomdpSim_h
#define INCPomdpSim_h

#include "AbstractSim.h"
#include "Pomdp.h"

namespace pomdp {

class PomdpSim : public AbstractSim {
public:
  Pomdp* pomdp;
  double rewardSoFar;
  int elapsedTime;
  int state;
  cvector currentBelief;
  int lastState;
  std::ostream *simOutFile;
  
  PomdpSim(Pomdp* _pomdp);

  const MDP* getModel(void) const { return pomdp; }
  void restart(void);
  void performAction(int a);
  const state_vector& getInformationState(void) const { return currentBelief; }
};

}; // namespace pomdp

#endif // INCPomdpSim_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.5  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.4  2005/10/27 22:09:21  trey
 * rename PomdpM to Pomdp
 *
 * Revision 1.3  2005/10/21 20:08:53  trey
 * added namespace pomdp
 *
 * Revision 1.2  2005/01/26 04:11:31  trey
 * replaced bvector with cvector
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/06 21:51:53  trey
 * many changes
 *
 * Revision 1.2  2003/07/23 20:49:17  trey
 * published/fixed sparseRep
 *
 * Revision 1.1  2003/07/16 16:09:35  trey
 * initial check-in
 *
 *
 ***************************************************************************/
