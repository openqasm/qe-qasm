# qe-qasm
An Quantum Engine's implementation of an OpenQASM v3.0 lexer and parser written in C++. The Quantum Engine is a system comprised of low-level software components enabling the execution of quantum programs in quantum hardware.

The parser is based on [Flex](https://github.com/westes/flex) and [Bison](https://www.gnu.org/software/bison/),
and therefore it is a [LALR parser](https://en.wikipedia.org/wiki/LALR_parser).

This is a CMake Project. It builds four libraries:
- libqasmParser.{so|a}
- libqasmAST.{so|a}
- libqasmFrontend.{so|a}
- libqasmDIAG.{so|a}

and a simple CLI tool [QasmParser](./examples/QasmParser/) for demonstrations and testing.

These libraries can be used to work on further development of a
fully functional OpenQASM 3 compiler. In particular, the library is used by the [qe-compiler](https://github.com/openqasm/qe-compiler) to
parse OpenQASM 3 source files to MLIR.

## Contents
- [qe-qasm](#qe-qasm)
  - [Contents](#contents)
  - [Building from source](#building-from-source)
    - [Conan](#conan)
      - [Building for development and debugging](#building-for-development-and-debugging)
    - [Make](#make)
      - [Running Tests](#running-tests)
  - [Static Code Checks](#static-code-checks)
  - [CI and Release Cycle](#ci-and-release-cycle)
    - [CI](#ci)
    - [Branches](#branches)
    - [Tags](#tags)
    - [Release cycle](#release-cycle)
    - [Example release cycle](#example-release-cycle)


## Building from source

The parser's build system is driven by [Cmake](https://cmake.org/).
For simplicity of building, packaging and distribution we provide a [conan](https://conan.io/)
packager and recommend this for development/integration.

Currently the supported platforms are Linux and OSX. It is possible to build on Windows using
[WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

### Conan
- Clone this repo: `git clone git@github.com:openqasm/qe-qasm.git`
- Install build dependencies: `pip install -r requirements-dev.txt`
  - It is recommended to use a Python virtual environment for this
- The package may be built and installed to conan with: `conan create . --build=outdated -pr:h default -pr:b default`
   - This will build the conan package and install it locally. The version will be detected automatically from the repo tag.
   - If you wish to override the package name, version or remote do so by calling conan with `conan create . <package>/<version>@remote -pr:h default -pr:b default`

#### Building for development and debugging
- Create a build directory: `mkdir build && cd build/`
- Install package with: `conan install .. --build=outdated -pr:h default -pr:b default` which will install and build all missing dependencies
- Build the package with: `conan build ..`
- The package tests may be run with: `conan build .. --test`


### Make

- Clone this repo: `git clone git@github.com:openqasm/qe-qasm.git`
- Install build dependencies: `pip install -r requirements-dev.txt`
   - It is recommended to use a Python virtual environment for this
- Install a recent GCC or LLVM/clang compiler that supports C++17,
   GNU MP, GNU MPFR, GNU MPC, GNU Flex and GNU Bison installed on your system.
   Please make sure you install the `devel` packages for GNU MP, GNU MPFR
   and GNU MPC.

   Recommended versions are:
   - GCC: >= 9.2
   - LLVM: >= 10.0
- Install the package dependencies:
  - Versions:
   - Bison: >= 3.6.2
   - Flex: >= 2.6.1
   - GNU MP: >= 5.0.0
   - GNU MPFR: >= 4.0.0
   - GNU MPC >= 1.0.0
   - CMake: >= 3.17.0
  - Platforms:
    - Linux: Recent versions of Fedora (32 or 33) or Ubuntu (20.X.X) should
   satisfy these version requirements.
     - On Fedora (RedHat), the names of the packages are:
      - gmp(gmp-devel)
      - mpfr(mpfr-devel)
      - libmpc(libmpc-devel)

     - Apple Darwin (MacOS):
      - GCC is a symlink to clang. Building with GCC on MacOS is not supported.
      - You need XCode >= 14.2. Lower versions do not handle static initializations
        correctly on MacOS M1 (AArch64).
      - Most of the Toolchain utilities provided by Darwin by default in /usr/bin
        are much too old to be able to build qe-qasm correctly. Recent equivalents
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
- Prepare the build directory:  `mkdir build && cd build`
- Configure: `cmake -G "Unix Makefiles" ..`
  - If dependencies are not installed or properly setup errors may be raised
- Build with: `make`
  - This will build both static and shared libraries
-  Running: `make DESTDIR=/path/to/DESTDIR/` in the build directory will install the
    required libraries and header files to `/path/to/DESTDIR/`.

#### Running Tests

- There are a number of OpenQASM 2.0 and OpenQASM 3.0 tests in the
    directory `./tests``.
- You can run any of these tests manually as: `./QasmParser -I../../qasm/tests/include ../../qasm/tests/src/<test-name>.qasm` from the 'build/bin' directory.
- You can also run the ``./short-run-tests.sh`` shell script found in the tests
   directory. It runs all the tests in the ``../../tests/src`` directory. This script
   is intended to be run from the `build/bin`` directory.
   Alternatively, you can run the 'make test' target from the toplevel `build``
   directory. This will run all the tests using the CMake CTest utility.
- QasmParser will tell you if it could parse and generate the AST for the
    OpenQASM program correctly, or if it encountered an errror.
- A pseudo-XML output of the AST being built by the parser will be printed
    either to stdout, or to the file used to capture QasmParser's output.


## Static Code Checks
The easiest, fastest, and most automated way to integrate the formatting into your workflow
is via [pre-commit](https://pre-commit.com). Note that this tool requires an internet connection
to initially setup because the formatting tools needs to be downloaded.

**In environments without an internet connection, please see one of the other solutions documented
below.**

These should be installed and setup prior to any development work so that they are not forgotten
about. The setup is straight forward:

```bash
pip install pre-commit
pre-commit install
```

The first time `pre-commit install` is run, it will take a minute to setup the environment.

After installation, the hooks will be run prior to every commit, and will be run against all staged
changes. Optionally, you can trigger this run via `pre-commit run`.

If you wish to run the hooks against the entire repository, run:

```bash
pre-commit run --all-files
```

The other checks that are performed can be seen in
[`.pre-commit-config.yaml`](.pre-commit-config.yaml). At the time of writing, these are:

- No direct committing to `main`, or `release/*`
- Check json for validity
- Ensure newline character at the end of files
- Trim end of line whitespace characters
- Check for no merge conflict lines accidentally being staged
- Clang format
- Python block (line length 100)


## CI and Release Cycle
Please keep the following points in mind when developing:

### CI
CI is currently run on every pull request against the `main` branch.
All CI processes are based on the [conan package](#conan) build.

### Branches

* `main`:
The main branch is used for the development of the next release.
It is updated frequently and should *not* be considered stable. On the development
branch, breaking changes can and will be introduced.
All efforts should be made to ensure that the development branch is maintained in
a self-consistent state that is passing continuous integration (CI).
Changes should not be merged unless they are verified by CI.
* `release/<major.minor>` branches:
Branches under `release/<major.minor>` are used to maintain released versions of the qss-qasm parser.
They contain the version of the parser corresponding to the
release as identified by its [semantic version](https://semver.org/). For example,
`release/1.5` would be the compiler version for major version 1 and minor version 5.
On these branches, the parser
is considered stable. The only changes that may be merged to a release branch are
patches/bugfixes. When a patch is required when possible the fix should
first be made to the development branch through a pull request.
The fix should then be backported from the development branch to the
target release branch (of name `release/<major.minor>`) by creating a pull request on
Github into the target release branch with the relevant cherry-picked commits.
The new release branch `HEAD` should be tagged (see [Tags](#tags)) with a new
`<major.minor.patch>` version and pushed to Github.

### Tags
Git tags are used to tag the specific commit associated with a versioned release.
Tags must take the form of `v<major>.<minor>.<patch>-<labels>`. For example the semver
`v1.5.1` would point to the parser release with major version 1,
minor version 5, and, patch version 1. The current development version would therefore be MINOR+1
`v1.6.0`. All official releases when tagged must always point to the current HEAD
of a release branch.

### Release cycle
To release a version a new version:

- (Option A) If releasing a major/minor version create a new release branch for the version (See [Branches](#branches)).
   This should be cut from the latest development branch.
   ```bash
   git checkout -b release/<version> <base>
   git push -u origin release/<version>
   ```
- (Option B) If releasing a patch version:
  -  checkout the existing release branch for your target major/minor version to apply the patch
   ```bash
   git checkout -b <backport>-<desc>-release/<version> release/<version>
   ```
  - Apply your changes (or cherry-pick existing commits) to your new branch and then push your branch to Github
   ```bash
   git push -u origin <your-branch>
   ```
  - Make a PR from your new branch into the target `release/<version>` branch with the form `[Backport] <Title>` and merge the PR
- Create a new tag with the required semantic version number (see [Tags](#tags)), tagging the `HEAD` of the target `release/<version>` branch.
  Push the tag to Github which will trigger CI.
    ```bash
    git tag -a v<version> -m "<description> e.g. release v<x>.<y>.<z>" # <- where version is the version number of the tag.
    git push -u origin v<version>
    ```

### Example release cycle

For this example assume the current release of the parser is version `0.5.1`. This will correspond to a commit
on `release/0.5`. The project's development branch reflects the development state of the next release - `0.6.0`
and is referred to by version as `0.6.0-dev`.

To trigger a bugfix release - `0.5.2`:
1. Create a PR into `release/0.5` with all required changes. The PR ideally should begin with title of the form `[Backport] <Title>`.
   These may be backported commits from `main`.
2. Upon merger of the PR tag the HEAD of `release/0.5` with `v0.5.2` and push to Github.

To trigger a minor release - `0.6.0`:
1. Create a new release branch `release/0.6` using the current development branch (`main`) as the base branch, eg., `git checkout -b release/0.6 main`.
   - *Note*: Branch protection rules are in place for release branches and these steps may only be completed by project
administrators.
2. Push this branch to Github.
3. Tag the branch with `v0.6.0` and push to Github.
