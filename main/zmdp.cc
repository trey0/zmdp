/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.1 $  $Author: trey $  $Date: 2007-07-08 20:59:30 $

 @file    zmdp.cc
 @brief   No brief

 Copyright (c) 2002-2007, Trey Smith.  All rights reserved.

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
#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <fstream>

#include "MatrixUtils.h"
#include "MDPSim.h"
#include "BoundPairExec.h"
#include "LSPathAndReactExec.h"
#include "solverUtils.h"
#include "zmdpMainConfig.h"
#include "PolicyEvaluator.h"
#include "zmdpCommonTime.h"
#include "TestDriver.h"

#include "zmdpMainConfig.cc" // embed default config file

using namespace std;
using namespace MatrixUtils;
using namespace zmdp;

enum CommandsEnum {
  CMD_SOLVE,
  CMD_BENCHMARK,
  CMD_EVALUATE
};

bool userTerminatedG = false;

void sigIntHandler(int sig) {
  userTerminatedG = true;

  printf("*** received SIGINT, user pressed control-C ***\n"
	 "terminating run and writing output policy as soon as the solver returns control\n");
  fflush(stdout);
}

void setSignalHandler(int sig, void (*handler)(int)) {
  struct sigaction act;
  memset (&act, 0, sizeof(act));
  act.sa_handler = handler;
  act.sa_flags = SA_RESTART;
  if (-1 == sigaction (sig, &act, NULL)) {
    cerr << "ERROR: unable to set handler for signal "
         << sig << endl;
    exit(EXIT_FAILURE);
  }
}

void doSolve(const ZMDPConfig& config)
{
  init_matrix_utils();
  StopWatch run;

  SolverParams p;
  p.setValues(config);

  printf("%05d reading model file and allocating data structures\n",
	 (int) run.elapsedTime());
  SolverObjects so;
  constructSolverObjects(so, p, config);

  // initialize the solver
  printf("%05d calculating initial heuristics\n",
	 (int) run.elapsedTime());
  so.solver->planInit(so.sim->getModel(), &config);
  printf("%05d finished initialization, beginning to improve policy\n",
	 (int) run.elapsedTime());
  
  setSignalHandler(SIGINT, &sigIntHandler);

  double lastPrintTime = -1000;
  bool reachedTargetPrecision = false;
  bool reachedTimeout = false;
  int numSolverCalls = 0;
  while (!(reachedTargetPrecision || reachedTimeout || userTerminatedG)) {
    // make a call to the solver
    reachedTargetPrecision =
      so.solver->planFixedTime(so.sim->getModel()->getInitialState(),
			       /* maxTime = */ -1, p.terminateRegretBound);
    numSolverCalls++;

    // check timeout
    double elapsed = run.elapsedTime();
    if (elapsed >= p.terminateWallclockSeconds) {
      reachedTimeout = true;
    }

    // print a progress update every 10 seconds
    if ((elapsed - lastPrintTime > 10)
	|| reachedTargetPrecision || reachedTimeout || userTerminatedG) {
      ValueInterval intv = so.solver->getValueAt(so.sim->getModel()->getInitialState());
      printf("%05d %6d calls to solver, bounds [%8.4f .. %8.4f], regret <= %g\n",
	     (int) elapsed, numSolverCalls, intv.l, intv.u, (intv.u - intv.l));
      lastPrintTime = elapsed;
    }
  }

  // say why the run ended
  if (reachedTargetPrecision) {
    printf("%05d terminating run; reached target regret bound of %g\n",
	   (int) run.elapsedTime(), p.terminateRegretBound);
  } else if (reachedTimeout) {
    printf("%05d terminating run; passed specified timeout of %g seconds\n",
	   (int) run.elapsedTime(), p.terminateWallclockSeconds);
  } else {
    printf("%05d terminating run; caught SIGINT from user\n",
	   (int) run.elapsedTime());
  }

  // write out a policy
  if (NULL == p.policyOutputFile) {
    printf("%05d (not outputting policy)\n", (int) run.elapsedTime());
  } else {
    printf("%05d writing policy to '%s'\n", (int) run.elapsedTime(), p.policyOutputFile);
    assert(so.bounds->lowerBound != NULL);
    so.bounds->writePolicy(p.policyOutputFile, /* canModifyBounds = */ true);
  }

  // finish up logging (if any, according to params specified in the config file)
  printf("%05d finishing logging (e.g., writing qValuesOutputFile if it was requested)\n",
	 (int) run.elapsedTime());
  so.solver->finishLogging();

  printf("%05d done\n", (int) run.elapsedTime());
}

