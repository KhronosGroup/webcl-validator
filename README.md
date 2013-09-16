webcl-validator
===============

This document describes how to build and use WebCL Validator.

Building
--------

Currently WebCL Validator is implemented as a Clang tool. Therefore
you need to fetch source code for LLVM and Clang and build WebCL
Validator along with them.

Checkout the required repositories into following locations:

        LLVM:            /path/llvm
        Clang:           /path/llvm/tools/clang
        WebCL Validator: /path/llvm/tools/clang/tools/webcl-validator

This can be done as follows:

        # Get LLVM 3.2 source

        cd /path/
        git clone http://llvm.org/git/llvm.git
        cd /path/llvm/
        git checkout release_32

        # Get Clang source (with minor changes required by the WebCL Validator)
        
        cd /path/llvm/tools
        git clone https://github.com/KhronosGroup/webcl-clang-dev.git clang
        
        # Get WebCL Validator source

        cd /path/llvm/tools/clang/tools
        git clone https://github.com/KhronosGroup/webcl-validator.git

Depending on your OS/configuration, you may need to install some additional tools:

        sudo yum install git-svn
        sudo yum install cmake28
        alias cmake=cmake28

You can now create a build directory and build all three components:

        mkdir /path/build
        cd /path/build
        # cmake uses git-svn if you used git instead of svn
        # For smaller executable, use -DCMAKE_BUILD_TYPE=MinSizeRel
        cmake -DCMAKE_BUILD_TYPE=Debug /path/llvm
        make -j4 # webcl-validator
        # run regression tests
        make -j4 check-webcl-validator

To get more verbose output:

        make VERBOSE=1 -j4 check-webcl-validator

Linking tests directly with pocl opencl driver (please fix to work without need to recompile the whole thing):

        rm CMakeCache.txt
        make USE_POCL=1 -j4 check-webcl-validator

Running
-------

You can find the WebCL Validator and example WebCL kernels from:

        /path/build/bin
        /path/llvm/tools/clang/tools/webcl-validator/test
        
Run the following command to let WebCL Validator transform your WebCL
kernels:

        webcl-validator kernel.cl [CLANG OPTIONS]

For example, use -D to inform WebCL Validator about used
extensions. Passing -Dcl_khr_initialize memory tells that local memory
initialization shouldn't be done, because the driver should initialize
local memory with cl_khr_initialize_memory extension. See DESIGN.txt
for more details:

        webcl-validator kernel.cl -Dcl_khr_initialize_memory

The validator adds some Clang options automatically. Option *-x cl*
forces sources to be interpreted as OpenCL code even if they wouldn't
use the *.cl* suffix. Option *-include FILE* automatically includes
helper code, such as OpenCL type and builtin definitions.


Building with Windows MinGW + MSYS
----------------------------------

Get CMake for Windows

Get MinGW64 4.6.3 and MSYS rev 12 from (NOTE: MinGW 4.8.1 failed compiling LLVM)

* http://sourceforge.net/projects/mingwbuilds/files/host-windows/
* http://sourceforge.net/projects/mingwbuilds/files/external-binary-packages/

Extract mingw to C:\MinGW64 and MSYS to C:\MinGW64\msys 

Add C:\MinGW64\bin to PATH.

Open MSYS shell and according to normal build instructions except tell cmake to generate MSYS make files

        cmake -G "MSYS Makefiles" /path/llvm

CMake might fail to some error, if so run it again. Usually it works second time and build files will be generated. Compilation requires ~4GB RAM.

If you want to build also OpenCL programs and possibly run the tests, set

        export CPLUS_INCLUDE_PATH=/C/path/to/opencl/include
        export LIBRARY_PATH=/C/path/to/opencl/lib

