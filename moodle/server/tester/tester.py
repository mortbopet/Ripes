import json
import re
from subprocess import run, CompletedProcess

regs_dict = {
    "zero": "x0",
    "ra": "x1",
    "sp": "x2",
    "gp": "x3",
    "tp": "x4",
    "t0": "x5",
    "t1": "x6",
    "t2": "x7",
    "s0": "x8",
    "s1": "x9",
    "a0": "x10",
    "a1": "x11",
    "a2": "x12",
    "a3": "x13",
    "a4": "x14",
    "a5": "x15",
    "a6": "x16",
    "a7": "x17",
    "s2": "x18",
    "s3": "x19",
    "s4": "x20",
    "s5": "x21",
    "s6": "x22",
    "s7": "x23",
    "s8": "x24",
    "s9": "x25",
    "s10": "x26",
    "s11": "x27",
    "t3": "x28",
    "t4": "x29",
    "t5": "x30",
    "t6": "x31",
}


class Tester:
    def __init__(self, path="app/tester", code_file="input.s", code_file_type='s', proc='RV32_5S', timeout=1000,
                 **kwargs) -> None:
        if timeout <= 0:
            raise ValueError("Timeout cannot be negative")

        # self.run_options = {
        #     "path": path,
        #     "code_file": code_file,
        #     "code_file_type": code_file_type,
        #     "proc": proc,
        #     "timeout": timeout
        # }

        self.test_run_strings = [
            f"{path}/create-display.sh",
            f"{path}/Ripes.AppImage",
            f"--appimage-extract-and-run",
            f"--mode cli",
            f"--src {code_file}",
            f"-t {code_file_type}",
            f"--proc {proc}",
            f"--timeout {timeout}",
        ]

        possible_options = ["isaexts", "verbose", "all", "cycles", "iret", "cpi", "ipc", "pipeline", "regs",
                            "runinfo", "reginit", "output"]

        self.has_report = False
        self.reginit_index = -1
        self.output = None

        for key, value in kwargs.items():
            if key not in possible_options:
                raise ValueError(f"{key} is not a valid ripes cli option")

            if key in ["verbose", "all", "cycles", "iret", "cpi", "ipc", "pipeline", "regs", "runinfo"]:
                if not isinstance(value, bool):
                    raise TypeError(f"{key} should be a boolean")
                if value:
                    self.test_run_strings.append(f"--{key}")
                    if key in ["all", "cycles", "iret", "cpi", "ipc", "pipeline", "regs",
                               "runinfo"] and not self.has_report:
                        self.test_run_strings.append("--json")
                        self.has_report = True

            if key == "isaexts":
                if isinstance(value, str):
                    self.test_run_strings.append(f"--isaexts {value}")
                elif isinstance(value, list):
                    self.test_run_strings.append(f"--isaexts {','.join(value)}")
                else:
                    raise TypeError(f"Invalid type for {key}")

            if key == "output":
                if not isinstance(value, str):
                    raise TypeError("Output must be a path to a file")
                self.output = value

            if key == "reginit":
                self.__set_reginit(value)

    def __run(self, reginit: str | dict = None, run_input: str = None) -> CompletedProcess:
        if reginit is not None:
            tmp = self.test_run_strings[self.reginit_index] if self.reginit_index != -1 else None
            self.__set_reginit(reginit)
            res = run(self.test_run_strings, capture_output=True, text=True, input=run_input)
            self.__set_reginit(tmp)
            return res
        return run(self.test_run_strings, capture_output=True, text=True, input=run_input)

    def run(self, reginit: str | dict = None, run_input: str = None) -> CompletedProcess | (CompletedProcess, dict):
        res = self.__run(reginit, run_input)
        if "Program exited with code: 0\n" not in res.stdout:
            if "ERROR" in res.stdout:
                raise Exception(res.stdout)
            raise ValueError("Program haven't exited or got non-zero exit code")
        if self.has_report:
            return res, self.__get_report(res)
        return self.__run(reginit, run_input)

    def __get_report(self, proc: CompletedProcess | str = None) -> dict:
        if self.output is None:
            if proc is None:
                raise ValueError("Haven't provided a process or a stdout string to ger report from")
            if isinstance(proc, CompletedProcess):
                return json.loads(proc.stdout.split("Program exited with code: 0\n")[1])
            if isinstance(proc, str):
                return json.loads(proc.split("Program exited with code: 0\n")[1])
            raise TypeError("Invalid proc type")
        with open(self.output, 'r') as file:
            report = json.load(file)
        return report

    def __set_reginit(self, reginit) -> None:
        if reginit is None and self.reginit_index != -1:
            self.test_run_strings.pop(self.reginit_index)
            self.reginit_index = -1
            return
        if isinstance(reginit, str):
            self.test_run_strings.append(f"--reginit {reginit}")
            return
        if isinstance(reginit, dict):
            regs_regex = r"^x(\d|1\d|2\d|3[0,1])$"
            reginit_list = []
            for reg_key, reg_value in reginit.items():
                reg = reg_key
                if reg_key in regs_dict:
                    reg = regs_dict[reg_key]
                if re.match(regs_regex, reg) is None:
                    raise ValueError(f"Invalid register {reg_key}")
                reg_index = reg[1:]
                if reg_index == "0":
                    raise ValueError(f"Register {reg_key} cannot be set")
                reginit_list.append(f"{reg_index}={reg_value}")
            reginit_list = ",".join(reginit_list)
            if self.reginit_index != -1:
                self.test_run_strings[self.reginit_index] = f"--reginit {reginit_list}"
                return
            self.reginit_index = len(self.test_run_strings)
            self.test_run_strings.append(f"--reginit {reginit_list}")
            return
        raise TypeError("Reginit must be a string or a dict")
