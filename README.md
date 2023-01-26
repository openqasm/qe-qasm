# qss-qasm
An OpenQASM Compiler written in C++.

It uses Flex and Bison. So, it's a LALR.

This is a CMake Project. It builds (thus far) four libraries:
- libqasmParser.{so|a}
- libqasmAST.{so|a}
- libqasmFrontend.{so|a}
- libqasmDIAG.{so|a}

and an example program named QasmParser.

These libraries can be used to work on further development of a
fully functional OpenQASM 3.0 Compiler.

How to build and test:
======================

1. Clone this repo.

2. You need to have a recent GCC or LLVM/clang that supports C++17,
   GNU MP, GNU MPFR, GNU MPC, GNU Flex and GNU Bison installed on your system.
   Please make sure you install the 'devel' packages for GNU MP, GNU MPFR
   and GNU MPC.

   Recommended versions are:
   - GCC: >= 9.2
   - LLVM: >= 10.0
   - Bison: >= 3.6.2
   - Flex: >= 2.6.1
   - GNU MP: >= 5.0.0
   - GNU MPFR: >= 4.0.0
   - GNU MPC >= 1.0.0
   - CMake: >= 3.17.0

   Recent versions of Fedora (32 or 33) or Ubuntu (20.X.X) should
   satisfy these version requirements.

   On Fedora or RedHat, the names of the packages are:
    - gmp | gmp-devel
    - mpfr | mpfr-devel
    - libmpc | libmpc-devel

   On Apple Darwin (MacOS):
    - GCC is a symlink to clang. Building with GCC on MacOS is not supported.
    - You need XCode >= 14.2. Lower versions do not handle static initializations
      correctly on MacOS M1 (AArch64).
    - Most of the Toolchain utilities provided by Darwin by default in /usr/bin
      are much too old to be able to build qss-qasm correctly. Recent equivalents
      of these tools must be downloaded from Homebrew [ https://brew.sh/ ].
    - List of dependencies that must be installed from Homebrew:

      - flex
      - bison
      - gmake
      - cmake
      - gmp
      - mpfr
      - libmpc
      - m4

    - Please consult the Homebrew documentation on how to use Homebrew.
    - If you want to use the Homebrew LLVM:
      The LLVM lld linker [lld] needs a symlink created to itself in order to
      work correctly. To do so:
      - cd to the Homebrew LLVM installation directory:
        - /opt/homebrew/opt/llvm/bin on MacOS M1
        - /usr/local/opt/llvm/bin on MacOS x86_64
      - ln -sf lld ld64.lld

3. There are two build Makefiles in the Toplevel directory:
   - Makefile.build.llvm
   - Makefile.build.gcc

4. Depending on which compiler you prefer, copy either of these files
   to the directory above Toplevel:

   %> cp -fp Makefile.build.\<compiler\> ../

   On Darwin (MacOS), the only supported option is Makefile.build.llvm.

   Linking with archive libraries is not supported. The archive libraries
   are being generated for testing and debugging purposes only.

5. cd to the directory above Toplevel:

   %> cd ../

6. Type gmake -f Makefile.build.\<compiler\> configure. This will configure
   the CMake build system for the project. Provided that you have installed
   all the required dependencies from Homebrew correctly, Makefile.build.llvm
   and CMake will auto-detect your specific MacOS configuration.

7. The project uses an out-of-tree build. A new directory named 'build'
   will have been created by the build Makefile.

8. %> cd build

9. Type gmake. This will build the project. The libraries will be found in
   build/lib and the QasmParser example will be in build/bin. You can also
   type 'gmake -j<N>', where <N> is the number of parallel gmake jobs.

10. The project builds both shared and archive libraries.

11. There are a number of OpenQASM 2.0 and OpenQASM 3.0 tests in the
    directory ${TOPLEVEL}/tests.

12. You can run any of these tests manually as:
    ./QasmParser -I../../qasm/tests/include ../../qasm/tests/src/\<test-name\>.qasm
    from the 'build/bin' directory.

13. You can also run the ./short-run-tests.sh shell script found in the tests
   directory. It runs all the tests in the ../../tests/src directory. This script
   is intended to be run from the 'build/bin' directory.

   Alternatively, you can run the 'gmake test' target from the toplevel 'build'
   directory. This will run all the tests using the CMake CTest utility.

14. QasmParser will tell you if it could parse and generate the AST for the
    OpenQASM program correctly, or if it encountered an errror.

15. A pseudo-XML output of the AST being built by the parser will be printed
    either to stdout, or to the file used to capture QasmParser's output.

16. Doing 'gmake DESTDIR=\${DESTDIR}' in the build directory will install the
    required libraries and header files to \${DESTDIR}.

