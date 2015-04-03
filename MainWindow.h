#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int size, int scale, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateImage(const std::vector<std::vector<int> >& grid);

private:
    Ui::MainWindow *ui;
    QImage img;
    int size,scale;
};

#endif // MAINWINDOW_H
