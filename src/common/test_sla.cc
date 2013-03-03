/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.8 $  $Author: trey $  $Date: 2006-04-28 17:57:41 $
   
 @file    test_sla.cc
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

#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "sla.h"
#include "sla_mask.h"

using namespace std;
using namespace sla;

void test_dvector(void) {
  dvector xd(3);
  xd.resize(4);
  xd(0) = 1;
  xd(1) = 2;
  xd(2) = 3;
  xd(3) = 4;
  cout << "--xd: size=4 data=1 2 3 4" << endl;
  cout << "  xd: size=" << xd.size()
       << " data=" << xd(0) << " " << xd(1)
       << " " << xd(2) << " " << xd(3) << endl;
  
  xd *= 2.0;
  cout << "--xd: data=2 4 6 8" << endl;
  cout << "  xd: data=" << xd(0) << " " << xd(1)
       << " " << xd(2) << " " << xd(3) << endl;

  dvector yd(4);
  yd(0) = 5;
  yd(1) = 4;
  yd(2) = 3;
  yd(3) = 2;

  xd += yd;
  cout << "--xd: data=7 8 9 10" << endl;
  cout << "  xd: data=" << xd(0) << " " << xd(1)
       << " " << xd(2) << " " << xd(3) << endl;

  string xs = "2 9 7";
  istringstream iss(xs);
  xd.read(iss);
  cout << "--xd: size=2 data=9 7" << endl;
  cout << "  xd: size=" << xd.size()
       << " data=" << xd(0) << " " << xd(1) << endl;
}

void test_cvector(void) {
  dvector xd(3);
  xd.resize(4);
  xd(0) = 1;
  xd(1) = 2;
  xd(2) = 3;
  xd(3) = 4;

  cvector xc;
  copy( xc, xd );

  cout << "--xc: size=4 data=1 2 3 4" << endl;
  cout << "  xc: size=" << xc.size()
       << " data=" << xc(0) << " " << xc(1)
       << " " << xc(2) << " " << xc(3) << endl;
  
  xc *= 2;
  cout << "--xc: data=2 4 6 8" << endl;
  cout << "  xc: data=" << xc(0) << " " << xc(1)
       << " " << xc(2) << " " << xc(3) << endl;

  // resize() implicitly sets everything to zero
  xc.resize(5);
  // push_back() calls must be in increasing order of index.
  xc.push_back(1,-1);
  xc.push_back(3,17);
  xc.canonicalize();
  cout << "--xc: size=5 data=0 -1 0 17 0" << endl;
  cout << "--xc: size=" << xc.size()
       << " data=" << xc(0) << " " << xc(1)
       << " " << xc(2) << " " << xc(3) << " " << xc(4) << endl;

  string xs = "4 " "2 " "0 7 " "3 9 ";
  istringstream iss(xs);
  xc.read(iss);
  cout << "--xc: size=4 data=7 0 0 9" << endl;
  cout << "  xc: size=" << xc.size()
       << " data=" << xc(0) << " " << xc(1)
       << " " << xc(2) << " " << xc(3) << endl;
}

void test_dmatrix(void) {
  dmatrix Ad;
  Ad.resize(2,2);
  Ad(0,0) = 1;
  Ad(0,1) = 2;
  Ad(1,0) = 3;
  Ad(1,1) = 4;

  cout << "--Ad: size=2 2 data=1 2 3 4" << endl;
  cout << "  Ad: size=" << Ad.size1() << " " << Ad.size2()
       << " data=" << Ad(0,0) << " " << Ad(0,1)
       << " " << Ad(1,0) << " " << Ad(1,1) << endl;
}

void test_kmatrix(void) {
  kmatrix Ak;
  Ak.resize(3,3);

  cout << "--Ak: size=3 3 filled=0" << endl;
  cout << "  Ak: size=" << Ak.size1() << " " << Ak.size2()
       << " filled=" << Ak.filled() << endl;
  
  // resize implicitly sets everything to zero.
  Ak.resize(2,2);
  // push_back calls need not be in order.  among calls with the same
  // (r,c) value the last one will be retained.  kmatrix_set_entry()
  // is an alias for kmatrix::push_back
  Ak.push_back(0,0,6);
  Ak.push_back(1,1,2);
  Ak.push_back(1,0,3);
  Ak.push_back(1,1,9);
  // must call canonicalize before doing anything else with the matrix
  Ak.canonicalize();

  cout << "--Ak: size=2 2 filled=3 data=6 0 3 9" << endl;
  cout << "  Ak: size=" << Ak.size1() << " " << Ak.size2()
       << " filled=" << Ak.filled()
       << " data=" << Ak(0,0) << " " << Ak(0,1)
       << " " << Ak(1,0) << " " << Ak(1,1) << endl;

  string As = "2 2 " "2 " "0 1 5 " "1 0 7 ";
  istringstream iss(As);
  Ak.read(iss);
  Ak.canonicalize();
  cout << "--Ak: size=2 2 filled=2 data=0 5 7 0" << endl;
  cout << "  Ak: size=" << Ak.size1() << " " << Ak.size2()
       << " filled=" << Ak.filled()
       << " data=" << Ak(0,0) << " " << Ak(0,1)
       << " " << Ak(1,0) << " " << Ak(1,1) << endl;
}

