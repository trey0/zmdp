/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2006-02-01 01:09:38 $
   
 @file    MDPValueFunction.h
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

#ifndef INCMDPValueFunction_h
#define INCMDPValueFunction_h

#include "Pomdp.h"
#include "ValueFunction.h"

#define MDP_MAX_ITERS (1000000)

namespace zmdp {

class MDPValueFunction : public ValueFunction {
public:
  dvector alpha;
  const Pomdp* pomdp;

  ValueInterval getValueAt(const belief_vector& b) const {
    return ValueInterval(0, inner_prod( alpha, b ));
  }

  void nextAlphaAction(dvector& result, int a);
  double valueIterationOneStep(void);
  void valueIteration(const Pomdp* _pomdp, double eps);
};


void testMDP(void);

}; // namespace zmdp

#endif // INCMDPValueFunction_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006/01/31 18:31:50  trey
 * moved many files from common to pomdpCore
 *
 * Revision 1.10  2006/01/28 22:02:47  trey
 * switched Pomdp* -> const Pomdp* in valueIteration()
 *
 * Revision 1.9  2006/01/28 03:02:45  trey
 * replaced PomdpP -> Pomdp*
 *
 * Revision 1.8  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.7  2005/10/28 02:51:40  trey
 * added copyright headers
 *
 * Revision 1.6  2005/10/27 22:06:19  trey
 * renamed PomdpM to Pomdp
 *
 * Revision 1.5  2005/10/21 20:07:17  trey
 * added namespace zmdp
 *
 * Revision 1.4  2005/03/28 18:11:21  trey
 * made MDP_MAX_ITERS effectively infinite
 *
 * Revision 1.3  2005/02/08 23:53:04  trey
 * updated to work for alpha_vector = cvector
 *
 * Revision 1.2  2005/01/27 05:30:10  trey
 * modified for sla compatibility
 *
 * Revision 1.1  2004/11/24 20:48:04  trey
 * moved to common from hsvi
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.3  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.2  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.1  2003/06/26 15:41:20  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/
