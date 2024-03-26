#include <string>
#include <vector>

#pragma once
enum class TestType {
  returnValue,
  printValue
};

class TestCase {

public:
	TestCase(TestType testtype, std::string input, std::string output);
	~TestCase();
	std::string getInput() const;
	std::string getOutput() const;
	TestType getType() const;

private:
	TestType testtype;
	std::string input;
	std::string output;
};

class Task {

public:
	Task(unsigned int section, unsigned int number, std::string name, std::string text);
	~Task();
	void addTest(TestCase test);
	unsigned int getNumber() const;
	unsigned int getSection() const;
	std::string getName() const;
	std::string getText() const;
	std::vector<TestCase> getTests() const;
	
private:
	unsigned int number;
	unsigned int section;
	std::string name;
	std::string text;
	std::vector<TestCase> tests;

};