void test_cmatrix(void) {
  cmatrix Ac;
  Ac.resize(3,3);

  cout << "--Ac: size=3 3 filled=0" << endl;
  cout << "  Ac: size=" << Ac.size1() << " " << Ac.size2()
       << " filled=" << Ac.filled() << endl;

  // resize implicitly sets everything to zero.
  Ac.resize(2,2);
  // push_back calls must be in column-major order.
  Ac.push_back(0,0,6);
  Ac.push_back(1,0,3);
  Ac.push_back(1,1,9);
  // must call canonicalize before doing anything else with the matrix
  Ac.canonicalize();

  cout << "--Ac: size=2 2 filled=3 data=6 0 3 9" << endl;
  cout << "  Ac: size=" << Ac.size1() << " " << Ac.size2()
       << " filled=" << Ac.filled()
       << " data=" << Ac(0,0) << " " << Ac(0,1)
       << " " << Ac(1,0) << " " << Ac(1,1) << endl;

  string As = "2 2 " "2 " "0 1 5 " "1 0 7 ";
  istringstream iss(As);
  Ac.read(iss);
  cout << "--Ac: size=2 2 filled=2 data=0 5 7 0" << endl;
  cout << "  Ac: size=" << Ac.size1() << " " << Ac.size2()
       << " filled=" << Ac.filled()
       << " data=" << Ac(0,0) << " " << Ac(0,1)
       << " " << Ac(1,0) << " " << Ac(1,1) << endl;
}

void test_conversions(void)
{
  dvector xd, xd2;
  cvector xc;
  kmatrix Ak;
  cmatrix Ac;

  xd.resize(3);
  xd(0) = 2;
  xd(1) = 0;
  xd(2) = 6;

  copy( xc, xd );

  cout << "--xc: size=3 filled=2 data=2 0 6" << endl;
  cout << "  xc: size=" << xc.size()
       << " filled=" << xc.filled()
       << " data=" << xc(0) << " " << xc(1) << " " << xc(2) << endl;

  copy( xd2, xc );
  
  cout << "--xd2: size=3 data=2 0 6" << endl;
  cout << "  xd2: size=" << xd2.size()
       << " data=" << xd2(0) << " " << xd2(1) << " " << xd2(2) << endl;


  string As = "2 2 " "2 " "0 1 5 " "1 0 7 ";
  istringstream iss(As);
  Ak.read(iss);
  Ak.canonicalize();
  
  copy( Ac, Ak );

  cout << "--Ac: size=2 2 filled=2 data=0 5 7 0" << endl;
  cout << "  Ac: size=" << Ac.size1() << " " << Ac.size2()
       << " filled=" << Ac.filled()
       << " data=" << Ac(0,0) << " " << Ac(0,1)
       << " " << Ac(1,0) << " " << Ac(1,1) << endl;

  copy_from_column( xc, Ac, 0 );

  cout << "--xc: size=2 filled=1 data=0 7" << endl;
  cout << "  xc: size=" << xc.size()
       << " filled=" << xc.filled()
       << " data=" << xc(0) << " " << xc(1) << endl;

  copy_from_column( xc, Ac, 1 );

  cout << "--xc: size=2 filled=1 data=5 0" << endl;
  cout << "  xc: size=" << xc.size()
       << " filled=" << xc.filled()
       << " data=" << xc(0) << " " << xc(1) << endl;

  // resize implicitly sets everything to zero.
  Ak.resize(2,2);
  // kmatrix_set_entry() calls need not be in order.
  kmatrix_set_entry( Ak, 0, 0, 6 );
  kmatrix_set_entry( Ak, 1, 1, 2 );
  kmatrix_set_entry( Ak, 1, 0, 3 );
  kmatrix_set_entry( Ak, 1, 1, 9 );
  // must call canonicalize before doing anything else with the matrix
  Ak.canonicalize();

  cout << "--Ak: size=2 2 filled=3 data=6 0 3 9" << endl;
  cout << "  Ak: size=" << Ak.size1() << " " << Ak.size2()
       << " filled=" << Ak.filled()
       << " data=" << Ak(0,0) << " " << Ak(0,1)
       << " " << Ak(1,0) << " " << Ak(1,1) << endl;

  kmatrix_transpose_in_place( Ak );
  cout << "--Ak: size=2 2 filled=3 data=6 3 0 9" << endl;
  cout << "  Ak: size=" << Ak.size1() << " " << Ak.size2()
       << " filled=" << Ak.filled()
       << " data=" << Ak(0,0) << " " << Ak(0,1)
       << " " << Ak(1,0) << " " << Ak(1,1) << endl;
}

