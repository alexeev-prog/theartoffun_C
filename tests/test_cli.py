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

    def test_new_operations(self):
        for args, expected in [
            (["--isqrt", "4294967295"], "65535"),
            (["--levenshtein", "kitten", "sitting"], "3"),
            (["--morton-encode", "65535", "65535"], "4294967295"),
            (["--morton-decode", "4294967295"], "65535 65535"),
            (["-p2", "-e10"], "1024.00"),
            (["--sfc32-random"], "sfc32()"),
        ]:
            with self.subTest(args=args):
                result = self.run_cli(*args)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertIn(expected, result.stdout)

    def test_strict_validation(self):
        for args in [
            ["--div3", "-1"], ["--div3", "4294967296"], ["--div3="],
            ["--div3", "3", "extra"], ["--isqrt", "nan"],
            ["--morton-encode", "65536", "1"], ["--levenshtein", "one"],
            ["--fast-mod", "10", "3"], ["--rle-decode", "1048577a"],
            ["--div3", "3", "--isqrt", "4"], ["--div3", "3", "--div3", "4"],
            ["--binary-power", "--exponent", "2"], ["--exponent", "2"],
            ["--fib", "100000"], ["--fib-cache", "nan"],
            ["--leap-year", "1e3"], ["--zellers-day", "31", "2", "2024"],
            ["--xor-swap", "1e2", "3"], ["--fast-pow", "nan", "2"],
            ["--fastest-pow", "2", "inf"], ["--jenkins-hash", "text", "badseed"],
        ]:
            with self.subTest(args=args):
                self.assertNotEqual(self.run_cli(*args).returncode, 0)


if __name__ == "__main__":
    unittest.main()
