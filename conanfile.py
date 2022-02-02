from conans import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.layout import cmake_layout
import os


class Forest(ConanFile):
    name = "forest"
    version = "0.1"
    requires = ["sqlite3/3.37.2", "sqlitecpp/3.1.1"]
    settings = "build_type"

    exports_sources = "CMakeLists.txt", "include/*", "test/*", "cmake/*"
    no_copy_source = True

    def layout(self):
        self.folders.build = os.path.join(
            "build", self.settings.get_safe("build_type", default="Release"))
        self.folders.generators = self.folders.build

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "forest")
        self.cpp_info.set_property("cmake_target_name", "forest")
        self.cpp_info.set_property("cmake_find_mode", "both")
        self.cpp_info.set_property("cmake_build_modules", [os.path.join(
            "lib", "cmake", "forest", "forestTargets.cmake")])
