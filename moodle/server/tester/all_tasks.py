from tester.task import Task
from tester.task1 import Task1
from tester.task2 import Task2
from tester.task3 import Task3


all_tasks = {
    0: Task,
    1: Task1,
    2: Task2,
    3: Task3
}


def get_task_by_id(task_id):
    if task_id not in all_tasks:
        raise ValueError(f"Task with id {task_id} does not exist")
    return all_tasks[task_id]
