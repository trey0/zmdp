/* tell emacs to use -*- c++ -*- mode */

%module pomdp
%{

#include <iostream>
#include <string.h>
#include <errno.h>
#include "pomdp.h"

%}

%exception python {
  try {
    $function
  } 
  catch (InputError) {
    PyErr_SetString(PyExc_IOError,"");
    return NULL;
  }
}

%include "pomdp.h"