void test_unary(void)
{
  cvector xc;
  double s;
  dvector xd;

  istringstream iss("5 " "3 " "1 2 " "3 5 " "4 -3 ");
  xc.read(iss);
  s = norm_1(xc);

  cout << "--s=10" << endl;
  cout << "  s=" << s << endl;

  istringstream iss2("5 " "2 " "3 6 " "4 -1 ");
  xc.read(iss2);
  s = norm_1(xc);

  cout << "--s=7" << endl;
  cout << "  s=" << s << endl;

  istringstream iss3("5 " "0 2 0 5 -3");
  xd.read(iss3);
  s = norm_inf(xd);

  cout << "--s=5" << endl;
  cout << "  s=" << s << endl;

  // reusing iss again causes a weird error...
  // but valgrind is happy, so i won't look into it further
  istringstream iss4("5 " "0 2 0 5 -7");
  xd.read(iss4);
  s = norm_inf(xd);

  cout << "--s=7" << endl;
  cout << "  s=" << s << endl;
}

void test_binary(void)
{
  cvector xc, yc, zc;
  dvector xd, yd;
  cmatrix Ac;
  double s;

  istringstream iss("2 2 " "3 " "0 0 2 " "0 1 5 " "1 0 3 ");
  Ac.read(iss);

  istringstream iss2("2 " "2 " "0 7 " "1 11 ");
  xc.read(iss2);

  istringstream iss3("2 " "7 11 ");
  xd.read(iss3);

  mult( yd, Ac, xc );

  cout << "--yd: 69 21" << endl;
  cout << "  yd: " << yd(0) << " " << yd(1) << endl;

  mult( yc, Ac, xc );

  cout << "--yc: 69 21" << endl;
  cout << "  yc: " << yc(0) << " " << yc(1) << endl;

  mult( yd, xd, Ac );

  cout << "--yd: 47 35" << endl;
  cout << "  yd: " << yd(0) << " " << yd(1) << endl;

  emult( zc, xc, yc );

  cout << "--zc: 483 231" << endl;
  cout << "  zc: " << zc(0) << " " << zc(1) << endl;

  istringstream iss4("7 " "4 " "1 2 " "2 3 " "4 5 " "5 7 ");
  xc.read(iss4);

  istringstream iss5("7 " "4 " "2 11 " "3 13 " "5 17 " "6 19 ");
  yc.read(iss5);

  emult( zc, xc, yc );
  
  cout << "--zc: 0 0 33 0 0 119 0" << endl;
  cout << "  zc: " << zc(0) << " " << zc(1)
       << " " << zc(2) << " " << zc(3)
       << " " << zc(4) << " " << zc(5)
       << " " << zc(6) << endl;

  emult( zc, yc, xc );

  cout << "--zc: 0 0 33 0 0 119 0" << endl;
  cout << "  zc: " << zc(0) << " " << zc(1)
       << " " << zc(2) << " " << zc(3)
       << " " << zc(4) << " " << zc(5)
       << " " << zc(6) << endl;

  add( zc, xc, yc );

  cout << "--zc: 0 2 14 13 5 24 19" << endl;
  cout << "  zc: " << zc(0) << " " << zc(1)
       << " " << zc(2) << " " << zc(3)
       << " " << zc(4) << " " << zc(5)
       << " " << zc(6) << endl;

  add( zc, yc, xc );

  cout << "--zc: 0 2 14 13 5 24 19" << endl;
  cout << "  zc: " << zc(0) << " " << zc(1)
       << " " << zc(2) << " " << zc(3)
       << " " << zc(4) << " " << zc(5)
       << " " << zc(6) << endl;

  copy( xc, xd );

  s = inner_prod( xd, xc );
  
  cout << "--s: 170" << endl;
  cout << "  s: " << s << endl;

  istringstream iss6("7 " "4 " "1 2 " "2 3 " "4 5 " "5 7 ");
  xc.read(iss6);

  istringstream iss7("7 " "3 " "1 1 " "4 4 " "5 6 ");
  yc.read(iss7);

  bool xdy = dominates( xc, yc, 0 );
  cout << "--xdy: 1" << endl;
  cout << "  xdy: " << xdy << endl;

  bool ydx = dominates( yc, xc, 0 );
  cout << "--ydx: 0" << endl;
  cout << "  ydx: " << ydx << endl;
}

