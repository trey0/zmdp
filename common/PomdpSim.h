/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-01-26 04:11:31 $
 *  
 * @file    PomdpSim.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCPomdpSim_h
#define INCPomdpSim_h

#include "PomdpM.h"

class PomdpSim {
public:
  PomdpP pomdp;
  double rewardSoFar;
  int elapsedTime;
  int state;
  cvector currentBelief;
  int lastState;
  bool terminated;
  std::ostream *simOutFile;
  
  PomdpSim(PomdpP _pomdp);

  // sets us back to the initial belief (choosing state randomly according to
  // initial belief) and zeros elapsed time and reward
  void restart(void);

  void performAction(int a);
};

#endif // INCPomdpSim_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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
