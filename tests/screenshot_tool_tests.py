"""Exercise screenshot request ownership and completion without launching Godot."""
import importlib.util
import json
from pathlib import Path
import tempfile
import threading
import unittest

spec = importlib.util.spec_from_file_location("screenshot", Path(__file__).resolve().parents[1] / "tools/screenshot.py")
screenshot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(screenshot)


class ScreenshotToolTests(unittest.TestCase):
    def test_timeout_removes_only_own_request(self):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / "latest.json").write_text('{"request_id":"old", "ok":true, "files":[]}', encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "Timed out"):
                screenshot.capture(folder, 0)
            self.assertFalse((folder / "capture.request").exists())
            self.assertTrue((folder / "latest.json").exists())

    def test_existing_request_is_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            request = Path(directory) / "capture.request"
            request.write_text("someone-else", encoding="utf-8")
            with self.assertRaisesRegex(RuntimeError, "already pending"):
                screenshot.capture(Path(directory), 0)
            self.assertEqual(request.read_text(encoding="utf-8"), "someone-else")

    def test_completion_and_game_error(self):
        for error in ("", "Rendering unavailable"):
            with self.subTest(error=error), tempfile.TemporaryDirectory() as directory:
                folder = Path(directory)
                stop = threading.Event()
                observed = []

                def game():
                    request = folder / "capture.request"
                    while not stop.wait(0.001):
                        if request.exists():
                            token = request.read_text(encoding="utf-8")
                            observed.append(token)
                            result = {"request_id": token, "ok": not error, "error": error,
                                      "files": [{"path": "captured.png"}] if not error else []}
                            (folder / "latest.json").write_text(json.dumps(result), encoding="utf-8")
                            request.unlink(missing_ok=True)
                            return

                worker = threading.Thread(target=game)
                worker.start()
                try:
                    if error:
                        with self.assertRaisesRegex(RuntimeError, error):
                            screenshot.capture(folder, 2)
                    else:
                        self.assertEqual(screenshot.capture(folder, 2), [{"path": "captured.png"}])
                finally:
                    stop.set()
                    worker.join()
                self.assertEqual(len(observed), 1)
                self.assertEqual(len(observed[0]), 32, "Only complete request tokens are published")
                self.assertFalse((folder / "capture.request").exists())


if __name__ == "__main__":
    unittest.main()
