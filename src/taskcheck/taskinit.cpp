#include "task.h"

std::vector<Task> createTaskList(){
	std::vector<Task> list;
	list.push_back(Task(1, 1, "Задание ветвление 1","Задание ветвление 1 текст"));
	list.push_back(Task(1, 2, "Задание ветвление 2","Задание ветвление 2 текст"));
	list.push_back(Task(2, 1, "Задание циклы","Задание циклы"));
	return list;
}

std::vector<std::string> createSectionNames(){
	std::vector<std::string> list;
	list.push_back("Ветвление");
	list.push_back("Циклы");
	return list;
}
