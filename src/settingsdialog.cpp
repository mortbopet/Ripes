#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "ccmanager.h"
#include "formattermanager.h"
#include "ripessettings.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpacerItem>
#include <QSpinBox>
#include <QStackedWidget>

#include "utilities/hexspinbox.h"
#include "utilities/scrolleventfilter.h"

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

template <typename T_TriggerWidget, typename T_EditWidget = T_TriggerWidget>
std::pair<QLabel*, T_TriggerWidget*> createSettingsWidgets(const QString& settingName, const QString& labelText) {
    auto* label = new QLabel(labelText);
    auto f = label->font();
    f.setBold(true);
    label->setFont(f);

    T_TriggerWidget* widget = new T_TriggerWidget();

    auto* settingObserver = RipesSettings::getObserver(settingName);

    if constexpr (std::is_same<T_EditWidget, QSpinBox>()) {
        // Ensure that the current value can be represented in the spinbox. It is expected that the spinbox range will
        // be specified after being created in this function.
        widget->setRange(INT_MIN, INT_MAX);
        widget->setValue(settingObserver->value().toUInt());
        widget->connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), settingObserver,
                        &SettingObserver::setValue);
        widget->installEventFilter(new ScrollEventFilter());
    } else if constexpr (std::is_same<T_EditWidget, HexSpinBox>()) {
        // The hex value is stored as an int, so get the int and static cast it to unsigned.
        unsigned uValue = static_cast<unsigned>(settingObserver->value().toInt());
        widget->setValue(uValue);
        widget->connect(widget, QOverload<int>::of(&QSpinBox::valueChanged), settingObserver,
                        &SettingObserver::setValue);
        widget->installEventFilter(new ScrollEventFilter());
    } else if constexpr (std::is_same<T_EditWidget, QLineEdit>()) {
        widget->connect(widget, &QLineEdit::textChanged, settingObserver, &SettingObserver::setValue);
        widget->setText(settingObserver->value().toString());
    } else if constexpr (std::is_same<T_EditWidget, QCheckBox>()) {
        widget->connect(widget, &QCheckBox::toggled, settingObserver, &SettingObserver::setValue);
        widget->setChecked(settingObserver->value().toBool());
    } else if constexpr (std::is_same<T_EditWidget, QColorDialog>()) {
        // Create a QPushButton which will trigger a QColorWidget when clicked. Changes in the color settings will
        // trigger a change in the pushbutton color
        auto colorSetterFunctor = [=] {
            QPalette pal = widget->palette();
            pal.setColor(QPalette::Button, settingObserver->value().value<QColor>());
            widget->setAutoFillBackground(true);
            widget->setPalette(pal);
        };

        // We want changes in the set color to propagate to the button, while in the dialog. But this connection must be
        // deleted once the dialog is closed, to avoid a dangling connection between the settings object and the trigger
        // widget.
        auto conn = settingObserver->connect(settingObserver, &SettingObserver::modified, colorSetterFunctor);
        widget->connect(widget, &QObject::destroyed, settingObserver, [=] { settingObserver->disconnect(conn); });

        widget->connect(widget, &QPushButton::clicked, settingObserver, [=](bool) {
            QColorDialog diag;
            diag.setCurrentColor(settingObserver->value().value<QColor>());
            if (diag.exec()) {
                settingObserver->setValue(diag.selectedColor());
            }
        });

        // Apply color of current setting
        colorSetterFunctor();
    } else if constexpr (std::is_same<T_EditWidget, QFontDialog>()) {
        // Create a QPushButton which will trigger a QFontDialog when clicked. Changes in the font settings will
        // trigger a change in the pushbutton text
        auto fontSetterFunctor = [=] {
            const auto& font = settingObserver->value().value<QFont>();
            const QString text = font.family() + " | " + QString::number(font.pointSize());
            widget->setText(text);
        };

        // We want changes in the set font to propagate to the button, while in the dialog. But this connection must be
        // deleted once the dialog is closed, to avoid a dangling connection between the settings object and the trigger
        // widget.
        auto conn = settingObserver->connect(settingObserver, &SettingObserver::modified, widget, fontSetterFunctor);
        widget->connect(widget, &QObject::destroyed, settingObserver, [=] { settingObserver->disconnect(conn); });

        widget->connect(widget, &QPushButton::clicked, settingObserver, [=](bool) {
            QFontDialog diag;
            diag.setCurrentFont(settingObserver->value().value<QFont>());
            diag.setOption(QFontDialog::MonospacedFonts, true);
            if (diag.exec()) {
                settingObserver->setValue(diag.selectedFont());
            }
        });

        // Apply font of current setting
        fontSetterFunctor();
    }

    return {label, widget};
}

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), m_ui(new Ui::SettingsDialog) {
    m_ui->setupUi(this);
    m_ui->settingsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    setWindowTitle("Settings");

    // Create settings pages
    addPage("Environment", createEnvironmentPage());
    addPage("Simulator", createSimulatorPage());
    addPage("Compiler", createCompilerPage());
    addPage("Editor", createEditorPage());

    m_ui->settingsList->setCurrentRow(RipesSettings::value(RIPES_SETTING_SETTING_TAB).toInt());
}

