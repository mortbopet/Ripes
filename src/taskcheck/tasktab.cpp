#include "tasktab.h"
#include "ui_tasktab.h"
#include "processorhandler.h"
#include "io/iomanager.h"

#include <QObject>
#include <QComboBox>
#include <QPushButton>

namespace Ripes {
TaskTab::TaskTab(QToolBar *toolbar, EditTab *edittab, QWidget *parent)
    : RipesTab(toolbar, parent), m_ui(new Ui::TaskTab) {

	m_ui->setupUi(this);
	this->edittab = edittab;
	
	connect(m_ui->sectionBox, &QComboBox::currentIndexChanged,
	this, &TaskTab::changeSection);
	connect(m_ui->numberBox, &QComboBox::currentIndexChanged,
	this, &TaskTab::changeTask);
	connect(m_ui->checkButton, &QPushButton::clicked, this, &TaskTab::checkTask);
	
	m_ui->sectionBox->setEditable(true);
	m_ui->sectionBox->setInsertPolicy(QComboBox::NoInsert);
	m_ui->sectionBox->setPlaceholderText(QStringLiteral("Выберите раздел"));
	m_ui->sectionBox->setCurrentIndex(-1);
	m_ui->numberBox->setEditable(false);
	m_ui->numberBox->setInsertPolicy(QComboBox::NoInsert);
	m_ui->numberBox->setPlaceholderText(QStringLiteral("Выберите задание"));
	m_ui->numberBox->setCurrentIndex(-1);
	m_ui->taskText->setReadOnly(true);
	m_ui->answerText->setReadOnly(true);
	m_ui->checkButton->setEnabled(false);	

	for (unsigned int i = 0; i < taskchecker.getSectionNum(); i++){
		m_ui->sectionBox->addItem(QString::number(i+1), 0);
	}

}

TaskTab::~TaskTab() { delete m_ui; }

void TaskTab::changeSection(){
	if(m_ui->sectionBox->currentIndex() > -1){
		currentSection = std::stoi(m_ui->sectionBox->currentText().toStdString());
	
		m_ui->numberBox->setEditable(true);
		m_ui->numberBox->clear();
		m_ui->numberBox->setCurrentIndex(-1);

		for(const unsigned int& num : taskchecker.getSectionTasks(currentSection)){
			m_ui->numberBox->addItem(QString::number(num), num);
		}
	}
}

void TaskTab::checkTask(){
	std::string answer;
	auto res = ProcessorHandler::getAssembler()->assembleRaw(
      edittab->getAssemblyText(), &IOManager::get().assemblerSymbols());
  	
  	if (res.errors.size() == 0) {
		answer = taskchecker.checkTask(edittab->getAssemblyText(), currentNumber, currentSection);
	} else {
		answer = "No check for program with syntax errors";
	}
	m_ui->answerText->setPlainText(QString::fromStdString(answer) + " - answer\n" + 
	"Current program is:\n" + edittab->getAssemblyText() );
}

void TaskTab::changeTask(){
	if(m_ui->numberBox->currentIndex() > -1){
		currentNumber = std::stoi(m_ui->numberBox->currentText().toStdString());

		m_ui->checkButton->setEnabled(true);
		Task *current = taskchecker.findTask(currentSection, currentNumber);
		if (current != nullptr) {
			m_ui->taskText->setPlainText(QString::fromStdString(current->getText()));
		} else {
			m_ui->taskText->setPlainText("No task");
		}
	} else {
		m_ui->checkButton->setEnabled(false);
	}	
}
}
