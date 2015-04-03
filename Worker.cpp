#include "Worker.h"

Worker::Worker(MainWindow *display, std::vector<std::vector<int> > &grid_, QObject *parent) :
    QObject(parent), display(display)
{
    grid.resize(grid_.size(), std::vector<int>(grid_[0].size(),0));
    isReservoir.resize(grid_.size(), std::vector<bool>(grid_[0].size(),false));
    for(size_t row=0; row < grid_.size(); row++) {
        for(size_t col=0; col < grid_[0].size(); col++) {
            grid[row][col] = grid_[row][col] % 1000;
            isReservoir[row][col] = (grid_[row][col] >= 1000);
        }
    }

    serv = new TcpServer(&grid, this);
    connect(serv, SIGNAL(iterated()), this, SLOT(triggerRender()));
}

Worker::~Worker()
{
}

void Worker::start()
{
    serv->iterate();
}

void Worker::triggerRender()
{
    display->updateImage(grid);
}