void doBenchmark(const ZMDPConfig& config)
{
  init_matrix_utils();

  SolverParams p;
  p.setValues(config);

  printf("reading input files\n");
  SolverObjects so;
  constructSolverObjects(so, p, config);

  TestDriver x;
  x.batchTestIncremental(config,
			 /* numIterations = */ p.evaluationTrialsPerEpoch,
			 so,
			 /* numSteps = */ p.evaluationMaxStepsPerTrial,
			 /* targetPrecision = */ p.terminateRegretBound,
			 /* minOrder = */ p.evaluationFirstEpochWallclockSeconds,
			 /* maxOrder = */ p.terminateWallclockSeconds,
			 /* ticksPerOrder = */ p.evaluationEpochsPerMagnitude,
			 /* incPlotFileName = */ p.evaluationOutputFile,
			 /* boundsFileName = */ p.boundsOutputFile,
			 /* simFileName = */ p.simulationTraceOutputFile,
			 /* policyOutputFile = */ p.policyOutputFile);
}

void doEvaluate(const ZMDPConfig& config)
{
  // seeds random number generator
  init_matrix_utils();

  SolverParams p;
  p.setValues(config);

  const char* simModelFileName = config.getString("simulatorModel").c_str();
  const char* plannerModelFileName = config.getString("plannerModel").c_str();
  if (0 == strcmp(plannerModelFileName, "-")) {
    plannerModelFileName = simModelFileName;
  }
  const char* policyFileName = config.getString("policyInputFile").c_str();
  const char* customModelFileName = config.getString("customModel").c_str();

  // initialize exec
  MDPExecCore* exec = NULL;
  MDPExec* mdpExec = NULL;
  std::string policyType = config.getString("policyType");
  if (policyType == "maxPlanes" || policyType == "cassandraAlpha") {
    BoundPairExec* bpExec = new BoundPairExec();
    bpExec->initReadFiles(plannerModelFileName, policyFileName, config);
    exec = mdpExec = bpExec;
  } else if (policyType == "lspath" || policyType == "lsblind") {
    if (policyType == "lspath" && 0 == strcmp(customModelFileName, "none")) {
      fprintf(stderr, "ERROR: lspath policy type requires --customModel argument (-h for help)\n");
      exit(EXIT_FAILURE);
    }
    LSPathAndReactExec* lpExec = new LSPathAndReactExec();
    lpExec->init(customModelFileName, &config);
    exec = lpExec;
  } else {
    fprintf(stderr, "ERROR: unknown policy type '%s' (-h for help)\n",
	    policyType.c_str());
    exit(EXIT_FAILURE);
  }

  // initialize simulator
  Pomdp* simPomdp;
  bool assumeIdenticalModels = false;
  if (mdpExec != NULL && plannerModelFileName == simModelFileName) {
    simPomdp = (Pomdp*) mdpExec->mdp;
    assumeIdenticalModels = true;
  } else {
    simPomdp = new Pomdp(simModelFileName, &config);

    if (mdpExec != NULL) {
      Pomdp* plannerPomdp = (Pomdp*) mdpExec->mdp;
      
      if (! ((plannerPomdp->getNumActions() == simPomdp->getNumActions())
	     && (plannerPomdp->getNumObservations() == simPomdp->getNumObservations()))) {
	printf("ERROR: planner model %s and evaluation model %s must have the same number of actions and observations\n",
	       plannerModelFileName, simModelFileName);
	exit(EXIT_FAILURE);
      }
    }
  }
  if (zmdpDebugLevelG >= 1) {
    printf("If planning and sim models are identical, evaluator can optimize:\n"
	   "  assumeIdenticalModels=%d\n",
	   assumeIdenticalModels);
  }

  // simulate running the policy many times and collect the per-run total reward values
  PolicyEvaluator eval(simPomdp, exec, &config, assumeIdenticalModels);
  dvector rewardSamples;
  double successRate;
  eval.getRewardSamples(rewardSamples, successRate, /* verbose = */ true);

  // output summary statistics, mean and 95% confidence interval for the mean
  double mean, quantile1, quantile2;
  calc_bootstrap_mean_quantile(rewardSamples,
			       0.05, // 95% confidence interval
			       mean, quantile1, quantile2);
  printf("REWARD_MEAN_CONF95MIN_CONF95MAX %.3lf %.3lf %.3lf\n", mean, quantile1, quantile2);
}

