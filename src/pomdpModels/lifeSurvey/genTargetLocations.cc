/********** tell emacs we use -*- c++ -*- style comments *******************
 Copyright (c) 2006, Trey Smith. All rights reserved.

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

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "LSModelFile.h"

using namespace std;
using namespace zmdp;

static double unitRand(void) {
  return (static_cast<double>(rand())) / RAND_MAX;
}

void usage(const char *binaryName) {
  cerr << "usage: " << binaryName
       << " OPTIONS <foo.lifeSurvey>\n"
          "  -h or --help   Display this help\n"
          "  -s or --seed   Set random seed\n"
          "  -n or --noise  Set noise factor\n";
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  static char shortOptions[] = "hs:n:";
  static struct option longOptions[] = {{"help", 0, NULL, 'h'},
                                        {"seed", 1, NULL, 's'},
                                        {"noiseFactor", 1, NULL, 'n'},
                                        {NULL, 0, 0, 0}};

  int seed = -1;
  double noiseFactor = 0;
  while (1) {
    char optchar = getopt_long(argc, argv, shortOptions, longOptions, NULL);
    if (optchar == -1) break;

    switch (optchar) {
      case 'h':  // help
        usage(argv[0]);
        break;

      case 's':  // seed
        seed = atoi(optarg);
        break;

      case 'n':  // noise
        noiseFactor = atof(optarg);
        break;

      case '?':  // unknown option
      case ':':  // option with missing parameter
        // getopt() prints an informative error message
        cerr << endl;
        usage(argv[0]);
        break;
      default:
        abort();  // never reach this point
    }
  }
  if (argc - optind != 1) {
    cerr << "ERROR: wrong number of arguments (should be 1)" << endl << endl;
    usage(argv[0]);
  }

  const char *modelFileName = argv[optind++];

  if (-1 == seed) {
    seed = time(NULL) % RAND_MAX;
  }
  srand(seed);

  LSModelFile m;

  // read the map in
  m.readFromFile(modelFileName);

  // add noise to priors (if any)
  std::vector<double> noisyPriors = m.regionPriors;
  if (noiseFactor > 0) {
    for (int r = 0; r < static_cast<int>(noisyPriors.size()); r++) {
      double noise = (2 * unitRand() * noiseFactor) - noiseFactor;
      noisyPriors[r] *= (1.0 + noise);
    }
  }

  // set locations of targets
  std::vector<LSPos> targets;
  for (int x = 0; x < static_cast<int>(m.grid.width); x++) {
    for (int y = 0; y < static_cast<int>(m.grid.height); y++) {
      unsigned char r = m.grid.getCell(LSPos(x, y));
      if (LS_OBSTACLE != r) {
        double prob = noisyPriors[r];
        bool hasTarget = (unitRand() < prob);
        m.grid.setCell(LSPos(x, y), hasTarget);
        if (hasTarget) {
          targets.push_back(LSPos(x, y));
        }
      }
    }
  }

  // write the resulting target map back out to stdout
  printf("noise=%lf seed=%d numTargets=%d\n", noiseFactor, seed,
         static_cast<int>(targets.size()));
  m.grid.writeToFile(stdout, /* showCoords = */ true, /* binaryMap = */ true);
  printf("--");
  for (int i = 0; i < static_cast<int>(targets.size()); i++) {
    if (i % 5 == 0) {
      printf("\n");
    }
    printf("%2d,%2d ", targets[i].x, targets[i].y);
  }
  printf("\n");
}
