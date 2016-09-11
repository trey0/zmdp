ZMDP Software for POMDP and MDP Planning
========================================

This C++ source code implements several approximate value iteration
algorithms for MDPs and POMDPs, including both algorithms I developed
(FRTDP 7_, HSVI2 6_) and some earlier algorithms that work on the same
problems (RTDP 1_, LRTDP 3_, and HDP 2_).  The software takes a
problem model file as input.  Currently supported model formats are Tony
Cassandra's POMDP file format 4_ and a new format I developed for
describing problems in the racetrack domain from 1_.

Special note: my implementations of some of the earlier algorithms are
based on publications that necessarily left out a lot of detail.  It's
quite possible that my implementation differs from the original in
important ways.  The original authors have not signed off on my
implementations: use at your own risk.  (And please inform me if you
find any errors.)

More information can be found at http://longhorizon.org/trey/zmdp

Please let me know if you have any comments!

Trey Smith <trey.smith@longhorizon.org>

Copyright (c) 2005-2016, Trey Smith.
More information is in the file named COPYING.

.. contents::

Package contents
~~~~~~~~~~~~~~~~

================ ===========================================================
README           This file
COPYING          Copyright information
src/build        Makefile system
src/common       Utility code, including a sparse linear algebra library
src/mdps         Racetrack domain and some example problems
src/bounds       Value function bounds representations
src/search       Search strategies FRTDP, HSVI, etc.
src/exec         Interface you can use to execute policies output by zmdp
src/parsers      Model parsing code (thanks to Tony Cassandra!)
src/pomdpCore    Basic POMDP operations and simulation
src/pomdpBounds  Value function bounds representations specific to POMDPs
src/pomdpModels  Some example POMDP problems
src/main         Glue code for invoking algorithms from the shell
src/tools        Tools for scripting multiple runs and plotting performance
src/tests        Regression testing suite
================ ===========================================================

Requirements
~~~~~~~~~~~~

Operating systems: Linux and Mac OS X

Some tested compilers:

 * gcc/g++ 3.2.2 under Linux Fedora Core 2
 * gcc/g++ 4.0.1 under Linux Fedora Core 4
 * gcc/g++ 4.0.1 from XCode 2.4 under Mac OS X 10.4.8
 * gcc/g++ 4.2.1 from XCode 3.2 under Mac OS X 10.6.4

Build system tested with:

 * GNU Make 3.80, 3.81
 * GNU Flex 2.5.4, 2.5.35
 * GNU Bison 2.0, 2.3
 * Perl 5.8.6, 5.10.0

Performance plotting scripts in tools directory tested with:

 * gnuplot 4.0
 * GNU gprof 2.15.94.0.2.2
 * GNU Octave 2.1.73

(Note: I don't frequently update this section, so the known-good version
information for the tools may get out of date as I upgrade my testing
machines.  Sorry.)

Installation instructions
~~~~~~~~~~~~~~~~~~~~~~~~~

 * Unpack the tarball to generate the zmdp directory::

    tar xvfz zmdp-VERSION.tgz

 * Build::

    cd zmdp-VERSION/src
    make install
    (generates binaries in zmdp-VERSION/bin/<os_name>/, <os_name> = e.g. linux2, darwin8)

 * Test (runs a suite of regression tests)::

    cd zmdp-VERSION/src
    make test

 * Try solving an example problem (look at results in 'out.policy')::

    mkdir zmdp-VERSION/results/
    cd zmdp-VERSION/results
    ../bin/<os_name>/zmdp solve ../src/pomdpModels/three_state.pomdp

 * If you want to solve one of the included RockSample problem
   instances, generate it (generates RockSample_5_7.pomdp)::

    cd zmdp-VERSION/src/pomdpModels
    ./gen_RockSample_5_7

Usage instructions
~~~~~~~~~~~~~~~~~~

The installation process creates binaries in the ``bin/<os_name>/``
directory:

zmdp solve
  'zmdp solve' generates an output policy for a search strategy and
  problem you select.  It runs the search strategy in an anytime
  fashion, periodically printing bounds on the value of the initial
  state to console so that you can monitor progress.  When the run ends,
  the final policy is output to the file you specify.  There are several
  options for how to end the run: you can specify a desired regret bound
  ('precision') for the output solution, specify a fixed timeout, or
  just use ctrl-C to interrupt the algorithm when you are satisfied (it
  will output the final policy before exiting).

  Run ``zmdp -h`` for a summary of the options and some examples.

