# (C) Copyright IBM 2024.
#
# This code is part of Qiskit.
#
# This code is licensed under the Apache License, Version 2.0 with LLVM
# Exceptions. You may obtain a copy of this license in the LICENSE.txt
# file in the root directory of this source tree.
#
# Any modifications or derivative works of this code must retain this
# copyright notice, and modified files need to carry a notice indicating
# that they have been altered from the originals.
import os
import platform
import subprocess

from conans import ConanFile
from conans.tools import save
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from setuptools_scm import get_version as get_version_scm


def get_version():
    try:
        return get_version_scm()
    except:  # noqa: E722
        return None


class QasmConan(ConanFile):
    name = "qe-qasm"
    version = get_version()
    url = "https://github.com/openqasm/qe-qasm.git"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "examples": [True, False]}
    default_options = {
        "shared": False,
        "examples": True,
        # Enforce dynamic linking against LGPL dependencies
        "gmp:shared": True,
        "mpc:shared": True,
        "mpfr:shared": True,
    }
    license = "Apache-2.0"
    author = "OpenQASM Organization"
    topics = ("Parser", "OpenQASM3", "Quantum", "Computing")
    description = "A flex/bison parser for OpenQASM v3. A part of the Quantum Engine project."
    generators = "CMakeDeps"

    def requirements(self):
        # Private deps won't be linked against by consumers, which is important
        # at least for Flex which does not expose a CMake target.
        private_deps = ["bison", "flex"]
        for req in self.conan_data["requirements"]:
            private = any(req.startswith(d) for d in private_deps)
            self.requires(req, private=private)

    def build_requirements(self):
        for req in self.conan_data["build_requirements"]:
            self.tool_requires(req)

    def export_sources(self):
        sources = [
            "cmake*",
            "examples*",
            "include*",
            "lib*",
            "licenses*",
            "LICENSE*",
            "utils*",
            "cmake*",
            "tests*",
            "Makefile*",
            "CMakeLists*",
            "VERSION.txt",
        ]
        for source in sources:
            self.copy(source)

        save(os.path.join(self.export_sources_folder, "VERSION.txt"), self.version)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.cache_variables["BUILD_STATIC_LIBS"] = not self.options.shared
        tc.cache_variables["OPENQASM_BUILD_EXAMPLES"] = self.options.examples
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()

        use_monitor = False
        if self.should_build:
            # Note that if a job does not produce output for a longer period of
            # time, then Travis will cancel that job.
            # Avoid that timeout by running vmstat in the background, which reports
            # free memory and other stats every 30 seconds.
            use_monitor = platform.system() == "Linux"
            if use_monitor:
                subprocess.run("free -h ; lscpu", shell=True)
                monitor = subprocess.Popen(["vmstat", "-w", "30", "-t"])
            cmake.build()
        if use_monitor:
            monitor.terminate()
            monitor.wait(1)

        if self.should_test:
            self.test(cmake)

    def test(self, cmake):
        # Tests require examples to be built
        if self.options.examples:
            cmake.test(target="test")

    def package(self):
        cmake = CMake(self)
        cmake.install()
        self.copy("*.tab.cpp", dst="include/lib/Parser", src="lib/Parser")
        self.copy("*.tab.h", dst="include/lib/Parser", src="lib/Parser")

    def package_info(self):
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_file_name", "qasm")
        self.cpp_info.set_property("cmake_target_name", "qasm::qasm")

        self.cpp_info.libs = ["qasmParser", "qasmFrontend", "qasmAST", "qasmDIAG"]
