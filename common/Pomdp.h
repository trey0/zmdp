/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-10-27 21:38:16 $
 *  
 * @file    Pomdp.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCPomdp_h
#define INCPomdp_h

#include <iostream>
#include <string>
#include <vector>

#include "pomdpCommonDefs.h"
#include "pomdpCommonTypes.h"
#include "pomdpSmartRef.h"

using namespace MATRIX_NAMESPACE;

namespace pomdp {

// this is a wrapper class around Pomdp that uses Lapack matrices
class Pomdp {
public:
  int numStates, numActions, numObservations;
  double discount;
  // initialBelief(s)

  cvector initialBelief;
  // R(s,a)
  cmatrix R;
  // T[a](s,s'), Ttr[a](s',s), O[a](s,o)
  std::vector<cmatrix> T, Ttr, O;
  std::vector<bool> isTerminalState;

  void readFromFile(const std::string& fileName,
		    bool useFastParser = false);

protected:
  void readFromFileCassandra(const std::string& fileName);
  void readFromFileFast(const std::string& fileName);

  void debugDensity(void);
};

typedef Pomdp* PomdpP;

}; // namespace pomdp

#endif // INCPomdp_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2005/10/21 20:08:41  trey
 * added namespace pomdp
 *
 * Revision 1.6  2005/03/10 22:53:32  trey
 * now initialize T matrix even when using sla
 *
 * Revision 1.5  2005/01/27 05:32:02  trey
 * switched to use Ttr instead of T under sla
 *
 * Revision 1.4  2005/01/26 04:10:48  trey
 * modified problem reading to work with sla
 *
 * Revision 1.3  2005/01/21 15:21:19  trey
 * added readFromFileFast
 *
 * Revision 1.2  2004/11/24 20:50:16  trey
 * switched PomdpP to be a pointer, not a SmartRef
 *
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 * Revision 1.2  2004/11/09 21:31:59  trey
 * got pomdp source tree into a building state again
 *
 * Revision 1.1.1.1  2004/11/09 16:18:56  trey
 * imported hsvi into new repository
 *
 * Revision 1.5  2003/09/16 00:57:02  trey
 * lots of performance tuning, mostly fixed rising upper bound problem
 *
 * Revision 1.4  2003/09/11 01:46:42  trey
 * completed conversion to compressed matrices
 *
 * Revision 1.3  2003/09/07 02:28:07  trey
 * started to adapt for boost matrix library
 *
 * Revision 1.2  2003/07/16 16:07:36  trey
 * added isTerminalState
 *
 * Revision 1.1  2003/06/26 15:41:22  trey
 * C++ version of pomdp solver functional
 *
 *
 ***************************************************************************/

