#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "ccmanager.h"
#include "ripessettings.h"

#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStackedWidget>

namespace Ripes {

std::pair<QWidget*, QGridLayout*> constructPage() {
    auto* pageWidget = new QWidget();
    auto* pageLayout = new QVBoxLayout();
    auto* itemLayout = new QGridLayout();

    pageLayout->addLayout(itemLayout);
    pageWidget->setLayout(pageLayout);
    pageLayout->addStretch(1);

    return {pageWidget, itemLayout};
}

template <typename T_Widget>
std::pair<QLabel*, T_Widget*> createSettingsWidgets(const QString& settingName, const QString& labelText) {
    auto* label = new QLabel(labelText);
    T_Widget* widget = new T_Widget();

    const auto* settingObserver = RipesSettings::getObserver(settingName);

    if constexpr (std::is_same<T_Widget, QSpinBox>()) {
        // Ensure that the current value can be represented in the spinbox. It is expected that the spinbox range will
        // be specified after being created in this function.
        widget->setRange(INT_MIN, INT_MAX);
        widget->setValue(settingObserver->value().toUInt());
        widget->connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), settingObserver,
                        &SettingObserver::setValue);
    } else if constexpr (std::is_same<T_Widget, QLineEdit>()) {
        widget->connect(widget, &QLineEdit::textChanged, settingObserver, &SettingObserver::setValue);
        widget->setText(settingObserver->value().toString());
    }

    return {label, widget};
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::SettingsDialog) {
    m_ui->setupUi(this);
    m_ui->settingsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    setWindowTitle("Settings");

    // Create settings pages
    addPage("Editor", createEditorPage());
    addPage("Simulator", createSimulatorPage());

    m_ui->settingsList->setCurrentRow(0);
}

SettingsDialog::~SettingsDialog() {
    delete m_ui;
}

void SettingsDialog::accept() {
    QDialog::accept();
}

QWidget* SettingsDialog::createEditorPage() {
    auto [pageWidget, pageLayout] = constructPage();

    // Setting: RIPES_SETTING_CCPATH
    CCManager::get();

    auto* CCGroupBox = new QGroupBox("RISC-V C/C++ Compiler");
    auto* CCLayout = new QVBoxLayout();
    CCGroupBox->setLayout(CCLayout);
    auto* CCDesc = new QLabel(
        "Providing a compatible RISC-V C/C++ compiler enables editing, compilation "
        "and execution of C-language programs within Ripes.\n\n"
        "A compiler may be autodetected if availabe in PATH.");
    CCDesc->setWordWrap(true);
    CCLayout->addWidget(CCDesc);

    auto* CCHLayout = new QHBoxLayout();
    auto [cclabel, ccpath] = createSettingsWidgets<QLineEdit>(RIPES_SETTING_CCPATH, "Compiler path:");
    m_ccpath = ccpath;
    CCLayout->addLayout(CCHLayout);
    CCHLayout->addWidget(cclabel);
    CCHLayout->addWidget(ccpath);
    auto* pathBrowseButton = new QPushButton("Browse");
    connect(pathBrowseButton, &QPushButton::clicked, [=, ccpath = ccpath] {
        QFileDialog dialog(this);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec()) {
            ccpath->setText(dialog.selectedFiles()[0]);
        }
    });

    // Make changes in the CC path trigger revalidation in the CCManager
    connect(ccpath, &QLineEdit::textChanged, &CCManager::get(), &CCManager::trySetCC);

    // Make CCManager compiler changes trigger background color of the ccpath, indicating whether the CC was determined
    // to be valid
    connect(&CCManager::get(), &CCManager::ccChanged, this, &SettingsDialog::CCPathChanged);

    // Trigger initial rehighlighting
    CCManager::get().trySetCC(CCManager::get().currentCC());

    CCHLayout->addWidget(pathBrowseButton);

    pageLayout->addWidget(CCGroupBox);

    return pageWidget;
}

void SettingsDialog::CCPathChanged(bool valid) {
    QPalette palette = this->palette();
    if (!valid) {
        palette.setColor(QPalette::Base, QColor(Qt::red).lighter());
    } else {
        palette.setColor(QPalette::Base, QColor(Qt::green).lighter());
    }
    m_ccpath->setPalette(palette);
}

QWidget* SettingsDialog::createSimulatorPage() {
    auto [pageWidget, pageLayout] = constructPage();

    // Setting: RIPES_SETTING_REWINDSTACKSIZE
    auto [rewindLabel, rewindSpinbox] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_REWINDSTACKSIZE, "Max. undo cycles:");
    rewindSpinbox->setRange(0, INT_MAX);

    pageLayout->addWidget(rewindLabel, 0, 0);
    pageLayout->addWidget(rewindSpinbox, 0, 1);

    return pageWidget;
}

void SettingsDialog::addPage(const QString& name, QWidget* page) {
    const int index = m_ui->settingsPages->addWidget(page);
    m_pageIndex[name] = index;

    auto* item = new QListWidgetItem(name);
    m_ui->settingsList->addItem(item);

    connect(m_ui->settingsList, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem*) {
        const QString name = current->text();
        Q_ASSERT(m_pageIndex.count(name));

        m_ui->settingsPages->setCurrentIndex(m_pageIndex.at(name));
    });
}

}  // namespace Ripes
