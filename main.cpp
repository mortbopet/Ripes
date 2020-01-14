#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QResource>
#include <iostream>

#include "src/mainwindow.h"
#include "src/parser.h"

using namespace std;

int main(int argc, char** argv) {
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(examples);

    QApplication app(argc, argv);
    Ripes::MainWindow m;
    m.show();
    return app.exec();
}
