
#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "aadd.h"

using namespace std;
using namespace sla;
using namespace aadd;

void doit(void) {
  dvector x(4);
  x(0) = 1;
  x(1) = 2;
  x(2) = 3;
  x(3) = 4;

  avector y;
  copy( y, x );

  cout << "y1: ";
  y.write(cout);
  cout << endl;

  //const double c = 1.0 / M_PI;
  const double c = 0.5;
  x(0) = pow(c,1);
  x(1) = pow(c,2);
  x(2) = pow(c,3);
  x(3) = pow(c,4);
  copy( y, x );

  cout << "y2: ";
  y.write(cout);
  cout << endl;

  x(0) = 1;
  x(1) = 0;
  x(2) = 1;
  x(3) = 0;
  copy( y, x );

  cout << "y3: ";
  y.write(cout);
  cout << endl;
}

void usage(void) {
  cerr <<
    "usage: test_aadd\n"
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

  doit();

  return 0;
}