zmdp benchmark
  'zmdp benchmark' tests performance for a search strategy and problem
  you select.  As the search strategy progresses, various statistics
  are logged, such as the algorithm's bounds on the value of the initial
  state, the number of backups so far, and periodic measurements of the
  solution quality in simulation.  The plotting tools in the src/tools
  directory can be used to plot performance based on the logs.

  Logged data includes:
   * bounds.plot: bounds on initial state, internal algorithm state such as
     number of trials, number of backups, etc.
   * inc.plot: logs periodic measurements of solution quality in simulation
   * sim.plot: logs some of the simulation traces so you can get a
     qualitative sense of how the policy changes

  Run ``zmdp -h`` for a summary of the options and some examples.

zmdp evaluate
  'zmdp evaluate' evaluates a policy output by 'zmdp solve' or 'zmdp benchmark'.
  The policy is evaluated by executing a number of trials in simulation;
  the output is mean reward and a 95% confidence interval for the estimate
  of the mean under the (not necessarily correct) assumption that the
  per-trial reward is normally distributed.  Sorry, statisticians may wince...

  Run ``zmdp -h`` for a summary of the options and some examples.

Plotting performance
~~~~~~~~~~~~~~~~~~~~

There are several scripts in the src/tools directory, a few of which
are described here.  They are intended to produce performance plots
based on the logs output by 'zmdp benchmark'.  Sorry, these tools are
somewhat messier / less documented than the solver itself.

makeplot
--------

usage: makeplot OPTIONS <results_directory>
   -h    Print this help
   -i    Plot bounds vs. number of top-level iterations
   -p    Generate postscript output

makeplot plots the upper bound, lower bound, and measured solution
quality at testing epochs in a single graph with a logarithmic x axis.
It looks in <results_directory> for the files bounds.plot and inc.plot,
output by 'zmdp benchmark'.  By default the plot is displayed in an X window.
Using the -p option drops it into a PostScript file instead, e.g. for
inclusion in a paper.

compareplots
------------

usage: compareplots OPTIONS <dir1> <dir2>
   -h    Print this help
   -t    Plot bounds vs. time
   -i    Plot bounds vs. number of top-level iterations
   -s    Plot speed (iterations v. time)
   -p    Generate postscript output

compareplots shows solution quality vs. wallclock time for two
directories in a single plot.  Anytime solution quality bounds are not
shown with the default options.

speedplot
---------

Speedplot is similar to compareplots, but uses number of updates on the
x axis rather than wallclock time.  You may prefer this so that results
are less dependent on the platform used.  A downside is that the amount
of time required for each update varies widely depending on the search
strategy and especially on the bounds representation.

Acknowledgments
~~~~~~~~~~~~~~~

The ideas behind the novel algorithms in this codebase were joint work
with Reid Simmons.  Thanks to Geoff Gordon for helpful discussions.
Thanks to Tony Cassandra for developing the POMDP problem format and
distributing his problem reading and solution code.  Thanks to Matthijs
Spaan for distributing his PERSEUS POMDP-solving code, which provided a
template for my release of zmdp.

References
~~~~~~~~~~

.. _1:

1. `Learning to Act Using Real-Time Dynamic
   Programming. <http://www.sciencedirect.com/science/article/pii/000437029400011O>`_
   A. Barto, S. Bradke, and S. Singh.
   *Artificial Intelligence* 72(1-2): 81-138. 1995.

.. _2:

2. `Faster heuristic Search Algorithms for Planning with
   Uncertainty and Full Feedback. <http://dl.acm.org/citation.cfm?id=1630835>`_
   B. Bonet and H. Geffner. In *Proc. of IJCAI*, 2003.

.. _3:

3. `Labeled RTDP: Improving the Convergence of Real Time Dynamic Programming.
   <ftp://ftp.cs.ucla.edu/pub/stat_ser/R319.pdf>`_
   B. Bonet and H. Geffner. In *Proc. of ICAPS*, 2003.

.. _4:

4. `Tony's POMDP Page <http://www.cs.brown.edu/research/ai/pomdp/>`_.
   Tony Cassandra.

.. _5:

5. `Heuristic Search Value Iteration for
   POMDPs. <http://longhorizon.org/trey/pubs/b2hd-smith04hsvi.html>`_
   T. Smith and R. Simmons. In  *Proc. of UAI*, 2004.

.. _6:

6. `Point-based POMDP Algorithms: Improved Analysis and
   Implementation. <http://longhorizon.org/trey/pubs/b2hd-smith05hsvi.html>`_
   T. Smith and R. Simmons. In *Proc. of UAI*, 2005.

.. _7:

