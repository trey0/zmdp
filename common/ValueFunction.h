/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-21 20:10:10 $
 *  
 * @file    ValueFunction.h
 * @brief   No brief
 ***************************************************************************/

#ifndef INCValueFunction_h
#define INCValueFunction_h

/**********************************************************************
 * INCLUDES
 **********************************************************************/

// this causes problems if it is included after the Lapack headers, so
//  pre-emptively include it here.  not sure exactly what the problem is.
#include <iostream>

#include <string>
#include <vector>
#include <ext/hash_map>

#include "pomdpCommonDefs.h"
#include "pomdpCommonTypes.h"

/**********************************************************************
 * CLASSES
 **********************************************************************/

namespace pomdp {

class ValueFunction {
public:
  int numStates;

  ValueFunction(void) : numStates(-1) {}
  virtual ~ValueFunction(void) {}

  virtual ValueInterval getValueAt(const belief_vector& b) const = 0;
  bool consistentWith(const ValueFunction& rhs, int numSamples,
		      bool debug = false) const;
};

}; // namespace pomdp

#endif // INCValueFunction_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/11/24 20:48:04  trey
 * moved to common from hsvi
 *
 *
 ***************************************************************************/
