"""Configure-only dependency boundaries; no SDK libraries or hardware are opened."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class DependencyVersions(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.prefix = self.root / "prefix"
        self.count = 0

    def package(self, name, version, targets):
        directory = self.prefix / "lib/cmake" / name
        directory.mkdir(parents=True, exist_ok=True)
        (directory / (name + "-config.cmake")).write_text(targets)
        script = self.root / "version.cmake"
        script.write_text('include(CMakePackageConfigHelpers)\n'
                          f'write_basic_package_version_file("{directory.as_posix()}/{name}-config-version.cmake" '
                          f'VERSION {version} COMPATIBILITY SameMinorVersion)\n')
        subprocess.run(["cmake", "-P", str(script)], check=True, capture_output=True, text=True)

    def transport(self, version, metadata=None):
        directory = self.root / "transport"
        directory.mkdir(exist_ok=True)
        targets = ('add_library(linkerbot::can INTERFACE IMPORTED GLOBAL)\n'
                   f'set_target_properties(linkerbot::can PROPERTIES LINKERBOT_TRANSPORT_VERSION {metadata or version})\n')
        (directory / "CMakeLists.txt").write_text(
            f'cmake_minimum_required(VERSION 3.16)\nproject(linkerbot-transport VERSION {version} LANGUAGES NONE)\n'
            + targets)
        self.package("linkerbot-transport", version, targets)
        return directory

    def configure(self, options, success=True):
        self.count += 1
        result = subprocess.run([
            "cmake", "-S", str(ROOT), "-B", str(self.root / f"build-{self.count}"),
            "-DBUILD_TESTING=OFF",
            "-DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF", "-DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF",
            "-DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=OFF", *['-DBUILD_EXAMPLES=OFF'],
            f"-DCMAKE_PREFIX_PATH={self.prefix}", *options,
        ], capture_output=True, text=True)
        self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
        return result.stdout + result.stderr

    def test_default_has_no_transport_dependency(self):
        self.configure([])

    def test_transport_source_and_package_versions(self):
        pass
        for source in [True, False]:
            for version in ["0.3.3", "0.3.4", "0.3.5", "0.4.0"]:
                with self.subTest(source=source, version=version):
                    directory = self.transport(version)
                    options = [*['-DLINKERHAND_USE_TRANSPORT=ON']]
                    if source:
                        options += [f"-DLINKERHAND_TRANSPORT_SOURCE_DIR={directory}"]
                    self.configure(options, version in ["0.3.4", "0.3.5"])

    def test_compatible_package_cannot_hide_old_transport_target(self):
        pass
        self.transport("0.3.4", metadata="0.3.3")
        output = self.configure([*['-DLINKERHAND_USE_TRANSPORT=ON']], False)
        self.assertIn("Legacy transport adapter requires", output)


if __name__ == "__main__":
    unittest.main()
