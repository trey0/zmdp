
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
  dvector xd(4), yd(4);
  avector xa, ya, za;

  xd(0) = 1;
  xd(1) = 2;
  xd(2) = 3;
  xd(3) = 4;

  copy( xa, xd );

  cout << "xa1: ";
  xa.write(cout);
  cout << endl;

  //const double c = 1.0 / M_PI;
  const double c = 0.5;
  xd(0) = pow(c,1);
  xd(1) = pow(c,2);
  xd(2) = pow(c,3);
  xd(3) = pow(c,4);
  copy( xa, xd );

  cout << "xa2: ";
  xa.write(cout);
  cout << endl;

  xd(0) = 3;
  xd(1) = 1;
  xd(2) = 3;
  xd(3) = 1;
  copy( xa, xd );

  cout << "xa3: ";
  xa.write(cout);
  cout << endl;

  yd(0) = 2;
  yd(1) = 4;
  yd(2) = 2;
  yd(3) = 4;
  copy( ya, yd );

  add( za, xa, ya );

  cout << "za: ";
  za.write(cout);
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
