from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class EmbeddedConan(ConanFile):
    name = "embedded"
    version = "1.0.0"
    license = "Apache-2.0"

    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        if self.settings.os != "baremetal":
            # ConanCenter-confirmed
            self.requires("libsodium/1.0.20")
            self.requires("zeromq/4.3.5")        # provides libzmq
            self.requires("protobuf/5.27.0")
            self.requires("spdlog/1.14.1")
            self.requires("fftw/3.3.10")
            self.requires("open62541/1.5.0")
            self.requires("pybind11/2.13.6")
            # Verify availability/version before relying on these:
            # self.requires("czmq/...")           # check: conan search czmq -r=conancenter
            # self.requires("volk/...")            # check: conan search volk -r=conancenter
        else:
            self.requires("etl/20.38.10")

    def _target_id(self):
        if self.settings.os == "baremetal" and self.settings.arch == "armv7":
            return "cortex-m4"
        return "host"

    def layout(self):
        self.folders.source = "."
        subdir = f"build/{self.settings.build_type}/{self._target_id()}"
        self.folders.build = subdir
        self.folders.generators = f"{subdir}/generators"

    def generate(self):
        CMakeDeps(self).generate()
        tc = CMakeToolchain(self)
        # Default prefix ("conan") produces the same "conan-<build_type>"
        # preset name regardless of target, so host and cortex-m4 collide
        # when both are included from the same CMakeUserPresets.json.
        tc.presets_prefix = f"conan-{self._target_id()}"
        tc.generate()

