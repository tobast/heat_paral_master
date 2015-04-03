#include <cstdio>
#include <vector>
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>

#include "MainWindow.h"
#include "Worker.h"

using namespace std;
typedef vector<vector<int> > Int_2d_vect;

int main(int argc, char** argv)
{
    //------------- GRID INPUT -----------------
    int size,scale;
    scanf("%d %d", &size, &scale);

    Int_2d_vect grid;
    grid.resize(size,vector<int>(size,0));

    for(int i=0; i < size; i++){
        for(int j=0; j<size; j++) {
            scanf("%d", &grid[i][j]);
        }
    }

    QApplication app(argc,argv);
    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&translator);

    MainWindow mw(size,scale);
    Worker worker(&mw, grid);
    mw.show();
    worker.triggerRender();
    worker.start();

    return app.exec();
}
