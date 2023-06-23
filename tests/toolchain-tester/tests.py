from __future__ import annotations
from typing import TYPE_CHECKING
from typing import Dict, List
from abc import ABC, abstractmethod
import sys

import difflib
import json
import os
import subprocess
import re
from pprint import pprint
from printer import Printer as P
from errors import TestFailure

if TYPE_CHECKING:
    from testsuite import TestSuite

def cleandict(d):
    if d == {} or d == [] or d is None:
       return None
    if not isinstance(d, dict):
       return d
    return dict((k,v) for k,v in sorted(d.items()) if cleandict(v) is not None)

class Test(ABC):
    """
    This class represents a singular test file.
    """

    def __init__(
        self, cpp_file: str, test_json: Dict, index: int, test_suite: TestSuite
    ):
        self.cpp_file: str = cpp_file
        self.test_json: Dict = test_json
        self.test_suite: TestSuite = test_suite

        self._name = cpp_file.split("/")[-1].split(".")[0]
        self.name: str = f"{self._name}_{index}"

        self.fullname: str = f"{test_suite.name}/{self.name}"

        self.out_wasm: str = f"{self._name}.wasm"

        self.success: bool = False

    @abstractmethod
    def _run(self, eosio_cpp: str, args: List[str]):
        pass

    def run(self):
        cf = self.test_json.get("compile_flags")
        args = cf if cf else []
        dir = os.path.dirname(self.cpp_file)
        proto_files = []
        for f in self.test_json.get("proto_files", []):
           proto_files.append(f) 

        if (len(proto_files)):
            args.extend(["--protobuf-files", ";".join(proto_files)])

        proto_dir = self.test_json.get("proto_dir", "")

        if proto_dir:
            args.extend(["--protobuf-dir", os.path.join(dir, proto_dir) ])
        print(args, file=sys.stderr)
        eosio_cpp = os.path.join(self.test_suite.cdt_path, "eosio-codegen")

        self._run(eosio_cpp, args)

    def handle_test_result(self, res: subprocess.CompletedProcess, expected_pass=True):
        stdout = res.stdout.decode("utf-8").strip()
        stderr = res.stderr.decode("utf-8").strip()

        P.print(stdout, verbose=True)
        P.print(stderr, verbose=True)

        if expected_pass and res.returncode > 0:
            self.success = False
            raise TestFailure(
                f"{self.fullname} failed with the following stderr {stderr}",
                failing_test=self,
            )

        if not expected_pass and res.returncode == 0:
            self.success = False
            raise TestFailure(
                "expected to fail compilation/linking but didn't.", failing_test=self
            )

        if not self.test_json.get("expected"):
            self.success = True
        else:
            self.handle_expecteds(res)

    def handle_expecteds(self, res: subprocess.CompletedProcess):
        expected = self.test_json["expected"]

        if expected.get("exit-code"):
            exit_code = expected["exit-code"]

            if res.returncode != exit_code:
                self.success = False
                raise TestFailure(
                    f"expected {exit_code} exit code but got {res.returncode}",
                    failing_test=self,
                )

        if expected.get("stderr"):
            expected_stderr = expected["stderr"]
            actual_stderr = res.stderr.decode("utf-8")

            if expected_stderr not in actual_stderr and not re.search(expected_stderr, actual_stderr, flags=re.S):
                self.success = False
                raise TestFailure(
                    f"expected {expected_stderr} stderr but got {actual_stderr}",
                    failing_test=self,
                )

        if expected.get("abi") or expected.get("abi-file"):
            if expected.get("abi"):
                expected_abi = expected["abi"]
            else:
                full_path = os.path.join(self.test_suite.directory, expected["abi-file"])
                expected_abi_file = open(full_path)
                expected_abi = expected_abi_file.read()
                expected_abi_file.close()

            with open(f"{self._name}.abi") as f:
                actual_abi = f.read()
                expected = json.loads(expected_abi)
                actual = json.loads(actual_abi)

                expected_abi_str = json.dumps(cleandict(expected), indent=2)
                actual_abi_str = json.dumps(cleandict(actual), indent=2)

                if expected_abi_str != actual_abi_str:
                    d = difflib.Differ()
                    diff = d.compare(
                        expected_abi_str.splitlines(), actual_abi_str.splitlines()
                    )
                    pprint(list(diff))
                    self.success = False
                    raise TestFailure(
                        "actual abi did not match expected abi", failing_test=self
                    )

        if expected.get("wasm"):
            expected_wasm = expected["wasm"]

            xxd = subprocess.Popen(("xxd", "-p", self.out_wasm), stdout=subprocess.PIPE)
            tr = subprocess.check_output(("tr", "-d", "\n"), stdin=xxd.stdout)
            xxd.wait()

            actual_wasm = tr.decode("utf-8")

            if expected_wasm != actual_wasm:
                self.success = False
                raise TestFailure(
                    "actual wasm did not match expected wasm", failing_test=self
                )

        self.success = True

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return self.fullname


class BuildPassTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res)

        return res


class CompilePassTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, "-c", self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res)

        return res


class AbigenPassTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res)

        return res


class BuildFailTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res, expected_pass=False)

        return res


class CompileFailTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, "-c", self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res, expected_pass=False)

        return res

class AbigenFailTest(Test):
    def _run(self, eosio_cpp, args):
        command = [eosio_cpp, self.cpp_file]
        command.extend(args)
        res = subprocess.run(command, capture_output=True)
        self.handle_test_result(res, expected_pass=False)

        return res
