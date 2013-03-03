SPECIFYING A CUSTOM MDP FOR THE ZMDP LIBRARY

  Copyright (c) 2006, Trey Smith.
  More information is in the file named COPYING.

This document briefly describes how to specify your own MDP model by
modifying the source files CustomMDP.h and CustomMDP.cc.

The ZMDP library represents MDP and POMDP problems as instances of the
abstract class 'MDP'.  Therefore, the basic approach to defining a new
domain is to create a class that derives from the MDP class and
implements the virtual functions declared in MDP.

In order to make that process less painful, I've written a derived class
called CustomMDP (found in the files CustomMDP.h and CustomMDP.cc) that
can serve as a template for your own MDP.  As it stands, CustomMDP is a
complete working derived class that implements a very simple example
problem.  I've called out places where you will likely want to make
changes with 'USER CUSTOMIZE' comments.  Hopefully the example code and
comments you find in those places will be informative enough that you
can modify the code to fit your own domain.

When you are done modifying CustomMDP.cc, you should be able to simply
recompile ZMDP -- the CustomMDP model is linked into the ZMDP binaries
as part of the normal compilation process. You can apply a solver to the
custom model by specifying 'custom' where you would normally give the
filename of the model to read in.

A more in-depth MDP domain is the racetrack domain implemented in the
files RaceTrack.h and RaceTrack.cc.  That implementation allows you to
specify different instances of the domain by reading in a map file at
run time.  Racetrack is considerably more complicated and less
thoroughly documented than CustomMDP, but you may still find it to be a
useful source of example code.

==========================
CUSTOM MDP EXAMPLE PROBLEM

Here is a brief description of the example problem currently implemented
in CustomMDP.  It is an indoor robot navigation problem.  The robot is
in a hallway that is n grid cells long and one grid cell wide, as shown
in the following map, with n=5:

 -----------
 |R| | | |G|
 -----------
  0 1 2 3 4

The "R" location (x=0) is the known starting state of the robot, and the
"G" location (x=4) is the goal.

At each time step, the robot may attempt to move either the right or the
left.  If the move succeeds (probability 90%), the robot moves one
square in the specified direction.  If the move fails (probability 10%),
the robot's position remains unchanged.  Illegal moves which would send
the robot off the map always fail.  Each time step that the robot moves,
it incurs a cost of 1.  When the robot reaches the (G) position the
problem terminates; by convention terminal states are represented as
absorbing states (self-transition probability 100%) in which all actions
have cost 0.

(Of course, the optimal policy for this problem is that the robot should
always try to move to the right.)

The value of the constant 'n' is specified in the file
src/main/zmdp.config using the 'customMDPNumStates' parameter.  You can
override the default value from the command line with the option e.g.
'--customMDPNumStates 10'.

==========================================================
$Id: README_CustomMDP.txt,v 1.1 2006-11-07 20:10:11 trey Exp $
