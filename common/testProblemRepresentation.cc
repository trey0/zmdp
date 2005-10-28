/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.3 $  $Author: trey $  $Date: 2005-10-28 03:50:32 $
   
 @file    testProblemRepresentation.cc
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

#include "Pomdp.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

int main(int argc, char** argv) {
  init_matrix_utils();

  Pomdp problem;
  problem.readFromFile( argv[1], /* use_fast_parser = */ false );

  write_to_file(problem.R,"R.dat");

  FOR (i, problem.numStates) {
    if (problem.isTerminalState[i]) {
      cout << "E: " << i << endl;
    }
  }

#if 0
  cout << "numStates = " << problem.numStates << endl;
  cout << "T.size = " << problem.T[4].size1()
       << " " << problem.T[4].size2() << endl;

  timeval startTime, endTime;
  belief_vector result(problem.numStates);
  bmatrix& T = problem.T[4];
  belief_vector& b0 = problem.initialBelief;
  dvector tmp(problem.numStates);

  gettimeofday(&startTime,0);
  for (int i=0; i < 10; i++) {
    //noalias(result) = prod( T, b0 );
    axpy_prod( T, b0, tmp, true );
    result = tmp;
  }
  gettimeofday(&endTime,0);

  cout << "multiplies: elapsed time = "
       << ((endTime.tv_sec - startTime.tv_sec)
	   + 1e-6*(endTime.tv_usec - startTime.tv_usec))
       << endl;

  const char* fname = "result_tp.dat";
  ofstream out(fname);
  if (!out) {
    cerr << "ERROR: couldn't open " << fname << " for writing: "
	 << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  out << problem.numStates << endl;
  FOR (i, problem.numStates) {
    out << result[i] << endl;
  }
  out.close();
#endif

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/10/28 02:51:41  trey
 * added copyright headers
 *
 *
 ***************************************************************************/
