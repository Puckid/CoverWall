#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QDir>
#include <ctime>
#include <QDesktopServices>

#define BORDER  3L      // % cropped out
#define RADIUS  0.2     // ratio square radius radiant

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    srand(time(NULL));
    ui->setupUi(this);

    state.create(ui->heightSpin->value(), ui->widthSpin->value(), CV_8UC1);
    cout<<"state="<<endl<<state<<endl<<endl;
    state.setTo(empty);
    cout<<"state="<<endl<<state<<endl<<endl;

    casePen.setWidth(2);
    casePen.setColor(QColor(0,0,0));

    colorPen.setStyle(Qt::NoPen);
    colorBrush.setStyle(Qt::SolidPattern);
    colorBrush.setColor(QColor(ui->RSpin->value(), ui->GSpin->value(), ui->BSpin->value()));

    blankBrush.setStyle(Qt::SolidPattern);
    blankBrush.setColor(QColor(30, 30, 30));

    emptyBrush.setStyle(Qt::SolidPattern);
    emptyBrush.setColor(QColor(240, 240, 240));

    this->updateGraph();

    connect(ui->heightSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->widthSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->pixelsSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->bigSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->nBigSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->RSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->GSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));
    connect(ui->BSpin, SIGNAL(valueChanged(int)),
            this, SLOT(updateGraph()));

    connect(ui->startButton, SIGNAL(clicked(bool)),
            this, SLOT(start()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateGraph()
{
    int heightSpin = ui->heightSpin->value();
    int widthSpin = ui->widthSpin->value();
    height = (float)(ui->graphicsView->height()-10)/heightSpin;
    width = (float)(ui->graphicsView->width()-10)/widthSpin;
    int i, j;

    Mat temp = state.clone();
    state.create(heightSpin, widthSpin, CV_8UC1);
    state.setTo(empty);
    for(i=0; i< (state.cols < temp.cols ? state.cols : temp.cols); i++)
    {
        for(j=0; j< (state.rows < temp.rows ? state.rows : temp.rows); j++)
            state.at<char>(j, i) = temp.at<char>(j, i);
    }

    int imgw, imgh;
    imgw = ui->widthSpin->value()*ui->pixelsSpin->value();
    imgh = ui->heightSpin->value()*ui->pixelsSpin->value();
    ui->imgwLabel->setText(QString::number(imgw));
    ui->imghLabel->setText(QString::number(imgh));

    ui->msgLabel->setText("Ready to go!");

    cout<<"new size state="<<endl<<state<<endl<<endl;

    this->scene.clear();
    for(i=0; i<widthSpin; i++)
    {
        for(j=0; j<heightSpin; j++)
        {
            switch(state.at<char>(j, i))
            {
            case empty:
                this->scene.addRect(i*width, j*height, width, height, casePen, emptyBrush);
                break;
            case blank:
                this->scene.addRect(i*width, j*height, width, height, casePen, blankBrush);
                break;
            }
        }
    }
    ui->graphicsView->setScene(&this->scene);

    this->colorScene.clear();
    colorBrush.setColor(QColor(ui->RSpin->value(), ui->GSpin->value(), ui->BSpin->value()));
    this->colorScene.addRect(0,0,ui->colorView->width()-2,ui->colorView->height()-2,colorPen, colorBrush);
    ui->colorView->setScene(&this->colorScene);
    return;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    int boxx = floor((float)(event->x()-ui->graphicsView->x()-5)/width);
    int boxy = floor((float)(event->y()-ui->graphicsView->y()-5)/height);
    qDebug()<<"box:"<<boxx<<"; "<<boxy<<endl;

    if(event->button() == Qt::LeftButton)
    {
        if(boxx >= 0 && boxx < state.cols && boxy >= 0 && boxy < state.rows)
        {
            state.at<char>(boxy, boxx) = blank;
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(boxx >= 0 && boxx < state.cols && boxy >= 0 && boxy < state.rows)
        {
            state.at<char>(boxy, boxx) = empty;
        }
    }

    updateGraph();
}

void MainWindow::start()
{
    QStringList nameFilter("*.png");
    nameFilter<<"*.jpg";
    QDir dir(QUrl("./img").toString());
    QDir dirBest(QUrl("./img/best").toString());
    QStringList bestFiles = dirBest.entryList(nameFilter);
    QStringList imgFiles = dir.entryList(nameFilter);
    //qDebug()<<"file list: "<<imgFiles<<endl;
    //qDebug()<<"first file path: "<<dir.filePath(imgFiles[0])<<endl;
    int pix = ui->pixelsSpin->value();
    Mat output(ui->heightSpin->value()*pix,
               ui->widthSpin->value()*pix,
               CV_8UC3);
    Point3_<ushort> initVal;
    initVal.x = ui->BSpin->value();
    initVal.y = ui->GSpin->value();
    initVal.z = ui->RSpin->value();
    output.setTo(Scalar(initVal.x, initVal.y, initVal.z));

    for(int i = ui->bigSpin->value(); i>0; i--)
    {
        // search if enough room in current matrix
        // to loop on all i x i mat possible.
        Mat toFind(i, i, CV_8UC1);
        Mat submat(i, i, CV_8UC1);
        toFind.setTo(empty);
        QList<QPoint> pos;
        // Scan matrix
        for(int ii = 0; ii<=ui->widthSpin->value()-i; ii++)
        {
            for(int ij = 0; ij<=ui->heightSpin->value()-i; ij++)
            {
                submat = state(Rect(ii, ij, i, i));
                Mat diff = toFind != submat;
                if(countNonZero(diff) == 0)
                    pos.append(QPoint(ii, ij));
            }
        }

        for(int j = 0; j<ui->nBigSpin->value(); j++)
        {     
            if(pos.isEmpty())
                break;

            QString chosenOne;
            if(!bestFiles.isEmpty())
            {
                int random = rand()%bestFiles.size();
                chosenOne = dirBest.filePath(bestFiles.at(random));
                bestFiles.removeAt(random);
            }
            else if(!imgFiles.isEmpty())
            {
                int random = rand()%imgFiles.size();
                chosenOne = dir.filePath(imgFiles.at(random));
                imgFiles.removeAt(random);
            }
            else
            {
                ui->msgLabel->setText("Error: no more\nimg");
                reset();
                return;
            }

            // load image and crop
            Mat chosenImg = imread(chosenOne.toStdString());
            long side = chosenImg.cols < chosenImg.rows ? chosenImg.cols : chosenImg.rows;
            side = ((100L-BORDER)*side)/100L;
            int X = (chosenImg.cols-side)/2;
            int Y = (chosenImg.rows-side)/2;
            Mat croppedImg = chosenImg(Rect(X, Y, side, side));

            // chose random position in output matrix
            int randomPos = rand()%pos.size();
            Mat subOutput = output(Rect(pos.at(randomPos).x()*pix,
                                        pos.at(randomPos).y()*pix,
                                        i*pix, i*pix));
            // place the resized image in the right place
            if(croppedImg.cols > subOutput.cols)
                cv::resize(croppedImg, subOutput, subOutput.size(), 0, 0, CV_INTER_AREA);
            else
                cv::resize(croppedImg, subOutput, subOutput.size(), 0, 0, CV_INTER_CUBIC);
            state(Rect(pos.at(randomPos).x(), pos.at(randomPos).y(), i, i)).setTo(full);
            // do until mat is filled with 1x1 pics
            if(i == 1)
            {
                j = -1;
                pos.removeAt(randomPos);
            }
            else
            {
                // remove positions occupied by last pic
                int x = pos.at(randomPos).x();
                int y = pos.at(randomPos).y();
                for(int k = pos.size()-1; k>=0; k--)
                {
                    if((pos.at(k).x()<x+i) && (pos.at(k).x()>x-i) && (pos.at(k).y()<y+i) && (pos.at(k).y()>y-i))
                        pos.removeAt(k);
                }
            }
        }
    }

    //  add gradient
    // first look for corners
    // 1
    float radius = RADIUS*(float)pix;
    Mat matCorner(2,2,CV_8UC1);
    matCorner.at<char>(0,0) = full;
    matCorner.at<char>(0,1) = full;
    matCorner.at<char>(1,0) = full;
    matCorner.at<char>(1,1) = blank;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i, j, 2, 2));
            Mat diff = matCorner != submat;
            if(countNonZero(diff) == 0)
            {
                int cornerx = (i+1)*pix;
                int cornery = (j+1)*pix;
                for(int k = i*pix; k<(i+1)*pix; k++)
                {
                    for(int l = j*pix; l<(j+1)*pix; l++)
                    {
                        float dist = sqrt((k-cornerx)*(k-cornerx)+(l-cornery)*(l-cornery));
                        if(dist < radius)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            float ratio = dist/radius;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // 2
    matCorner = Mat(2,2,CV_8UC1);
    matCorner.at<char>(0,0) = full;
    matCorner.at<char>(0,1) = full;
    matCorner.at<char>(1,0) = blank;
    matCorner.at<char>(1,1) = full;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i, j, 2, 2));
            Mat diff = matCorner != submat;
            if(countNonZero(diff) == 0)
            {
                int cornerx = (i+1)*pix;
                int cornery = (j+1)*pix;
                for(int k = (i+1)*pix; k<(i+2)*pix; k++)
                {
                    for(int l = j*pix; l<(j+1)*pix; l++)
                    {
                        float dist = sqrt((k-cornerx)*(k-cornerx)+(l-cornery)*(l-cornery));
                        if(dist < radius)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            float ratio = dist/radius;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // 3
    matCorner = Mat(2,2,CV_8UC1);
    matCorner.at<char>(0,0) = full;
    matCorner.at<char>(0,1) = blank;
    matCorner.at<char>(1,0) = full;
    matCorner.at<char>(1,1) = full;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i, j, 2, 2));
            Mat diff = matCorner != submat;
            if(countNonZero(diff) == 0)
            {
                int cornerx = (i+1)*pix;
                int cornery = (j+1)*pix;
                for(int k = i*pix; k<(i+1)*pix; k++)
                {
                    for(int l = (j+1)*pix; l<(j+2)*pix; l++)
                    {
                        float dist = sqrt((k-cornerx)*(k-cornerx)+(l-cornery)*(l-cornery));
                        if(dist < radius)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            float ratio = dist/radius;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // 4
    matCorner = Mat(2,2,CV_8UC1);
    matCorner.at<char>(0,0) = blank;
    matCorner.at<char>(0,1) = full;
    matCorner.at<char>(1,0) = full;
    matCorner.at<char>(1,1) = full;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i, j, 2, 2));
            Mat diff = matCorner != submat;
            if(countNonZero(diff) == 0)
            {
                int cornerx = (i+1)*pix;
                int cornery = (j+1)*pix;
                for(int k = (i+1)*pix; k<(i+2)*pix; k++)
                {
                    for(int l = (j+1)*pix; l<(j+2)*pix; l++)
                    {
                        float dist = sqrt((k-cornerx)*(k-cornerx)+(l-cornery)*(l-cornery));
                        if(dist < radius)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            float ratio = dist/radius;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }


    // then look for arretes
    // top
    matCorner = Mat(2,1,CV_8UC1);
    matCorner.at<char>(0,0) = full;
    matCorner.at<char>(0,1) = blank;
    for(int i = 0; i<state.cols; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i,j,1,2));
            Mat diff = matCorner != submat;
            int cornery = (j+1)*pix;
            if(countNonZero(diff) == 0)
            {
                for(int l = j*pix; l<(j+1)*pix; l++)
                {
                    float dist = cornery-l;
                    float ratio = dist/radius;
                    if(dist < radius)
                    {
                        for(int k = i*pix; k<(i+1)*pix; k++)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // down
    matCorner = Mat(2,1,CV_8UC1);
    matCorner.at<char>(0,0) = blank;
    matCorner.at<char>(0,1) = full;
    for(int i = 0; i<state.cols; i++)
    {
        for(int j= 0; j<state.rows-1; j++)
        {
            Mat submat = state(Rect(i,j,1,2));
            Mat diff = matCorner != submat;
            int cornery = (j+1)*pix;
            if(countNonZero(diff) == 0)
            {
                for(int l = (j+1)*pix; l<(j+2)*pix; l++)
                {
                    float dist = l-cornery;
                    float ratio = dist/radius;
                    if(dist < radius)
                    {
                        for(int k = i*pix; k<(i+1)*pix; k++)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // right        NOT WOKRING
    matCorner = Mat(1,2,CV_8UC1);
    matCorner.at<char>(0,0) = blank;
    matCorner.at<char>(1,0) = full;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows; j++)
        {
            Mat submat = state(Rect(i,j,2,1));
            Mat diff = matCorner != submat;
            int cornerx = (i+1)*pix;
            if(countNonZero(diff) == 0)
            {
                for(int k = (i+1)*pix; k<(i+2)*pix; k++)
                {
                    float dist = k-cornerx;
                    float ratio = dist/radius;
                    if(dist < radius)
                    {
                        for(int l = j*pix; l<(j+1)*pix; l++)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }
    // left             NOT WORKING
    matCorner = Mat(1,2,CV_8UC1);
    matCorner.at<char>(0,0) = full;
    matCorner.at<char>(1,0) = blank;
    for(int i = 0; i<state.cols-1; i++)
    {
        for(int j= 0; j<state.rows; j++)
        {
            Mat submat = state(Rect(i,j,2,1));
            Mat diff = matCorner != submat;
            int cornerx = (i+1)*pix;
            if(countNonZero(diff) == 0)
            {
                for(int k = i*pix; k<(i+1)*pix; k++)
                {
                    float dist = cornerx-k;
                    float ratio = dist/radius;
                    if(dist < radius)
                    {
                        for(int l = j*pix; l<(j+1)*pix; l++)
                        {
                            Vec3b p = output.at<Vec3b>(l, k);
                            Vec3b d;
                            d[0] = ratio*p[0] + (1-ratio)*ui->BSpin->value();
                            d[1] = ratio*p[1] + (1-ratio)*ui->GSpin->value();
                            d[2] = ratio*p[2] + (1-ratio)*ui->RSpin->value();
                            output.at<Vec3b>(l, k) = d;
                        }
                    }
                }
            }
        }
    }


    vector<int> params;
    params.push_back(IMWRITE_PNG_COMPRESSION);
    params.push_back(0);
    time_t t = time(NULL);
    struct tm * now = localtime(&t);
    QString filename = QString("./coverWall%1x%2_%3-%4-%5_%6.png").arg(ui->imgwLabel->text(),
                                                                    ui->imghLabel->text(),
                                                                    QString::number(now->tm_year+1900),
                                                                    QString::number(now->tm_mon+1),
                                                                    QString::number(now->tm_mday),
                                                                    QString::number(t));
    imwrite(filename.toStdString(), output, params);

    cout<<"time="<<time(NULL)<<endl;
    ui->msgLabel->setText("Success!");

    QDesktopServices::openUrl(QUrl(filename));
    reset();
}

void MainWindow::reset()
{
    for(int i = 0; i<state.cols; i++)
        for(int j = 0; j<state.rows; j++)
        {
            if(state.at<char>(j,i) == full)
                state.at<char>(j,i) = empty;
        }
}
