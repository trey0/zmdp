/********** tell emacs we use -*- c++ -*- style comments *******************
 * COPYRIGHT 2004, Carnegie Mellon University
 * $Revision: 1.1 $  $Author: trey $  $Date: 2005-10-21 20:11:52 $
 *  
 * PROJECT: Life in the Atacama
 *
 * @file    commonTypes.cc
 * @brief   No brief
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>

#include "pomdpCommonTypes.h"

using namespace std;

namespace pomdp {

ostream& operator<<(ostream& out, const ValueInterval& v) {
  out << "[" << v.l << " .. " << v.u << "]";
  return out;
}

};

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2004/11/13 23:29:44  trey
 * moved many files from hsvi to common
 *
 *
 ***************************************************************************/
