#include <QApplication>
#include <iostream>

#include "mainwindow.h"
#include "parser.h"
#include "runner.h"

#include "QDebug"

using namespace std;

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow m;
    m.show();

    // execute runner
    return app.exec();
}
