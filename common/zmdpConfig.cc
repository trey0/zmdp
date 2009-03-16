/********** tell emacs we use -*- c++ -*- style comments *******************
 $Revision: 1.4 $  $Author: trey $  $Date: 2007-07-08 20:57:24 $
   
 @file    zmdpConfig.cc
 @brief   No brief

 Copyright (c) 2006, Trey Smith.

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
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "zmdpConfig.h"
#include "zmdpCommonDefs.h"

using namespace std;

namespace zmdp {

static bool allWhiteSpace(const char* buf)
{
  for (const char* p = buf; *p != '\0'; p++) {
    if (!isspace(*p)) return false;
  }
  return true;
}

static void replace_string(string& s,
			   const string& pattern,
			   const string& replacement) {
  size_t pos;
  while (string::npos != (pos = s.find(pattern))) {
    s.replace(pos, pattern.size(), replacement);
  }
}

static std::vector<std::string> split(const string& stringToSplit) {
  char *s = strdup(stringToSplit.c_str());
  std::vector<std::string> ret;
  
  char *tok, *tmp = s;
  while (0 != (tok = strtok(tmp," \t"))) {
    tmp = 0;
    ret.push_back(tok);
  }

  free(s);

  return ret;
}

ZMDPConfig::ZMDPConfig(void) :
  noNewFieldsAllowed(false)
{}

void ZMDPConfig::readFromStream(const std::string& _sourceName, std::istream& in)
{
  sourceName = _sourceName;

  char buf[1024], field[1024], val[1024];
  int lnum = 0;
  while (1) {
    in.getline(buf, sizeof(buf));
    lnum++;
    if (in.eof()) break;
    if (allWhiteSpace(buf)) continue;
    if ('#' == buf[0]) continue;
    if (0 == strncmp(buf, "alias", 5)) {
      // line is an alias

      // eat 'alias'
      char* bufp = strtok(buf, " \t");

      // get field
      bufp = strtok(NULL, " \t");
      if (NULL == bufp) {
	fprintf(stderr, "ERROR: %s:%d: syntax error, expected 'alias <field> <value>'\n",
		sourceName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      snprintf(field, sizeof(field), "%s", bufp);

      // rest of line is value
      bufp = strtok(NULL, "");
      if (NULL == bufp) {
	fprintf(stderr, "ERROR: %s:%d: syntax error, expected 'alias <field> <value>'\n",
		sourceName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      snprintf(val, sizeof(field), "%s", bufp);

      setAlias(field, val);
    } else {
      // line is config data
      if (2 != sscanf(buf, "%s %s", field, val)) {
	fprintf(stderr, "ERROR: %s:%d: syntax error, expected '<field> <value>'\n",
		sourceName.c_str(), lnum);
	exit(EXIT_FAILURE);
      }
      setString(field, val);
    }
  }
}

void ZMDPConfig::readFromFile(const std::string& fname)
{
  ifstream in(fname.c_str());
  if (!in) {
    fprintf(stderr, "ERROR: couldn't open config file '%s' for reading: %s\n",
	    fname.c_str(), strerror(errno));
    exit(EXIT_FAILURE);
  }
  readFromStream(fname, in);
  in.close();
}

void ZMDPConfig::readFromString(const std::string& _sourceName, const char* buf)
{
  istringstream in(buf);
  readFromStream(_sourceName, in);
}

void ZMDPConfig::readFromConfig(const ZMDPConfig& x)
{
  typeof(x.cmap.begin()) xi;
  for (xi = x.cmap.begin(); xi != x.cmap.end(); xi++) {
    setString(xi->first, xi->second);
  }
}

const std::string& ZMDPConfig::getString(const std::string& field) const
{
  typeof(cmap.begin()) ci = cmap.find(field);
  if (cmap.end() == ci) {
    fprintf(stderr, "ERROR: required config field '%s' was not specified\n", field.c_str());
    exit(EXIT_FAILURE);
  }
  return ci->second;
}

int ZMDPConfig::getInt(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like an integer here
  return atoi(s.c_str());
}

double ZMDPConfig::getDouble(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like a double here
  return atof(s.c_str());
}

bool ZMDPConfig::getBool(const std::string& field) const
{
  const std::string& s = getString(field);
  // it would be smart to check if s looks like a boolean value here
  return (0 != atoi(s.c_str()));
}

bool ZMDPConfig::defined(const std::string& field) const
{
  return (cmap.end() != cmap.find(field));
}

void ZMDPConfig::setString(const std::string& field, const std::string& val)
{
  if (noNewFieldsAllowed) {
    if (!defined(field)) {
      fprintf(stderr, "ERROR: unknown config field '%s' was specified\n", field.c_str());
      exit(EXIT_FAILURE);
    }
  }

  cmap[field] = val;
}

void ZMDPConfig::setInt(const std::string& field, int val)
{
  ostringstream out;
  out << val;
  setString(field, out.str());
}

void ZMDPConfig::setDouble(const std::string& field, double val)
{
  ostringstream out;
  out << setprecision(20) << val;
  setString(field, out.str());
}

void ZMDPConfig::setBool(const std::string& field, bool val)
{
  setInt(field, val);
}

void ZMDPConfig::setAlias(const std::string& field, const std::string& val)
{
  aliases[field] = val;
}

std::string ZMDPConfig::expandAliases(const std::string& args)
{
  // pad with spaces so padded fields will match
  string ret = (" " + args + " ");

  const int numPasses = 3;
  for (int i=0; i < numPasses; i++) {
    FOR_EACH (pr, aliases) {
      string field = " " + pr->first + " ";
      string val = " " + pr->second + " ";
      replace_string(ret, field, val);
    }
  }

  return ret;
}

std::vector<std::string> ZMDPConfig::processArgs(const std::string& argStr)
{
  std::vector<std::string> args = split(expandAliases(argStr));
  std::vector<std::string> unprocessedArgs;

  bool pastOptions = false;
  typeof(args.begin()) argp;
  for (argp = args.begin(); argp != args.end(); argp++) {
    string arg = *argp;
    if (allWhiteSpace(arg.c_str())) continue;
    
    if (!pastOptions && '-' == arg[0]) {
      if (arg == "--") {
	// "end of options" marker
	pastOptions = true;
      } else if (arg.substr(0,2) == "--") {
	// option
	if (++argp == args.end()) {
	  fprintf(stderr,
		  "ERROR: got %s option without argument (-h for help)\n",
		  arg.c_str());
	  exit(EXIT_FAILURE);
	}
	setString(arg.substr(2), *argp);
      } else {
	// non-option argument
	unprocessedArgs.push_back(arg);
      }
    } else {
      // non-option argument
      unprocessedArgs.push_back(arg);
    }
  }

  return unprocessedArgs;
}

void ZMDPConfig::setNoNewFieldsAllowed(bool _noNewFieldsAllowed)
{
  noNewFieldsAllowed = _noNewFieldsAllowed;
}

void ZMDPConfig::writeToStream(std::ostream& out) const
{
  FOR_EACH (pr, aliases) {
    out << "alias " << pr->first << " " << pr->second << endl;
  }
  FOR_EACH (pr, cmap) {
    out << pr->first << " " << pr->second << endl;
  }
}

};

int zmdpDebugLevelG = 0;

/***************************************************************************
 * REVISION HISTORY:
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2006/10/30 20:23:03  trey
 * added writeToStream()
 *
 * Revision 1.2  2006/10/30 20:00:15  trey
 * USE_DEBUG_PRINT replaced with a run-time config parameter "debugLevel"
 *
 * Revision 1.1  2006/10/15 21:41:22  trey
 * initial check-in
 *
 *
 *
 ***************************************************************************/
