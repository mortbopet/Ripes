#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QResource>
#include <QTimer>
#include <iostream>

#include "src/mainwindow.h"

using namespace std;

int main(int argc, char** argv) {
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(examples);
    Q_INIT_RESOURCE(layouts);
    Q_INIT_RESOURCE(fonts);

    QApplication app(argc, argv);
    Ripes::MainWindow m;

    // The following sequence of events manages to successfully start the application as maximized, with the processor
    // at a reasonable size. This has been found to be specially a problem on windows.
    m.resize(800, 600);
    m.showMaximized();
    m.setWindowState(Qt::WindowMaximized);
    QTimer::singleShot(100, [&m] { m.fitToView(); });

    return app.exec();
}