SettingsDialog::~SettingsDialog() {
    delete m_ui;
}

void SettingsDialog::accept() {
    QDialog::accept();
}

QWidget* SettingsDialog::createCompilerPage() {
    auto [pageWidget, pageLayout] = constructPage();

    /***********************************************************************
     * Compiler settings
     **********************************************************************/

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
        dialog.setOption(QFileDialog::DontUseNativeDialog);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec()) {
            ccpath->setText(dialog.selectedFiles().at(0));
        }
    });

    // Make changes in the CC path trigger revalidation in the CCManager
    connect(ccpath, &QLineEdit::textChanged, &CCManager::get(), &CCManager::trySetCC);

    // Make CCManager compiler changes trigger background color of the ccpath, indicating whether the CC was determined
    // to be valid
    connect(&CCManager::get(), &CCManager::ccChanged, this, &SettingsDialog::CCPathChanged);

    CCHLayout->addWidget(pathBrowseButton);

    // Add compiler arguments widget
    auto [ccArgLabel, ccArgs] = createSettingsWidgets<QLineEdit>(RIPES_SETTING_CCARGS, "Compiler arguments:");
    auto* CCArgHLayout = new QHBoxLayout();
    CCArgHLayout->addWidget(ccArgLabel);
    CCArgHLayout->addWidget(ccArgs);
    CCLayout->addLayout(CCArgHLayout);
    // Make changes in argument reemit ccpath text changed. By doing so, we revalidate the compiler once new arguments
    // have been added, implicitely validating the arguments along with it.
    connect(ccArgs, &QLineEdit::textChanged, [=, ccpath = ccpath] { emit ccpath->textChanged(ccpath->text()); });

    // Add linker arguments widget
    auto [ldArgLabel, ldArgs] = createSettingsWidgets<QLineEdit>(RIPES_SETTING_LDARGS, "Linker arguments:");
    auto* LDArgHLayout = new QHBoxLayout();
    LDArgHLayout->addWidget(ldArgLabel);
    LDArgHLayout->addWidget(ldArgs);
    CCLayout->addLayout(LDArgHLayout);
    connect(ldArgs, &QLineEdit::textChanged, [=, ccpath = ccpath] { emit ccpath->textChanged(ccpath->text()); });

    // Add effective compile command line view
    auto* CCCLineHLayout = new QHBoxLayout();
    m_compileInfoHeader = new QLabel();
    auto f = m_compileInfoHeader->font();
    f.setBold(true);
    m_compileInfoHeader->setFont(f);
    CCCLineHLayout->addWidget(m_compileInfoHeader);
    m_compileInfo = new QPlainTextEdit();
    m_compileInfo->setReadOnly(true);
    m_compileInfo->setWordWrapMode(QTextOption::WordWrap);
    CCCLineHLayout->addWidget(m_compileInfo);
    CCLayout->addLayout(CCCLineHLayout);

    pageLayout->addWidget(CCGroupBox);

    // Trigger initial rehighlighting
    CCManager::get().trySetCC(m_ccpath->text());

    /***********************************************************************
     * Assembler settings
     **********************************************************************/

    auto* ASMGroupBox = new QGroupBox("Assembler");
    auto* ASMLayout = new QGridLayout();
    ASMGroupBox->setLayout(ASMLayout);

    appendToLayout(createSettingsWidgets<HexSpinBox>(RIPES_SETTING_ASSEMBLER_TEXTSTART, ".text section start address:"),
                   ASMLayout);
    appendToLayout(createSettingsWidgets<HexSpinBox>(RIPES_SETTING_ASSEMBLER_DATASTART, ".data section start address:"),
                   ASMLayout);
    appendToLayout(createSettingsWidgets<HexSpinBox>(RIPES_SETTING_ASSEMBLER_BSSSTART, ".bss section start address:"),
                   ASMLayout);

    pageLayout->addWidget(ASMGroupBox);

    return pageWidget;
}