void test_mask(void)
{
  dvector zd;
  cvector xc, yc, zc;
  cvector zm;

  istringstream iss1("7 " "4 " "1 2 " "2 3 " "4 5 " "5 7 ");
  xc.read(iss1);

  istringstream iss2("7 " "3 " "1 1 " "4 4 " "5 6 ");
  yc.read(iss2);

  bool xsy = mask_subset( xc, yc );
  cout << "--xsy: 0" << endl;
  cout << "  xsy: " << xsy << endl;
  
  bool ysx = mask_subset( yc, xc );
  cout << "--ysx: 1" << endl;
  cout << "  ysx: " << ysx << endl;
  
  bool xmdy = mask_dominates( xc, yc, 0, xc, yc );
  cout << "--xmdy: 1" << endl;
  cout << "  xmdy: " << xmdy << endl;

  bool ymdx = mask_dominates( yc, xc, 0, yc, xc );
  cout << "--ymdx: 0" << endl;
  cout << "  ymdx: " << ymdx << endl;

  istringstream iss3("7 " "1 1 1 1 1 1 1");
  zd.read(iss3);
  copy( zc, zd );

  mask_copy( zm, zd, xc );
  cout << "--zm: 0 1 1 0 1 1 0" << endl;
  cout << "  zm: " << zm(0) << " " << zm(1)
       << " " << zm(2) << " " << zm(3)
       << " " << zm(4) << " " << zm(5)
       << " " << zm(6) << endl;

  mask_copy( zm, zc, xc );
  cout << "--zm: 0 1 1 0 1 1 0" << endl;
  cout << "  zm: " << zm(0) << " " << zm(1)
       << " " << zm(2) << " " << zm(3)
       << " " << zm(4) << " " << zm(5)
       << " " << zm(6) << endl;
}

void test_performance(void)
{
  cmatrix T;
  dvector b0_in;
  cvector b0, result;
  timeval start_time, end_time;

  cout << "reading files" << endl;

  gettimeofday(&start_time,0);
  read_from_file(T, "T4.dat");
  read_from_file(b0_in, "b0.dat");
  copy( b0, b0_in );
  gettimeofday(&end_time,0);

  cout << "reading files: elapsed time = "
       << ((end_time.tv_sec - start_time.tv_sec)
	   + 1e-6*(end_time.tv_usec - start_time.tv_usec))
       << endl;

  gettimeofday(&start_time,0);
  
  FOR (i, 10) {
    mult( result, T, b0 );
  }

  gettimeofday(&end_time,0);

  cout << "multiplies: elapsed time = "
       << ((end_time.tv_sec - start_time.tv_sec)
	   + 1e-6*(end_time.tv_usec - start_time.tv_usec))
       << endl;

  dvector result_out;
  copy( result_out, result );
  write_to_file(result_out,"result.dat");
}

void usage(void) {
  cerr <<
    "usage: test_sla\n"
    "  -h or --help   Print this help\n";
  exit(-1);
}

int main(int argc, char** argv) {
  for (int argi=1; argi < argc; argi++) {
    string args = argv[argi];
    if (args[0] == '-') {
      if (args == "-h" || args == "--help") {
	usage();
      } else {
	cerr << "ERROR: unknown option " << args << endl << endl;
	usage();
      }
    } else {
      cerr << "ERROR: too many arguments" << endl << endl;
      usage();
    }
  }

  test_dvector();
  test_cvector();
  test_dmatrix();
  test_kmatrix();
  test_cmatrix();
  test_conversions();
  test_unary();
  test_binary();
  test_mask();

  test_performance();

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.7  2005/10/28 03:50:32  trey
 * simplified license
 *
 * Revision 1.6  2005/10/28 02:51:41  trey
 * added copyright headers
 *
 *
 ***************************************************************************/
