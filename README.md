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

        # Get LLVM 3.4 source

        cd /path/
        git clone --single-branch http://llvm.org/git/llvm.git -b release_34
        cd /path/llvm/

        # Get Clang 3.4
        
        cd /path/llvm/tools
        git clone --single-branch http://llvm.org/git/clang.git -b release_34
        

        # Get WebCL Validator source and add it to CMakeLists.txt for compiling

        cd /path/llvm/tools/clang/tools
        git clone https://github.com/KhronosGroup/webcl-validator.git
        echo "add_subdirectory(webcl-validator)" >> CMakeLists.txt

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

To disable certain extensions from the default set of supported
extensions you may use the --disable=xx switch:

        webcl-validator kernel.cl --disable=cl_khr_fp16

Building with Windows MinGW + MSYS (not tested recently since we changed to Visual Studio express)
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

Building with Windows / Visual Studio
-----------------------------

Get Visual Studio 2012 (I used Express for Windows Desktop version)

Get GnuWin32 tools (http://getgnuwin32.sourceforge.net/)

- install all packages to c:\gnuwin32
- append c:\gnuwin32\bin to path

Get Python 2.7 (http://www.python.org/download/)

Get CMake 2.8 (http://www.cmake.org/cmake/resources/software.html)

Get Intel OpenCL SDK (http://software.intel.com/en-us/vcsource/tools/opencl-sdk)

Copy these items from a MSysGit installation to somewhere fairly late in PATH (e.g. c:\gnuwin32\bin)

- msys-1.0.dll
- bash.exe
- sh.exe

do not put the entire MSYS environment in PATH; for example MSYS grep is incompatible with LLVM/Clang tests.

Replace (overwrite) some old/broken GnuWin32 stuff with:

- Newer libxml2: ftp://ftp.zlatkovic.com/libxml/
- Non-broken grep(1): https://code.google.com/p/unix-cmd-win32/downloads/detail?name=grep-2.9-w32.zip

Then open VS2012 developer command prompt, go to a suitable build dir and do

    cmake -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 11"

Open the generated llvm.sln file. Build by building the "ALL_BUILD" project, don't use Build Solution / F7.

Run tests by building "check-webcl-validator" project, found under "WebCL Validator Tests" in the Solution Explorer.


Building with Xcode
--------------------

1. Generate xcode project:
        
        cmake -G Xcode /path/llvm

or if want to compile test bench against pocl driver:
        
        USE_POCL=1 cmake -G Xcode /path/llvm
        

2. Open and build in Xcode or from commandline

        xcodebuild -configuration MinSizeRel -target check-webcl-validator


