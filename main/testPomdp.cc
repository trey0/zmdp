
#include <assert.h>
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "PomdpM.h"
#include "FocusedPomdp.h"
#if USE_EMPOMDP
#  include "EMPomdp.h"
#endif
#include "QMDP.h"
#include "Interleave.h"
#include "stdinInterface.h"

using namespace std;
using namespace MatrixUtils;

void usage(void) {
  cerr <<
    "usage: testPomdp OPTIONS <algorithm> <probname> [minOrder maxOrder]\n"
    "  -h or --help       Print this help\n"
    "  --version          Print version information\n"
    "  -f or --fast       Use fast (but very picky) alternate problem parser\n"
    "  -i or --iterations Set number of simulation iterations (default: 1000)\n"
    "  -n or --no-console Do not poll for user console commands\n"
    "\n"
    "  available algorithms:\n"
    "    hsvi\n"
#if USE_EMPOMDP
    "    empomdp\n"
#endif
    "    qmdp\n"
;
  exit(-1);
}

void testBatchIncremental(string algorithm,
			  string prob_name,
			  int min_order,
			  int max_order,
			  int num_iterations,
			  bool use_fast_parser)
{
  PomdpM problem;

  cout << "CFLAGS = " << CFLAGS << endl;

  //prob_name = "examples/" + prob_name + ".pomdp";
  problem.readFromFile( prob_name, use_fast_parser );

#if 0
  // test benchmark code

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

  exit(0);
#endif

  Solver* solver;
  if (0) {
  }
  else if (algorithm == "hsvi") {
    solver = new FocusedPomdp();
  }
#if USE_EMPOMDP
  else if (algorithm == "empomdp") {
    solver = new EMPomdp();
  }
#endif
  else if (algorithm == "qmdp") {
    solver = new QMDP();
  }
  else {
    cerr << "ERROR: unknown algorithm " << algorithm << endl << endl;
    usage();
  }

  //string prefix = "/tmp/";
  string prefix = "";
  Interleave x;
  x.batchTestIncremental(/* numIterations = */ num_iterations,
			 &problem, *solver,
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

  char *algorithm = NULL;
  char *prob_name = NULL;
  int min_order = -999;
  int max_order = -999;
  bool use_fast_parser = false;
  int num_iterations = 1000;
  bool past_options = false;

  for (int argi=1; argi < argc; argi++) {
    string args = argv[argi];
    if (args[0] == '-' && !past_options) {
      if (args == "-h" || args == "--help") {
	usage();
      } else if (args == "--version") {
	cout << "CFLAGS = " << CFLAGS << endl;
	exit(EXIT_SUCCESS);
      } else if (args == "-f" || args == "--fast") {
	use_fast_parser = true;
      } else if (args == "-i" || args == "--iterations") {
	if (++argi == argc) {
	  cerr << "ERROR: -i flag without argument" << endl;
	}
	num_iterations = atoi(argv[argi]);
      } else if (args == "-n" || args == "--no-console") {
	setPollingEnabled(0);
      } else if (args == "--") {
	past_options = true;
	cout << "got --" << endl;
      } else {
	cerr << "ERROR: unknown option " << args << endl << endl;
	usage();
      }
    } else if (algorithm == NULL) {
      algorithm = argv[argi];
    } else if (prob_name == NULL) {
      prob_name = argv[argi];
    } else if (-999 == min_order) {
      min_order = atoi(argv[argi]);
    } else if (-999 == max_order) {
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
  if (-999 == min_order) {
    min_order = 0; // default
  }
  if (-999 == max_order) {
    max_order = 6; // default
  }

  testBatchIncremental(algorithm, prob_name, min_order, max_order,
		       num_iterations, use_fast_parser);

  // signal we are done
  FILE *fp = fopen("/tmp/testPomdp_done", "w");
  fprintf(fp, "success\n");
  fclose(fp);

  return 0;
}
