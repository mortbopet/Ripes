from task import Task
import random


class Task1(Task):
    def __init__(self, code_file):
        super().__init__(code_file, regs=True)

    def __generate_tests(self) -> list[dict]:
        tests = []
        for i in range(100):
            a = random.randint(0, 10)
            b = random.randint(0, 10)
            test = {
                "input": None,
                "reginit": {'x1': a, 'x2': b},
                "expected": "",
                "expected_regs": {'x3': a + b}
            }
            tests.append(test)
        return tests

    def __check_test(self, test: dict) -> bool:
        res = self.tester.run(test["input"], test["reginit"])
        return int(res['report']['registers']['x3']) == test["expected_regs"]['x3']
