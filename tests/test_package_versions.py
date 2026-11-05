"""Check actual find-package version behavior, including exclusive range bounds."""
from pathlib import Path
import subprocess
import tempfile
import unittest

root = Path(__file__).resolve().parents[1]


class PackageVersions(unittest.TestCase):
    def check_version(self, version, compatible, exact=False, maximum=None, bound="INCLUDE"):
        parts = [f'set(PACKAGE_FIND_VERSION "{version}")',
                 f'set(PACKAGE_FIND_VERSION_MAJOR "{version.split(".")[0]}")']
        if maximum is not None:
            parts += ['set(PACKAGE_FIND_VERSION_RANGE TRUE)',
                      f'set(PACKAGE_FIND_VERSION_MIN "{version}")',
                      f'set(PACKAGE_FIND_VERSION_MIN_MAJOR "{version.split(".")[0]}")',
                      f'set(PACKAGE_FIND_VERSION_MAX "{maximum}")',
                      f'set(PACKAGE_FIND_VERSION_RANGE_MAX "{bound}")']
        parts += [f'include("{(root / "cmake/linkerhand-legacy-cpp-sdk-config-version.cmake").as_posix()}")',
                  f'if(NOT "${{PACKAGE_VERSION_COMPATIBLE}}" STREQUAL "{str(compatible).upper()}")',
                  'message(FATAL_ERROR "unexpected compatibility result")', 'endif()',
                  f'if(NOT "${{PACKAGE_VERSION_EXACT}}" STREQUAL "{str(exact).upper()}")',
                  'message(FATAL_ERROR "unexpected exact-version result")', 'endif()']
        with tempfile.TemporaryDirectory() as tmp:
            script = Path(tmp) / "check.cmake"
            script.write_text("\n".join(parts))
            subprocess.run(["cmake", "-P", str(script)], check=True, capture_output=True, text=True)

    def test_versions(self):
        for version, compatible, exact in (("2.0", True, False), ("2.2.1", True, True),
                                            ("2.3", False, False), ("3.0", False, False)):
            with self.subTest(version=version):
                self.check_version(version, compatible, exact)

    def test_ranges(self):
        self.check_version("2.0", True, maximum="3.0", bound="EXCLUDE")
        self.check_version("2.0", False, maximum="2.2.1", bound="EXCLUDE")
        self.check_version("2.0", True, maximum="2.2.1", bound="INCLUDE")
        self.check_version("2.3", False, maximum="3.0")


if __name__ == "__main__":
    unittest.main()
