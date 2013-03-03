#!/usr/local/bin/perl -w
#
# Copyright (c) 1996-2005, Carnegie Mellon University. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

######################################################################
# this is the long-hand form of the script we use to calculate CHECKOUT_DIR
# in each Makefile.

$_ = `pwd`;
chop;
$_ .= "/";
# reversing twice is a kludge in order to match right to left
$_ = scalar reverse("$_");
if (s:.*/crs/::) {
    print scalar reverse($_);
} else {
    print "<error>";
    die "*** could not calculate CHECKOUT_DIR ***\n";
}

# and here is the short-hand form
# $(shell perl -e '$$_ = `pwd`; chop; $$_ = scalar reverse("$$_/"); if (s:.*/crs/::) { print scalar reverse($$_); } else { print "<error>"; die "*** could not calculate CHECKOUT_DIR ***\n"; }')