void solveUsage(const char* cmd0)
{
  cerr <<
    "usage: " << cmd0 << " solve [options] <model>\n"
    "  Run 'zmdp -h' for an overview of commands and generic options.\n"
    "\n"
    "  'zmdp solve' generates an output policy for a search strategy and\n"
    "  problem you select.  It runs the search strategy in an anytime\n"
    "  fashion, periodically printing bounds on the value of the initial\n"
    "  state to console so that you can monitor progress.  When the run ends,\n"
    "  the final policy is output to the file you specify.  There are several\n"
    "  options for how to end the run: you can specify a desired regret bound\n"
    "  for the output solution, specify a fixed timeout, or just use ctrl-C to\n"
    "  interrupt the algorithm when you are satisfied (it will output the final\n"
    "  policy before exiting).\n"
    "\n"
    "Commonly used options:\n"
    "  -f        Use fast model parser (for larger RockSample and LifeSurvey problems)\n"
    "  -o <file> Specify where to write output policy [out.policy]\n"
    "  -p <#>    Terminate when policy regret is bounded to this  precision [1e-3]\n"
    "  -s <alg>  Specify search strategy, like 'frtdp', 'hsvi', or others [frtdp]\n"
    "  -t <#>    Terminate after this number of seconds wallclock time [none]\n"
    "  For many more options and more detailed descriptions, see the config file.\n"
    "\n"
    "Examples:\n"
    "  " << cmd0 << " solve RockSample_4_4.pomdp\n"
    "  " << cmd0 << " solve large-b.racetrack\n"
    "  " << cmd0 << " solve -t 60 -o my.policy RockSample_4_4.pomdp\n"
    "  " << cmd0 << " solve -s lrtdp RockSample_4_4.pomdp\n"
    "  " << cmd0 << " solve -f RockSample_5_7.pomdp\n"
    "\n"
    ;
  exit(-1);
}

void benchmarkUsage(const char* cmd0)
{
  cerr <<
    "usage: " << cmd0 << " benchmark [options] <model>\n"
    "  Run 'zmdp -h' for an overview of commands and generic options.\n"
    "\n"
    "  'zmdp benchmark' tests performance for a search strategy and problem\n"
    "  you select.  As the solution algorithm progresses, various statistics\n"
    "  are logged, such as the algorithm's bounds on the value of the initial\n"
    "  state, the number of backups so far, and periodic measurements of the\n"
    "  solution quality in simulation.  The plotting tools in the src/tools\n"
    "  directory can be used to plot performance based on the logs.\n"
    "\n"
    "Commonly used options:\n"
    "  -f        Use fast model parser (for larger RockSample and LifeSurvey problems)\n"
    "  -i <#>    Specify number of simulation runs at each evaluation epoch [100]\n"
    "  -o <file> Specify where to write output policy [none]\n"
    "  -p <#>    Terminate when policy regret is bounded to this precision [1e-3]\n"
    "  -s <alg>  Specify search strategy, like 'frtdp', 'hsvi', or others [frtdp]\n"
    "  -t <#>    Terminate after this number of seconds wallclock time [none]\n"
    "  For many more options and more detailed descriptions, see the config file.\n"
    "\n"
    "Examples:\n"
    "  " << cmd0 << " bench RockSample_4_4.pomdp\n"
    "  " << cmd0 << " bench large-b.racetrack\n"
    "  " << cmd0 << " bench -s lrtdp RockSample_4_4.pomdp\n"
    "  " << cmd0 << " bench -f RockSample_5_7.pomdp\n"
    "\n"
    ;
  exit(-1);
}

