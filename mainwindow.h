#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>

using namespace cv;

namespace Ui {
class MainWindow;
}

enum State
{
    blank,
    empty,
    full
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void mousePressEvent(QMouseEvent * event);

public slots:
    void updateGraph(void);
    void start(void);
private:
    void reset();
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QGraphicsScene colorScene;
    Mat state;
    float height;
    float width;
    QPen casePen;
    QBrush blankBrush;
    QBrush emptyBrush;
    QPen colorPen;
    QBrush colorBrush;
};
#endif // MAINWINDOW_H
