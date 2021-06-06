/********** tell emacs we use -*- c++ -*- style comments *******************
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

#ifndef ZMDP_SRC_COMMON_ZMDPCONFIG_H_
#define ZMDP_SRC_COMMON_ZMDPCONFIG_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace zmdp {

struct ZMDPConfig {
  typedef std::map<std::string, std::string> CMap;
  CMap cmap;
  CMap aliases;
  std::string sourceName;
  bool noNewFieldsAllowed;

  ZMDPConfig(void);

  void readFromStream(const std::string &_sourceName, std::istream &in);
  void readFromFile(const std::string &fname);
  void readFromString(const std::string &_sourceName, const char *buf);

  void readFromConfig(const ZMDPConfig &x);

  const std::string &getString(const std::string &field) const;
  int getInt(const std::string &field) const;
  double getDouble(const std::string &field) const;
  bool getBool(const std::string &field) const;
  bool defined(const std::string &field) const;

  void setString(const std::string &field, const std::string &val);
  void setInt(const std::string &field, int val);
  void setDouble(const std::string &field, double val);
  void setBool(const std::string &field, bool val);

  void setAlias(const std::string &field, const std::string &val);
  std::string expandAliases(const std::string &args);
  std::vector<std::string> processArgs(const std::string &args);

  void setNoNewFieldsAllowed(bool _noNewFieldsAllowed);

  void writeToStream(std::ostream &out) const;
};

};  // namespace zmdp

extern int zmdpDebugLevelG;

#endif  // ZMDP_SRC_COMMON_ZMDPCONFIG_H_
