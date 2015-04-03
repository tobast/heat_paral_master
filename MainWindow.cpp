#include "MainWindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(int size, int scale, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    img(size*scale, size*scale, QImage::Format_ARGB32),
    size(size),scale(scale)
{
    ui->setupUi(this);
    img.fill(Qt::black);
    ui->imageDisplay->setPixmap(QPixmap::fromImage(img));
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateImage(const std::vector<std::vector<int> > &grid)
{
    for(size_t row=0; row < grid.size(); row++){
        for(size_t col=0; col < grid[0].size(); col++) {
            for (int i=0; i < scale; i++) {
                for(int j=0; j < scale; j++) {
                    img.setPixel(row*scale+i, col*scale+j,
                            QColor::fromHsv(240 - grid[row][col] * 2.4,0xFF,0xFF).rgb());
                }
            }
        }
    }
    ui->imageDisplay->setPixmap(QPixmap::fromImage(img));
}
