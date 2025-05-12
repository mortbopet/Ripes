from tester.task import Task
from tester.task1 import Task1
from tester.task2 import Task2


all_tasks = {
    0: Task,
    1: Task1,
    2: Task2
}


def get_task_by_id(task_id):
    if task_id not in all_tasks:
        raise ValueError(f"Task with id {task_id} does not exist")
    return all_tasks[task_id]
