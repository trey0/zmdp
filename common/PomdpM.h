/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.4 $  $Author: trey $  $Date: 2005-01-26 04:10:48 $
 *  
 * @file    PomdpM.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCPomdpM_h
#define INCPomdpM_h

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <string>
#include <vector>

#include "commonDefs.h"
#include "commonTypes.h"
#include "SmartRef.h"

using namespace MATRIX_NAMESPACE;

// this is a wrapper class around Pomdp that uses Lapack matrices
class PomdpM {
public:
  int numStates, numActions, numObservations;
  double discount;
  // initialBelief(s)

  cvector initialBelief;
  // R(s,a)
  cmatrix R;
  // T[a](s,s'), O[a](s,o)
  std::vector<cmatrix> T, O;
#if USE_UBLAS
  std::vector<cmatrix> Ttr, Otr;
#endif

  std::vector<bool> isTerminalState;

  void readFromFile(const std::string& fileName,
		    bool useFastParser = false);

protected:
  void readFromFileCassandra(const std::string& fileName);
  void readFromFileFast(const std::string& fileName);

  void debugDensity(void);
};

//typedef SmartRef<PomdpM> PomdpP;
typedef PomdpM* PomdpP;

#endif // INCPomdpM_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
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

