#include <QApplication>
#include <iostream>

#include "mainwindow.h"
#include "parser.h"
#include "runner.h"

#include "QDebug"

using namespace std;

int main(int argc, char** argv) {
    if (argc != 2) {
        // insert error msg
        return 1;
    }

    /*
    Parser parser;
    // For now, initialize parser with input argument
    if (parser.init(argv[1])) {
        return 1;
    }
*/

    QApplication app(argc, argv);
    MainWindow m;
    m.show();

    // execute runner
    return app.exec();
}