void evaluateUsage(const char* cmd0)
{
  cerr <<
    "usage: " << cmd0 << " evaluate [options] <simulatorModel>\n"
    "  Run 'zmdp -h' for an overview of commands and generic options.\n"
    "\n"
    "  'zmdp evaluate' evaluates a policy output by 'zmdp solve' or 'zmdp benchmark'.\n"
    "  The policy is evaluated by executing a number of trials in simulation;\n"
    "  the output is mean reward and the 95% confidence interval for the\n"
    "  estimate of the mean, calculated using the bootstrap method.\n"
    "\n"
    "Commonly used options:\n"
    "  -f        Use fast model parser (for larger RockSample and LifeSurvey problems)\n"
    "  -i <#>    Specify number of simulation runs at each evaluation epoch [100]\n"
    "  For many more options and more detailed descriptions, see the config file.\n"
    "\n"
    "Examples:\n"
    "  " << cmd0 << " eval -f --policyInputFile my.policy ltv1.pomdp\n"
    "  " << cmd0 << " eval -f --policyType lspath --customModel ltv1.lifeSurvey -i 1000 ltv1.pomdp\n"
    "  " << cmd0 << " eval -f --plannerModel ltv1.pomdp --simulatorModel ltv1a.pomdp\n"
    "\n"
    ;
  exit(-1);
}

void genericUsage(const char* cmd0)
{
  cerr <<
    "usage: " << cmd0 << " <command> [options]\n"
    "\n"
    "Commands:\n"
    "  zmdp solve      Solves an MDP or POMDP, generating an output policy\n"
    "  zmdp benchmark  Like 'solve', but interleaves evaluation during the solution process\n"
    "  zmdp evaluate   Evaluates a policy output by 'solve' or 'benchmark'\n"
    "\n"
    "  For more information on a command, run (for example), 'zmdp solve -h'.\n"
    "\n"
    "Generic options:\n"
    "  -h or --help           Print this help\n"
    "  --version              Print version information (CFLAGS used at compilation)\n"
    "  -c or --config <file>  Specify a config file to read options from\n"
    "  --genConfig <file>     Generate an example config file and exit\n"
    "\n"
    "  ZMDP gets configuration information from three places: (1) Default values\n"
    "  are embedded in the binary at compile time.  (2) If you specify a config file\n"
    "  using the --config option, any fields set in that file override the defaults.\n"
    "  (3) You can override individual fields in the config file from the command line.\n"
    "  For instance, if the config file specifies 'searchStrategy frtdp' you can override\n"
    "  with '--searchStrategy hsvi' at the command line.\n"
    "\n"
    "  To generate an example config file (including default values and comments describing\n"
    "  all the parameters), use the '--genConfig <file>' option.\n"
    "\n"
    "Examples:\n"
    "  " << cmd0 << " solve RockSample_4_4.pomdp\n"
    "  " << cmd0 << " benchmark RockSample_4_4.pomdp\n"
    "  " << cmd0 << " evaluate --policyInputFile out.policy RockSample_4_4.pomdp\n"
    "\n"
    ;
  exit(-1);
}

void usage(const char* cmd0, const std::string& cmd1)
{
  if (cmd1 == "solve") {
    solveUsage(cmd0);
  } else if (cmd1 == "benchmark") {
    benchmarkUsage(cmd0);
  } else if (cmd1 == "evaluate") {
    evaluateUsage(cmd0);
  } else {
    genericUsage(cmd0);
  }
}