void SettingsDialog::CCPathChanged(CCManager::CCRes res) {
    QPalette palette = this->palette();
    if (res.success) {
        palette.setColor(QPalette::Base, QColorConstants::Green.lighter());
        m_compileInfoHeader->setText("Compile command:");
    } else {
        palette.setColor(QPalette::Base, QColorConstants::Red.lighter());
        m_compileInfoHeader->setText("Error:");
    }
    m_ccpath->setPalette(palette);

    // Update compile command preview
    auto cc = CCManager::get().createCompileCommand({"${input}"}, "${output}");
    if (m_compileInfo) {
        if (res.success) {
            m_compileInfo->setPlainText(cc.toString());
        } else {
            m_compileInfo->setPlainText(res.errorOutput.toString(res.cc));
        }
    }
}

QWidget* SettingsDialog::createSimulatorPage() {
    auto [pageWidget, pageLayout] = constructPage();

    // Setting: RIPES_SETTING_REWINDSTACKSIZE
    auto [rewindLabel, rewindSpinbox] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_REWINDSTACKSIZE, "Max. undo cycles:");
    rewindSpinbox->setRange(0, INT_MAX);
    appendToLayout({rewindLabel, rewindSpinbox}, pageLayout, "Maximum cycles that the simulator is able to undo.");

    appendToLayout(createSettingsWidgets<HexSpinBox>(RIPES_SETTING_PERIPHERALS_START, "I/O start address:"), pageLayout,
                   "Start address in the address space where peripherals will be allocated from, growing upwards");

    return pageWidget;
}

QWidget* SettingsDialog::createEditorPage() {
    auto [pageWidget, pageLayout] = constructPage();

    auto [indentLabel, indentSpinbox] = createSettingsWidgets<QSpinBox>(RIPES_SETTING_INDENTAMT, "Indent size:");
    indentSpinbox->setRange(0, 100);
    appendToLayout({indentLabel, indentSpinbox}, pageLayout, "Indent size (in spaces) of tab characters.");

    auto [regsLabel, regsCheckbox] =
        createSettingsWidgets<QCheckBox>(RIPES_SETTING_EDITORREGS, "Show registers in editor tab");
    appendToLayout({regsLabel, regsCheckbox}, pageLayout,
                   "Show (or hide) a view of the processor register file(s) in the editor tab.");

    auto [consoleLabel, consoleCheckbox] =
        createSettingsWidgets<QCheckBox>(RIPES_SETTING_EDITORCONSOLE, "Show console in editor tab");
    appendToLayout({consoleLabel, consoleCheckbox}, pageLayout,
                   "Show (or hide) a view of the console in the editor tab.");

    auto [editorStageHighlightingLabel, editorStageHighlightingCheckbox] =
        createSettingsWidgets<QCheckBox>(RIPES_SETTING_EDITORSTAGEHIGHLIGHTING, "Highlight stages in source");
    appendToLayout({editorStageHighlightingLabel, editorStageHighlightingCheckbox}, pageLayout,
                   "Show (or hide) highlighting of processor stages in the program source code.");

    // ===== Source formatter
    auto* formatterGroupBox = new QGroupBox("Formatter");
    appendToLayout(formatterGroupBox, pageLayout);
    auto* formatterLayout = new QGridLayout();
    auto* formatterDesc = new QLabel("Format .c files using clang-format, if available.");
    formatterDesc->setWordWrap(true);
    appendToLayout(formatterDesc, formatterLayout);
    formatterGroupBox->setLayout(formatterLayout);

    // Formatter path
    auto* FormatterPathLayout = new QHBoxLayout();
    auto [formatterlabel, formatterpath] =
        createSettingsWidgets<QLineEdit>(RIPES_SETTING_FORMATTER_PATH, "clang-format path:");
    appendToLayout(FormatterPathLayout, formatterLayout);
    FormatterPathLayout->addWidget(formatterlabel);
    FormatterPathLayout->addWidget(formatterpath);
    auto* pathBrowseButton = new QPushButton("Browse");
    FormatterPathLayout->addWidget(pathBrowseButton);
    connect(pathBrowseButton, &QPushButton::clicked, formatterpath, [=, formatterpath = formatterpath] {
        QFileDialog dialog(this);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec()) {
            formatterpath->setText(dialog.selectedFiles().at(0));
        }
    });

    // Format on save
    appendToLayout(createSettingsWidgets<QCheckBox>(RIPES_SETTING_FORMAT_ON_SAVE, "Format on save:"), formatterLayout);

    // Formatter arguments
    appendToLayout(createSettingsWidgets<QLineEdit>(RIPES_SETTING_FORMATTER_ARGS, "Formatter arguments:"),
                   formatterLayout);

    return pageWidget;
}

