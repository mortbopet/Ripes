#include <QString>
#include "task.h"
class TaskChecker {

public:
	TaskChecker();
	~TaskChecker();
	std::string checkTask(QString program, unsigned int section, unsigned int number);
	unsigned int getSectionNum() const;
	std::vector<unsigned int> getSectionTasks(unsigned int section) const;
	Task* findTask(unsigned int section, unsigned int number);
private:
	std::vector<Task> tasks;
	std::vector<std::string> sections;

	void setTasks();
	void setSections();
	bool checkPrintVal(QString program, std::string input, std::string output, std::string &answer);
	bool checkReturnVal(QString program, std::string input, std::string output, std::string &answer);	

};