int main(int argc, char **argv) {
  SolverParams p;

  // save arguments for debug printout later
  ostringstream outs;
  for (int i=1; i < argc; i++) {
    outs << argv[i] << " ";
  }

  ostringstream configArgs;

  string cmd1 = "";
  const char* configFileName = NULL;

  for (int argi=1; argi < argc; argi++) {
    std::string args = argv[argi];

    // this stuff lets us handle (for instance) 'zmdp solve -h' correctly
    if (args == "eval") {
      args = "evaluate";
    }
    if (args == "bench") {
      args = "benchmark";
    }
    if (args == "solve" || args == "benchmark" || args == "evaluate") {
      cmd1 = args;
    }

    // first pass through arguments: only process --help, --config, and --genConfig.
    // all other arguments get passed through for possible alias expansion
    if (args == "-h" || args == "--help") {
      usage(argv[0], cmd1);
    } else if (args == "--version") {
      cout << "CFLAGS = " << CFLAGS << endl;
      exit(EXIT_SUCCESS);
    } else if (args == "-c" || args == "--config") {
      if (++argi == argc) {
	fprintf(stderr, "ERROR: found -c option without argument (use -h for help)\n");
	exit(EXIT_FAILURE);
      }
      configFileName = argv[argi];
    } else if (args == "--genConfig") {
      if (++argi == argc) {
	fprintf(stderr, "ERROR: found --genConfig option without argument (use -h for help)\n");
	exit(EXIT_FAILURE);
      }
      embedWriteToFile(argv[argi], defaultConfig);
      printf("wrote config file with default settings to %s\n", argv[argi]);
      exit(EXIT_SUCCESS);
    } else {
      // append option to configArgs
      configArgs << args << " ";
    }
  }
  
  // config step 1: read defaults embedded in binary
  ZMDPConfig config;
  config.setString("binaryName", argv[0]);
  config.readFromString("<defaultConfig>", defaultConfig.data);
  config.setNoNewFieldsAllowed(true);

  // config step 2: overwrite defaults with values specified in config file
  if (NULL != configFileName) {
    config.readFromFile(configFileName);
  }

  // config step 3: overwrite with values specified on command line
  vector<string> nonOptionArgs = config.processArgs(configArgs.str());

  // check not too many non-option (unprocessed) arguments
  if (nonOptionArgs.size() > 2) {
    fprintf(stderr, "ERROR: expected at most two arguments without flags (use -h for help)\n");
    exit(EXIT_FAILURE);
  }
  
  // process command
  if (nonOptionArgs.size() >= 1) {
    config.setString("command", nonOptionArgs[0]);
  }
  string cmdStr = config.getString("command");
  if (cmdStr == "none") {
    fprintf(stderr, "ERROR: no command specified (use -h for help)\n");
    exit(EXIT_FAILURE);
  }
  int cmd;
  if (cmdStr ==  "solve") {
    cmd = CMD_SOLVE;
  } else if (cmdStr == "benchmark") {
    cmd = CMD_BENCHMARK;
  } else if (cmdStr == "evaluate") {
    cmd = CMD_EVALUATE;
  } else {
    fprintf(stderr, "ERROR: unknown command '%s' (use -h for help)\n", cmdStr.c_str());
    exit(EXIT_FAILURE);
  }

  // process simulatorModel
  if (nonOptionArgs.size() >= 2) {
    config.setString("simulatorModel", nonOptionArgs[1]);
  }
  string simulatorModel = config.getString("simulatorModel").c_str();
  if (simulatorModel == "none") {
    fprintf(stderr, "ERROR: no simulator model specified (use -h for help)\n");
    exit(EXIT_FAILURE);
  }

  // fill in default value of policyOutputFile, depends on command
  if (config.getString("policyOutputFile") == "-") {
    switch (cmd) {
    case CMD_SOLVE:
      config.setString("policyOutputFile", "out.policy");
      break;
    case CMD_BENCHMARK:
    case CMD_EVALUATE:
      config.setString("policyOutputFile", "none");
      break;
    default:
      assert(0); // never reach this point
    }
  }

  // extra debug information with benchmark command
  if (CMD_BENCHMARK == cmd) {
    printf("CFLAGS = %s\n", CFLAGS);
    printf("ARGS = %s\n", outs.str().c_str());
    fflush(stdout);
  }

  // the main show
  switch (cmd) {
  case CMD_SOLVE:
    doSolve(config);
    break;
  case CMD_BENCHMARK:
    doBenchmark(config);
    break;
  case CMD_EVALUATE:
    doEvaluate(config);
    break;
  default:
    assert(0); // never reach this point
  }

  return 0;
}

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 *
 ***************************************************************************/
