#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <iostream>

#include "mainwindow.h"
#include "parser.h"
#include "pipeline.h"

using namespace std;

int main(int argc, char** argv) {
    Q_INIT_RESOURCE(images);
    Q_INIT_RESOURCE(examples);

    QApplication app(argc, argv);
    MainWindow m;
    m.show();

    if (argc == 2) {
        // Load file specified in command-line argument
        QString ext = QFileInfo(argv[1]).suffix();
        if (ext == QString("bin")) {
            m.loadBinaryFile(argv[1]);
        } else if (ext == "asm" || ext == "s") {
            m.loadAssemblyFile(argv[1]);
        } else {
            QMessageBox msg;
            msg.setText(
                QString("Unknown extension for input file.\ngot: \"%1\" - expected:\n- bin\n- asm\n- s").arg(ext));
            msg.exec();
        }
    }

    return app.exec();
}
