#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <vector>

#include "MainWindow.h"
#include "TcpServer.h"

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(MainWindow* display, std::vector<std::vector<int> >& grid_, QObject *parent = 0);

    ~Worker();

signals:

public slots:
    void start();
    void triggerRender();

private:
    MainWindow* display;
    std::vector<std::vector<int> > grid;
    std::vector<std::vector<bool> > isReservoir;
    TcpServer* serv;
};

#endif // WORKER_H
