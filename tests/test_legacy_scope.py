import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("legacy_web", root / "webui/run.py")
web = importlib.util.module_from_spec(spec)
spec.loader.exec_module(web)


class LegacyScopeTests(unittest.TestCase):
    def test_model_and_protocol_boundary(self):
        self.assertEqual(set(web.COMM_SUPPORT), {"L7", "L20_10", "L21", "L25", "O20"})
        self.assertEqual(web.COMM_SUPPORT["L7"], ["can", "modbus"])
        self.assertEqual(web.COMM_SUPPORT["O20"], ["canfd"])
        for name in ("L20_10", "L21", "L25"):
            self.assertEqual(web.COMM_SUPPORT[name], ["can"])

    def test_aliases_preserve_g20_identity(self):
        for alias in ("G20", "L20_11", "l20_10"):
            self.assertEqual(web.canonical_model(alias), "L20_10")
        self.assertEqual(web.canonical_model("L20"), "L20")

    def test_invalid_model_or_transport_never_spawns_bridge(self):
        with patch.object(web.subprocess, "Popen") as spawn:
            for model, comm in (("O6", "can"), ("L20", "can"), ("L21", "modbus"), ("O20", "can")):
                with self.assertRaises(ValueError):
                    web.start_bridge("unused", model, "left", comm, "can0")
            spawn.assert_not_called()


if __name__ == "__main__":
    unittest.main()
