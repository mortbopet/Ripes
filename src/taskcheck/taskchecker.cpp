#include <algorithm>

#include "taskchecker.h"
#include "taskinit.h"

TaskChecker::TaskChecker()
{
    setTasks();
    setSections();
}

TaskChecker::~TaskChecker()
{
        
}

std::string TaskChecker::checkTask(QString program, unsigned int section, unsigned int number)
{
    auto check = [section, number](const Task& task) {
        return section == task.getSection() && number == task.getNumber();
    };
    auto taskIt = find_if(this->tasks.begin(), this->tasks.end(), check);

    if (taskIt !=  tasks.end() ){
        bool passed = true;
        std::string curAnswer;
        auto testIt = taskIt->getTests().begin();

        while(testIt != taskIt->getTests().end() && passed){
	    TestCase test = (*testIt);
            if(test.getType() == TestType::returnValue){
                passed = checkReturnVal(program, test.getInput(), test.getOutput(), curAnswer);
            } else {
                passed = checkPrintVal(program, test.getInput(), test.getOutput(), curAnswer);
            }
            testIt++;
        }
        return curAnswer;
    }
    return "No task with these numbers\n";
}

void TaskChecker::setTasks()
{
    this->tasks = createTaskList();
}

void TaskChecker::setSections()
{
    this->sections = createSectionNames();
}

bool TaskChecker::checkPrintVal(QString program, std::string input, std::string output, std::string &answer)
{
    answer = "tests havent been implemented yet \n";
    return true;
}

bool TaskChecker::checkReturnVal(QString program, std::string input, std::string output, std::string &answer)
{
    answer = "tests havent been implemented yet \n";
    return true;
}

unsigned int TaskChecker::getSectionNum() const{
    return sections.size();
}

std::vector<unsigned int> TaskChecker::getSectionTasks(unsigned int section) const
{
    std::vector<unsigned int> sectionTaskNums;
    for (const Task& task : tasks){
        if(task.getSection() == section){
            sectionTaskNums.push_back(task.getNumber());
        }
    }
    return sectionTaskNums;
}

Task* TaskChecker::findTask(unsigned int section, unsigned int number)
{
     auto check = [section, number](const Task& task) {
        return section == task.getSection() && number == task.getNumber();
    };

    auto taskIt = find_if(this->tasks.begin(), this->tasks.end(), check);

    if (taskIt != tasks.end()){
        return &(*taskIt);
    }
    return nullptr;    
}
