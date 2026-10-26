"""CLI integration tests using only Python's standard library."""
import pathlib
import subprocess
import sys
import unittest

BINARY = str(pathlib.Path(sys.argv.pop(1)).resolve())


class CLITests(unittest.TestCase):
    def run_cli(self, *args):
        return subprocess.run(
            [BINARY, *args], text=True, capture_output=True, timeout=10
        )

    def test_help(self):
        result = self.run_cli("--help")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("Usage:", result.stdout)

    def test_binary_power(self):
        result = self.run_cli("--binary-power", "2", "--exponent", "10")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("1024.00", result.stdout)

    def test_invalid_power(self):
        for args in [
            ["--binary-power", "garbage", "--exponent", "2"],
            ["--binary-power", "2", "--exponent", "1.5"],
            ["--binary-power", "2", "--exponent=-1"],
            ["--binary-power", "2"],
            ["--binary-power", "nan", "--exponent", "2"],
            ["--binary-power", "2", "--exponent", "18446744073709551616"],
        ]:
            with self.subTest(args=args):
                self.assertNotEqual(self.run_cli(*args).returncode, 0)

    def test_parser_errors(self):
        for args in [["--not-an-option"], ["--binary-power"]]:
            with self.subTest(args=args):
                self.assertNotEqual(self.run_cli(*args).returncode, 0)

    def test_compression(self):
        result = self.run_cli("--rle-encode", "aaabbc")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("3a2bc", result.stdout)
        result = self.run_cli("--rle-decode", "1000a")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("a" * 1000, result.stdout)
        for option, text in [("--rle-decode", "12"), ("--rle-encode", "abc123")]:
            self.assertNotEqual(self.run_cli(option, text).returncode, 0)


if __name__ == "__main__":
    unittest.main()
