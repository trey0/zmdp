
#include <assert.h>

#include <iostream>

#include "PomdpM.h"
#include "MatrixUtils.h"
#include "FocusedPomdp.h"
#include "Interleave.h"

using namespace std;
using namespace MatrixUtils;

void usage(void) {
  cout << "usage: testPomdp <probname> [minOrder maxOrder]" << endl;
  exit(-1);
}

void testBatchIncremental(string prob_name, int min_order, int max_order) {
  PomdpP p = new PomdpM;
  //prob_name = "examples/" + prob_name + ".pomdp";
  p->readFromFile(prob_name);

  FocusedPomdp solver;

  //string prefix = "/tmp/";
  string prefix = "";
  Interleave x;
  x.batchTestIncremental(/* numIterations = */ 100,
			 p, solver,
			 /* numSteps = */ 251,
			 /* minPrecision = */ 1e-10,
			 /* minOrder = */ min_order,
			 /* maxOrder = */ max_order,
			 /* ticksPerOrder = */ 3,
			 /* outFileName = */ "inc.plot",
			 /* boundsFileName = */ "bounds.plot",
			 /* simFileName = */ "sim.plot");

  x.printRewards();
}

int main(int argc, char **argv) {
  init_matrix_utils();

  char *prob_name = NULL;
  int min_order = -1;
  int max_order = -1;
  for (int argi=1; argi < argc; argi++) {
    string args = argv[argi];
    if (args[0] == '-') {
      if (args == "-h" || args == "--help") {
	usage();
      } else {
	cerr << "ERROR: unknown option " << args << endl << endl;
	usage();
      }
    } else if (prob_name == NULL) {
      prob_name = argv[argi];
    } else if (min_order == -1) {
      min_order = atoi(argv[argi]);
    } else if (max_order == -1) {
      max_order = atoi(argv[argi]);
    } else {
      cerr << "ERROR: too many arguments" << endl << endl;
      usage();
    }
  }

  if (prob_name == NULL) {
    cerr << "ERROR: not enough arguments" << endl << endl;
    usage();
  }
  if (min_order == -1) {
    max_order = 0; // default
  }
  if (max_order == -1) {
    max_order = 6; // default
  }

  testBatchIncremental(prob_name, min_order, max_order);

  // signal we are done
  FILE *fp = fopen("/tmp/testPomdp_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}
