
#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "sla.h"

using namespace std;
using namespace sla;

void usage(void) {
  cerr <<
    "usage: test_sla\n"
    "  -h or --help   Print this help\n";
  exit(-1);
}

void test_correctness(void)
{
  cmatrix A;
  cvector x, result;
  dvector x_in, result_out;

  read_from_file(A,"A.dat");
  A.write(cout);
  read_from_file(x_in,"x.dat");
  cvector_from_dvector(x, x_in);
  result.resize(x.size());
  mult( result, A, x );
  
  dvector_from_cvector(result_out, result);
  result_out.write(cout);
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
  cvector_from_dvector( b0, b0_in );
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
  dvector_from_cvector( result_out, result );
  write_to_file(result_out,"result.dat");
}

int main(int argc, char **argv) {

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

  //test_correctness();
  test_performance();

  return 0;
}