7. `Focused Real-Time Dynamic Programming for
   MDPs. <http://longhorizon.org/trey/pubs/b2hd-smith06frtdp.html>`_
   T. Smith and R. Simmons. In *Proc. of AAAI*, 2006.

Release notes
~~~~~~~~~~~~~

Changes from 1.1.0 to 1.1.7 (10 Sep 2009)

 * Tweaked build system to re-enable running ranlib over libraries.
   Seems to be required under Mac OS X 10.6 with some versions of Xcode
   (1.1.7, svn revision 1027).

 * Fixed several problems introduced by stricter gcc 4.3 standards
   compliance.  Examples: added #include <string.h> and <limits.h> in
   several places; added 'const' modifiers; now use c++0x unordered_map
   in place of GNU hash_map extension (1.1.6, svn revision 1026).

 * Changed 'unsigned pos' to 'size_t pos' in zmdpConfig.cc
   for 64-bit compatibility (1.1.6, svn revision 1025).

 * Added #include of signal.h in zmdp.cc to get sigaction() declaration.
   g++ became more strict about requiring #includes in version 4.x.
   (1.1.5, svn revision 1023).

 * Same Mac OS X version string problem cropped up again with Leopard; improved
   the build system fix to be more robust against future changes (1.1.5,
   svn revision 1023).

 * Fixed Mac OS X-specific compile error -- the version string returned by
   gcc changed after the transition to Intel processors, breaking a
   conditional in the build system (1.1.4, svn revision 1022).

 * Fixed compile error in the seldom-used example program
   src/exec/testExec.cc, which was trying to use an older version of the
   BoundPairExec interface (1.1.3).

 * Added capability for zmdpEvaluate to read in policies in the
   alpha vector format output by Tony Cassandra's pomdp-solve
   with the '-t cassandraAlpha' option.  However, at this point
   ZMDP does not output policies in this format (1.1.2).

 * Sped up policy evaluation by removing some unnecessary cache
   resets (1.1.2).

 * Added a "margin of error" to adaptive maximum depth calculation
   in FRTDP to avoid the potential for FRTDP to run an infinite
   number of trials without increasing the maximum depth (although
   this pathological behavior was never observed) (1.1.2).

 * Fixed invalid use of uninitialized memory in HDP implementation.  This
   calls into question earlier performance evaluations for HDP, but
   luckily the bug appears not to have had a significant impact (1.1.2).

 * Made further improvements to policy evaluation, including improved
   caching and sample reweighting to reduce variance (1.1.1).

 * Fixed a bug that caused a crash during policy evaluation with the
   hallway.pomdp problem (1.1.1).

Changes from 1.0.2 to 1.1.0 (24 Mar 2007)

 * Greatly improved speed of policy evaluation in zmdpBenchmark and
   zmdpEvaluate with better caching.  The improvements are controlled
   with the 'useEvaluationCache' config option, which is turned on by
   default.  Increased the default value of 'evaluationTrialsPerEpoch'
   to 1000.

 * Changed how confidence intervals are calculated when reporting
   expected long-term reward of a policy.  We now use the bootstrap
   method, which does not rely on a normality assumption.  The bootstrap
   result is expressed as 'mean min max' rather than 'mean +/- delta'.
   This forced the format of the 'inc.plot' output file and the
   zmdpEvaluate console output to change slightly.

 * Substantial code refactoring, which largely consisted of combining
   mostly redundant code blocks to improve maintainability.

Changes from 1.0.0 to 1.0.2 (22 Feb 2007)

 * Fixed problem of intermittent crashing when selecting actions in
   policy evaluation of certain POMDPs (1.0.2).

 * Fixed crashing problem when zmdpEvaluate is run with distinct
   planner and evaluation models (1.0.1).

Changes from 0.6.4 to 1.0.0 (9 Nov 2006)

 * Added ability to specify arbitrary discrete MDPs in the MDP variant
   of Tony Cassandra's POMDP format.

Changes from 0.6.0 to 0.6.4 (31 Oct 2006)

 * Added a simple template for implementing your own MDP model, see
   src/mdps/CustomMDP.{h,cc} (0.6.4).

 * Added ability to request a log of Q values for all queried nodes
   at the end of a run, using the 'qValuesOutputFile' parameter (0.6.4).

 * Fixed compilation problem under Mac OS X (0.6.3).

 * Several bug fixes and new regression tests, now properly respects
   optional speedup flags for POMDP bounds representations (0.6.2).

 * Old 'valueFunctionRepresentation' parameter now split into separate
   'lowerBoundRepresentation' and 'upperBoundRepresentation' parameters
   for more flexibility (0.6.1).

 * Added regression tests: run with 'make test' (0.6.1).

 * Fixed bug with zmdpEvaluate crashing (0.6.1).

