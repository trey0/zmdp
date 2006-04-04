/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2006-04-04 17:20:42 $
   
 @file    AbstractBound.h
 @brief   No brief

 Copyright (c) 2005, Trey Smith. All rights reserved.

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

#ifndef INCAbstractBound_h
#define INCAbstractBound_h

#include <iostream>
#include <string>
#include <vector>

#include "MDP.h"

using namespace sla;

namespace zmdp {

struct AbstractBound {
  virtual ~AbstractBound(void) {}

  // performs any computation necessary to initialize the bound
  virtual void initialize(double targetPrecision) = 0;

  // returns the bound value at state s
  virtual double getValue(const state_vector& s) const = 0;
};

}; // namespace zmdp

#endif // INCAbstractBound_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/02/14 19:30:13  trey
 * added targetPrecision argument to initialize()
 *
 * Revision 1.3  2006/02/01 01:09:37  trey
 * renamed pomdp namespace -> zmdp
 *
 * Revision 1.2  2006/01/31 20:08:24  trey
 * changed args to initialize()
 *
 * Revision 1.1  2006/01/31 19:13:18  trey
 * initial check-in
 *
 *
 ***************************************************************************/

