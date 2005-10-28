/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.2 $  $Author: trey $  $Date: 2005-10-28 02:51:41 $
   
 @file    testProblemRepresentation.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

 * This code and derivatives may not be incorporated into commercial
   products without specific prior written permission.
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * The name of Trey Smith may not be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
 *
 ***************************************************************************/