Changes from 0.5.5 to 0.6.0 (17 Oct 2006)

 * Added configuration file support, modified command-line options to
   front-end binaries.  Several parameters that were previously
   hard-coded can now be configured at run-time.

 * Fixed problem with zmdpSolve crashing on some POMDP models; the
   problem was caused by subtle errors in upper bound pruning with -v
   convex.

Changes from 0.5.0 to 0.5.5 (2 Oct 2006)

 * Added --max-horizon option to zmdpSolve and zmdpBenchmark.  The
   new parameter allows solution of undiscounted POMDP problems (0.5.5).

 * Another fix for zmdpEvaluate crashes, specifically when the fast
   model parser is not used (0.5.4).

 * Fixed zmdpEvaluate crashes.  Added --max-steps option to zmdpEvaluate
   (0.5.3).

 * Added some caching and optimizations to the 'convex' value function
   for POMDPs (-v convex).  On large sparse problems, I measured up to 8x
   speedup relative to the old implementation (0.5.2).

 * Added beta version of 'zmdpEvaluate' binary, which reads in a policy
   (currently POMDP only) and evaluates it in simulation.  The model
   used in simulation can be different from the model used in planning
   (0.5.2).

 * PomdpExec is now an abstract class with one derived implementation.
   This will make it easier to add different policy types later (0.5.1).

Changes from 0.4.2 to 0.5.0 (24 Jun 2006)

 * Added PomdpExec, an interface you can use to write an executive that
   reads in and executes POMDP policies output by zmdp.  (See
   src/pomdpExec/testPomdpExec.cc for an example.)

 * Added ability for zmdpBenchmark to write out a policy file at each
   evaluation epoch.

 * Added an early version of the LifeSurvey POMDP domain (this domain may
   change in future releases).

Changes from 0.4.0 to 0.4.2 (5 Jun 2006)

 * Fixed initialization of bounds when '-v point' is used with POMDPs
   (0.4.2).

 * Fixed syntax error in the problem generated by
   pomdpModels/gen_RockSample_5_5 (0.4.2).

 * Fixed a bug which caused zmdpSolve to crash when convex bounds
   and FRTDP were used together on some problems (0.4.1).

Changes from 0.3.1 to 0.4.0 (1 Jun 2006)

 * The POMDP model file format has changed slightly to restore
   conformance with Tony Cassandra's definition.  The zmdp language
   extension for declaring terminal states ('E: ...') is no longer
   necessary and no longer supported.  To my knowledge, only models that
   I created and distributed in zmdp use E: declarations; those models
   have been updated by commenting out E: lines.

 * Initialization with the standard parser (used when zmdpSolve is
   invoked without -f) is now much more efficient in time and memory.
   Use of -f should no longer be necessary in most cases.

Changes from 0.3.0 to 0.3.1 (12 May 2006)

 * Fixed a spurious error about policy output not being supported.

Changes from 0.2.1 to 0.3.0 (28 Apr 2006)

 * Refactored code so that different problem types, search strategies,
   and value function representations can be mixed and matched.

 * There are now separate binaries for benchmarking algorithms
   (with incremental policy evaluation) and just solving particular
   POMDPs (with policy output at the end).

 * No longer split binaries for each solution algorithm -- there are too
   many combinations to have a different binary for each.

Changes from 0.2.0 to 0.2.1 (14 Apr 2006)

 * Fixed problem with directory structure in tarball that broke
   compilation.

 * Performance vs. number of updates for the HSVI algorithm is now
   output in the bounds.plot file in the same format as for other
   algorithms, suitable for plotting using the speedplot utility.

Changes from zpomdp 0.1 to zmdp 0.2.0 (27 Feb 2006)

 * zmdp now includes MDP solving as well as POMDP solving
   capabilities (hence the name change).

 * Created uniform MDP/POMDP API so that POMDPs formulated as
   belief-space MDPs can be passed to MDP heuristic search algorithms.

 * Some code has been refactored to reduce code duplication and
   provide cleaner interfaces.  This involved shuffling directories.

 * Implemented racetrack domain and some sample problems from the
   literature.

 * Implemented RTDP, LRTDP, HDP, HDP+L, and FRTDP.

 * Separate binaries are now produced for each (problem type, algorithm)
   pair rather than a single unified binary.

zpomdp 0.1.0 (28 Oct 2005)

 * Initial release.
