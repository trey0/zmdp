
#include "PomdpM.h"
#include "MatrixUtils.h"

using namespace std;
using namespace MatrixUtils;

int main(int argc, char** argv) {
  init_matrix_utils();

  PomdpM problem;
  //prob_name = "examples/" + prob_name + ".pomdp";
  problem.readFromFile( argv[1], /* use_fast_parser = */ false );

#if 0
  char buf[256];
  FOR (a, problem.numActions) {
    snprintf(buf,sizeof(buf),"O_%02d.dat", a);
    write_to_file(problem.O[a],buf);
  }
#endif

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
