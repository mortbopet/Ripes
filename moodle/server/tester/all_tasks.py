from tester.task import Task
from tester.task1 import Task1
from tester.task2 import Task2
from tester.task3 import Task3
from tester.task4 import Task4


all_tasks = {
    'SystemTaskID1': Task,
    'SystemTaskID2': Task1,
    'SystemTaskID3': Task2,
    'SystemTaskID4': Task3,
    'SystemTaskID5': Task4,
}


def get_task_by_id(task_id: str):
    if task_id not in all_tasks:
        raise ValueError(f"Task with id {task_id} does not exist")
    return all_tasks[task_id]
