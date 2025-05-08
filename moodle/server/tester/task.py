from tester.ripes_tester import Tester


class Task:
    def __init__(self, code_file, **kwargs) -> None:
        self.tester = Tester(code_file=code_file, **kwargs)
        self.tests = self.__generate_tests()

    def __generate_tests(self) -> list[dict]:
        tests = [
            {
                "input": None,
                "reginit": None,
                "expected": "",
                "expected_regs": None
            }
        ]
        return tests

    def __check_test(self, test: dict) -> bool:
        res = self.tester.run(test["input"], test["reginit"])
        if res["output"] == test["expected"]:
            return True
        return False

    def run(self) -> float:
        passed = 0
        for test in self.tests:
            if self.__check_test(test):
                passed += 1

        return passed / len(self.tests)
