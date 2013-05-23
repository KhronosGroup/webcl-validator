This document describes how to build and use WebCL Validator.

--------
Building
--------

Currently WebCL Validator is implemented as a Clang tool. Therefore
you need to fetch source code for LLVM and Clang and build WebCL
Validator along with them.

Checkout the required repositories into following locations:
LLVM:             /path/llvm
Clang:            /path/llvm/tools/clang
WebCL Validator:  /path/llvm/tools/clang/tools/webcl-validator

The repositories can be found from:
LLVM:
        git clone http://llvm.org/git/llvm.git
        git checkout -b release_32 origin/release_32
Clang:
        git clone ssh://git@bitbucket.org/vincit/webcl-clang.git
        git checkout -b webcl-validator origin/webcl-validator
WebCL Validator:
        git clone ssh://git@github.com/KhronosGroup/webcl-validator.git

You can now create a build directory and build all three components:
        mkdir /path/build
        cd /path/build
        # cmake uses git-svn if you used git instead of svn
        cmake -DCMAKE_BUILD_TYPE=Debug /path/llvm
        make -j4
        # run regression tests
        make -j4 check-webcl-validator

-------
Running
-------

Run the following command to let WebCL Validator transform your WebCL
kernels:
        webcl-validator kernel.cl -- -x cl -include _kernel.h

Option "-x cl" forces sources to be interpreted as OpenCL code even if
they wouldn't use the ".cl" suffix. Option "-include _kernel.h"
automatically includes OpenCL type and builtin definitions.

You can find the WebCL Validator, _kernel.h and example WebCL kernels
from:
        /path/build/bin
        /path/llvm/tools/clang/tools/webcl-validator/test