QWidget* SettingsDialog::createEnvironmentPage() {
    auto [pageWidget, pageLayout] = constructPage();

    auto [uiUpdateMsLabel, uiUpdateMsSb] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_UIUPDATEPS, "Max. UI updates per second");
    uiUpdateMsSb->setMinimum(1);
    uiUpdateMsSb->setMaximum(100);
    appendToLayout({uiUpdateMsLabel, uiUpdateMsSb}, pageLayout,
                   "Maximum updates of UI elements per second. Increasing this may reduce performance.");

    auto [maxcyclesLabel, maxcyclesSb] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_CACHE_MAXCYCLES, "Max. cache plot cycles");
    maxcyclesSb->setMinimum(0);
    maxcyclesSb->setMaximum(INT_MAX);
    appendToLayout({maxcyclesLabel, maxcyclesSb}, pageLayout,
                   "Maximum number of cache cycles to plot. Increasing this may incur substantial slowdown for long "
                   "time executing programs, given the number of points to be plotted.");

    auto [maxPointsLabel, maxPointsSb] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_CACHE_MAXPOINTS, "Min. cache plot points:");
    maxPointsSb->setMinimum(2);
    maxPointsSb->setMaximum(INT_MAX);
    appendToLayout(
        {maxPointsLabel, maxPointsSb}, pageLayout,
        "Minimum number of points to be kept in the cache plot. Once 2x this amount is reached, the cache "
        "plot will be resampled to this minimum value, and new points will be added based on the sampling "
        "rate after the resampling. This resampling allows for real-time plotting regardless of the number of "
        "simulation cycles.");

    auto [maxPipeDiagCycLabel, maxPipeDiagCycSb] =
        createSettingsWidgets<QSpinBox>(RIPES_SETTING_PIPEDIAGRAM_MAXCYCLES, "Max. pipeline diagram cycles:");
    maxPipeDiagCycSb->setMinimum(0);
    maxPipeDiagCycSb->setMaximum(INT_MAX);
    appendToLayout({maxPipeDiagCycLabel, maxPipeDiagCycSb}, pageLayout,
                   "Maximum number of cycles to be recorded in the pipeline diagram.");

    // Console settings
    auto* consoleGroupBox = new QGroupBox("Console");
    auto* consoleLayout = new QGridLayout();
    consoleGroupBox->setLayout(consoleLayout);

    appendToLayout(createSettingsWidgets<QCheckBox>(RIPES_SETTING_CONSOLEECHO, "Echo console input:"), consoleLayout);
    appendToLayout(createSettingsWidgets<QPushButton, QFontDialog>(RIPES_SETTING_CONSOLEFONT, "Console font:"),
                   consoleLayout);
    appendToLayout(
        createSettingsWidgets<QPushButton, QColorDialog>(RIPES_SETTING_CONSOLEFONTCOLOR, "Console font color:"),
        consoleLayout);
    appendToLayout(
        createSettingsWidgets<QPushButton, QColorDialog>(RIPES_SETTING_CONSOLEBG, "Console background color:"),
        consoleLayout);
    appendToLayout(consoleGroupBox, pageLayout);

    return pageWidget;
}

void SettingsDialog::appendToLayout(QLayout* layout, QGridLayout* pageLayout, int colSpan) {
    pageLayout->addLayout(layout, pageLayout->rowCount(), 0, 1, colSpan);
}

void SettingsDialog::appendToLayout(QWidget* widget, QGridLayout* pageLayout, int colSpan) {
    pageLayout->addWidget(widget, pageLayout->rowCount(), 0, 1, colSpan);
}

void SettingsDialog::appendToLayout(std::pair<QLabel*, QWidget*> settingsWidgets, QGridLayout* pageLayout,
                                    const QString& description) {
    const unsigned row = pageLayout->rowCount();
    pageLayout->addWidget(settingsWidgets.first, row, 0);
    pageLayout->addWidget(settingsWidgets.second, row, 1);
    if (!description.isEmpty()) {
        auto* desc = new QLabel(description, this);
        auto f = desc->font();
        f.setPointSize(f.pointSize() * 0.9);
        desc->setFont(f);
        desc->setWordWrap(true);
        pageLayout->addWidget(desc, row + 1, 0);
    }
}

void SettingsDialog::addPage(const QString& name, QWidget* page) {
    auto* scrollArea = new QScrollArea();
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(page);
    const int index = m_ui->settingsPages->addWidget(scrollArea);
    m_pageIndex[name] = index;

    auto* item = new QListWidgetItem(name);
    m_ui->settingsList->addItem(item);

    connect(m_ui->settingsList, &QListWidget::currentItemChanged, [=](QListWidgetItem* current, QListWidgetItem*) {
        const QString _name = current->text();
        Q_ASSERT(m_pageIndex.count(_name));

        const int idx = m_pageIndex.at(_name);
        m_ui->settingsPages->setCurrentIndex(idx);
        RipesSettings::setValue(RIPES_SETTING_SETTING_TAB, idx);
    });
}

}  // namespace Ripes
