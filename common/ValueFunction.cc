/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.3 $  $Author: trey $  $Date: 2005-01-27 05:33:40 $
 *  
 * @file    ValueFunction.cc
 * @brief   No brief
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

//#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "commonDefs.h"
#include "MatrixUtils.h"
#include "ValueFunction.h"

using namespace std;
using namespace MatrixUtils;
using namespace MATRIX_NAMESPACE;

/**********************************************************************
 * MEMBER FUNCTIONS
 **********************************************************************/

// return true if the value intervals returned by this value function
// and <rhs> overlap at all of <numSamples> randomly chosen belief points.
bool ValueFunction::consistentWith(const ValueFunction& rhs, int numSamples,
				   bool debug) const
{
  ValueInterval selfint, rhsint;
  dvector bd(numStates);
  belief_vector b(numStates);
  FOR (i, numSamples) {
    rand_vector(bd,numStates);
    cvector_from_dvector(b,bd);
    b *= (1.0/norm_1(b)); // normalize so components add to 1
    selfint = getValueAt(b);
    rhsint = rhs.getValueAt(b);
    if (debug) {
      cout << "b' = " << sparseRep(b) << endl;;
      cout << "lhs = " << selfint << endl;
      cout << "rhs = " << rhsint << endl << endl;
    }
    if (!selfint.overlapsWith(rhsint)) {
      cout << "inconsistent at b' = " << sparseRep(b) << endl;
      cout << "lhs = " << selfint << endl;
      cout << "rhs = " << rhsint << endl << endl;
      return false;
    }
  }
  return true;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/01/26 04:12:06  trey
 * fixed for new rand_vector() API
 *
 * Revision 1.1  2004/11/24 20:48:05  trey
 * moved to common from hsvi
 *
 *
 ***************************************************************************/
