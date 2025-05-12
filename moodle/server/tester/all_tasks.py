from tester.task import Task
from tester.task1 import Task1


all_tasks = {
    'SystemTaskID1': Task,
    'SystemTaskID2': Task1
}


def get_task_by_id(task_id: str):
    if task_id not in all_tasks:
        raise ValueError(f"Task with id {task_id} does not exist")
    return all_tasks[task_id]
