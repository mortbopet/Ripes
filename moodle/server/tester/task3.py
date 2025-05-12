from tester.task import Task
import random


class Task3(Task):
    def __init__(self, code_file):
        super().__init__(code_file, regs=True)
        self.tests = self.__generate_tests()

    def __generate_tests(self) -> list[dict]:
        fib = lambda n: n if n <= 1 else fib(n - 1) + fib(n - 2)
        tests = []
        for i in range(10):
            n = random.randint(1, 20)
            test = {
                "input": None,
                "reginit": {'x10': n},
                "expected": "",
                "expected_regs": {'x10': fib(n)}
            }
            tests.append(test)
        return tests

    def __check_test(self, test: dict) -> bool:
        res = self.tester.run(test["input"], test["reginit"])
        return int(res['report']['registers']['x10']) == test["expected_regs"]['x10']
    
    def run(self) -> float:
        passed = 0
        for test in self.tests:
            if self.__check_test(test):
                passed += 1

        return passed / len(self.tests)
