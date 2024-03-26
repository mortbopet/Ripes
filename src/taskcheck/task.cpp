#include "task.h"

TestCase::TestCase(TestType testtype, std::string input, std::string output){
    this->testtype = testtype;
    this->input = input;
    this->output = output;
}

TestCase::~TestCase(){

}

std::string TestCase::getInput() const
{
    return this->input;
}

std::string TestCase::getOutput() const
{
    return this->output;
}

TestType TestCase::getType() const
{
    return this->testtype;
};

Task::Task(unsigned int section, unsigned int number, std::string text, std::string name)
{
    this->section = section;
    this->number = number;
    this->text = text;
    this->text = name;
}

Task::~Task()
{

}

void Task::addTest(TestCase test)
{
    this->tests.push_back(test);
}

unsigned int Task::getNumber() const
{
    return this->number;
}

unsigned int Task::getSection() const
{
    return this->section;
}

std::string Task::getName() const
{
    return this->name;
}

std::string Task::getText() const
{
    return this->text;
}

std::vector<TestCase> Task::getTests() const
{
    return this->tests;
}