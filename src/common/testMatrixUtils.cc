/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.5 $  $Author: trey $  $Date: 2007-01-14 01:24:57 $
   
 @file    testMatrixUtils.cc
 @brief   No brief

 Copyright (c) 2002-2005, Trey Smith. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License"); you may
 not use this file except in compliance with the License.  You may
 obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ***************************************************************************/

#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

void set_to_count(bmatrix& m) {
  m(0,0) = -3;
  m(0,1) = -2;
  m(0,2) = -1;

  m(1,0) = 0;
  m(1,1) = 1;
  m(1,2) = 2;

  m(2,0) = 3;
  m(2,1) = 4;
  m(2,2) = 5;
}

double square(double x) {
  return x*x;
}

double add(double x, double y) {
  return x+y;
}

int main(void) {
  init_matrix_utils();

  bmatrix m = rand_matrix(3,3);
  cout << "rand_matrix(3,3) = " << m << endl;

  set_to_zero(m);
  cout << "(set_to_zero) m = " << m << endl;
  
  set_to_count(m);
  cout << "trans(m) = " << trans(m) << endl;

  double q_init[] = { 1, 0, 2 };
  bvector q = make_vector(q_init, 3);
  cout << "q = " << q << endl;

  cvector w(4);
  w(0) = 3;
  w(1) = 4;
  cout << "w = " << w << ", argmax_elt(w) = " << argmax_elt(w) << endl;
  w(0) = -2;
  w(1) = -1;
  cout << "w = " << w << ", argmax_elt(w) = " << argmax_elt(w) << endl;

  cout << "m = " << m << endl;
  cout << "scale_columns(q,m) = " << scale_columns(q,m) << endl;

#if 0
  cout << "m(range(1,3),1) = " << sub_vector(m,range(1,3),1) << endl;
  cout << "m(1,range(1,3)) = " << sub_vector(m,1,range(1,3)) << endl;
  cout << "max_elt(m) = " << max_elt(m) << endl;
  cout << "min_elt(m) = " << min_elt(m) << endl;
  cout << "eabs(m) = " << eabs(m) << endl;
#endif

  cout << "diag(m) = "
       << matrix_vector_range<bmatrix>(m, range(0,3), range(0,3))
       << endl;

  bvector v = range_vector(6);
  cout << "range_vector(6) = " << v << endl;

  bvector b = v / norm_1(v);
  cout << "normalized v = " << b << endl;

  cout << "chooseVector samples = ";
  FOR (i, 10) {
    cout << chooseFromDistribution(b) << " ";
  }
  cout << endl;

  cout << "mapfun(square, m) = " << mapfun(square, m) << endl;
  cout << "mapfun2(add, v, v) = "
       << mapfun2(add, v, v) << endl;

  cout << "mapfunrange(square, 6) = " << mapfunrange(square, 6) << endl;
  cout << "emax(v,2.0*v) = " << emax(v, 2.0*v) << endl;
  cout << "eprod(v,v) = " << eprod(v,v) << endl;

  std::vector<double> stats;
  stats.push_back(3);
  stats.push_back(3);
  stats.push_back(3);
  stats.push_back(5);
  double avg, stdev;
  calc_avg_stdev_collection(stats.begin(), stats.end(), avg, stdev);
  cout << "avg = " << avg << ", stdev = " << stdev << endl;

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2006/04/28 17:57:41  trey
 * changed to use apache license
 *
 * Revision 1.3  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.2  2005/10/28 02:51:41  trey
 * added copyright headers
 *
 *
 ***************************************************************************/
