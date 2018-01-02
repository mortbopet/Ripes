#include <QApplication>
#include <iostream>

#include "mainwindow.h"
#include "parser.h"
#include "pipeline.h"

#include "QDebug"

using namespace std;

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow m;
    m.show();

    if (argc == 2) {
        // Load file specified in command-line argument
        m.loadBinaryFile(argv[1]);
    }

    return app.exec();
}
