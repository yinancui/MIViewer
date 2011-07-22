#include "miviewer.h"
#include "ui_miviewer.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QPrintDialog>
#include <QPainter>
#include <QLatin1String>
#include <QDirIterator>
#include <iostream>
#include <QSize>
#include <QDesktopWidget>
#include <QTransform>
#include <QScrollArea>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QCheckBox>
//

/********************************************* ctor & dtor ****************************************************/
MIViewer::MIViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MIViewer), workingDir(NULL)
{
    ui->setupUi(this);
    position = 0;
    scaleFactor = 1;
    std::cout << "workingDir initialized to: " << workingDir.dirName().toStdString() << std::endl;

    ui->statusBar->showMessage("Please select a directory.");

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    this->scrollArea = new QScrollArea;
    this->scrollArea->setBackgroundRole(QPalette::Dark);
    this->scrollArea->setWidget(this->imageLabel);
    this->setCentralWidget(scrollArea);
    this->scrollArea->setAlignment(Qt::AlignCenter);    // setting the qlabel to the center, finally!

    //this->resize(800, 600);

    setGeometry((int)(QApplication::desktop()->width() -  // set main window position and size in proportion to the desktop size
    (QApplication::desktop()->width() -
      (QApplication::desktop()->width() / 2)) * 1.5) / 2,
      (int)(QApplication::desktop()->height() -
    (QApplication::desktop()->height() -
      (QApplication::desktop()->height() / 2)) * 1.5) / 2,
      (int)((QApplication::desktop()->width() -
    (QApplication::desktop()->width() / 2)) * 1.5),
      (int)((QApplication::desktop()->height() -
    (QApplication::desktop()->height() / 2)) * 1.5));

    /*********** connecting the gobackform widget ************/
    goBackForm = new GoBackForm;
    QObject::connect(this->goBackForm->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listWidget_itemDoubleClicked(QListWidgetItem*)));
    /********************************************/

    MIViewer::setupStatusBar();
}

MIViewer::~MIViewer()
{
    delete ui;
}
/************************************************************************************************************/

/********************************************* File menu ****************************************************/
void MIViewer::on_actionOpen_triggered()
{
    //do open job
    if (this->getDir()) {
        QDir oldDir(workingDir);
        std::cout << "dir selected nowwwwwwwwwwwwwwwwww.\n";
        if (oldDir.dirName().toStdString() == (".")){
            std::cout << "oldDir should be empty and is: " << oldDir.dirName().toStdString() << std::endl;
            std::cout << "dir selected from the beginning.\n";
            this->createItemList();
        }
        if (oldDir.dirName().toStdString() == workingDir.dirName().toStdString()) {
            std::cout << "reselected the present dir.\n";
            std::cout << "oldDir is: " << oldDir.dirName().toStdString() << std::endl;
            std::cout << "workingDir is: " << workingDir.dirName().toStdString() << std::endl;
            this->on_actionGo_Back_triggered();
        }
        this->createItemList();
    }
    return;
}


void MIViewer::on_actionPrint_triggered()
{
    if (fileName.isEmpty())     // in case no image is present
      return;

    Q_ASSERT(this->imageLabel->pixmap());
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = this->imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(this->imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *this->imageLabel->pixmap());
    }
}

void MIViewer::on_actionQuit_triggered()
{
    this->close();
}

/****************************************************************************************************/

/************************************** View menu ************************************************************/

/************************************** navigation ************************************************************/

void MIViewer::on_actionPrevious_triggered()
{
    if (fileName.isEmpty())     // in case no image is present
        return;

    scaleFactor = 1;
    position = (position + fileInfoList.count() - 1) % fileInfoList.count();
    fileName = fileInfoList.at(position).absoluteFilePath();
    currImage.load(fileName);
    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));
    fitImg();
    statusUpdate();
    printInfo();
}

void MIViewer::on_actionNext_triggered()
{
    if (fileName.isEmpty())     // in case no image is present
        return;

    scaleFactor = 1;
    position = (position + 1) % fileInfoList.count();
    fileName = fileInfoList.at(position).absoluteFilePath();
    currImage.load(fileName);
    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));
    fitImg();
    statusUpdate();
    printInfo();
}


void MIViewer::on_actionGo_Back_triggered()
{
    if (fileInfoList.isEmpty())
        return;
    std::cout << "From going back: position before going back is: " << position << std::endl;
    this->goBackForm->show();

}
/***********************************************************************************************************/

/************************************** zooming ************************************************************/

void MIViewer::on_actionZoom_In_triggered()
{
      if (fileName.isEmpty())     // in case no image is present
        return;
      this->scrollArea->setWidgetResizable(false);
      scaleFactor *= 1.2;
      this->imageLabel->resize(scaleFactor * this->imageLabel->pixmap()->size());
      std::cout << "zoomed in.\n";

      adjustScrollBar(this->scrollArea->horizontalScrollBar(), 1.2);
      adjustScrollBar(this->scrollArea->verticalScrollBar(), 1.2);
      printInfo();
}

void MIViewer::on_actionZoom_Out_triggered()
{
      if (fileName.isEmpty())     // in case no image is present
      return;
      this->scrollArea->setWidgetResizable(false);
      scaleFactor *= 0.8;
      this->imageLabel->resize(scaleFactor * this->imageLabel->pixmap()->size());
      std::cout << "zoomed out.\n";

      adjustScrollBar(this->scrollArea->horizontalScrollBar(), 0.8);
      adjustScrollBar(this->scrollArea->verticalScrollBar(), 0.8);
      printInfo();
}


void MIViewer::on_actionNormal_triggered()
{
    if (fileName.isEmpty())     // in case no image is present
        return;

    this->scrollArea->setWidgetResizable(false);
    scaleFactor = 1;
    this->imageLabel->resize(scaleFactor * this->imageLabel->pixmap()->size());
    std::cout << "showing real size.\n";

    adjustScrollBar(this->scrollArea->horizontalScrollBar(), 1);
    adjustScrollBar(this->scrollArea->verticalScrollBar(), 1);
    printInfo();
}


void MIViewer::on_actionFit_to_Window_triggered()
{
    if (fileName.isEmpty())     // in case no image is present
      return;
    fitImg();
}



void MIViewer::on_actionRotate_Left_triggered()     // rotate the image to left by 90 degrees
{

    if (currImage.isNull()) {       // if loading failed, interrupt
        //QMessageBox::information(this, tr("Image Viewer"), tr("No image available."));
        return;
    }
    QTransform myTransform;
    myTransform.rotate(-90);
    currImage = currImage.transformed(myTransform);

    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));   // display the image
    fitImg();

    statusUpdate();
    printInfo();
}

void MIViewer::on_actionRotate_Right_triggered()        // rotate the image to right by 90 degrees
{
    if (currImage.isNull()) {       // if loading failed, interrupt
        //QMessageBox::information(this, tr("Image Viewer"), tr("No image available."));
        return;
    }
    QTransform myTransform;
    myTransform.rotate(90);
    currImage = currImage.transformed(myTransform);

    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));   // display the image
    fitImg();

    statusUpdate();
    printInfo();
}



/****************************************************************************************************/

/*********************************************** About menu *****************************************************/

void MIViewer::on_actionAbout_MIV_triggered()
{
    QMessageBox::about(this, tr("About this app"), tr("<h2>Marrk Image Viewer 0.2</h2>"
                                                      "<p>Copyright &copy; 2011 Marrk Inc."
                                                      "<p>This application is a basic image viewer."));
}

void MIViewer::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
/****************************************************************************************************/

/********************************** subroutines ******************************************************************/


bool MIViewer::getDir() {    // open the selected directory and display the first image in alphabetical order
    QDir newDir(NULL);
    newDir = QFileDialog::
                 getExistingDirectory(this, tr("Open Directory"),
                                      QDir::currentPath(), QFileDialog::ShowDirsOnly);
                                                    // get the directory containing images
    if (newDir.dirName().isEmpty()) {     // in case user cancels directory selection
        std::cout << "user failed to select newDir.\n";
        return 0;
    }
    std::cout << "From getDir(): user selected newDir.\n";
    workingDir = newDir;
    QDir::setCurrent(workingDir.absolutePath());   // set currentPath to image directory
    std::cout << "setting current path to: " << QDir::currentPath().toStdString() << std::endl;

    fileInfoList = getFileInfoList(workingDir.absolutePath());
    return 1;
}

void MIViewer::getFileName() {
    if (workingDir.dirName().isEmpty())     // in case user cancels directory selection
        return;
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.png";
    files = workingDir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);  // get the filename list from the directory
    if (files.isEmpty())
        return;         // in case there is no jpg nor png in the directory
    fileName = files.at(0);                        // set fileName to the first image name
}


QFileInfoList MIViewer::getFileInfoList(QString dirPath) {
    QDir dir(dirPath);

    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << ".bmp" << "tiff" << "ico" << "*.JPG" << "*.PNG";

    dir.setNameFilters(filters);
    return dir.entryInfoList(filters, QDir::Files);
}


void MIViewer::listWidget_itemDoubleClicked(QListWidgetItem *item) {

    this->goBackForm->hide();
    scaleFactor = 1;
    position = item->data(Qt::AccessibleDescriptionRole).toInt();
    fileName = fileInfoList.at(position).fileName();        // do not use the item->text() to pass the fileName, it'll mess up the images' alignment
    std::cout << "From doubleClicked: position after going back is: " << position << std::endl;

    currImage.load(fileName);       // loading the image

    if (currImage.isNull()) {       // if loading failed, interrupt
        QMessageBox::information(this, tr("Image Viewer"), tr("Cannot load %1.").arg(fileName));
        return;
    }

    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));   // display the image
    fitImg();

    statusUpdate();
    printInfo();

}

void MIViewer::statusUpdate() {
    statusBarInfo = "";
    //ui->statusBar->clearMessage();
    ui->statusBar->showMessage(fileName + statusBarInfo.
                               append(QString( "___original size:___")).
                               append(QString("%1").arg(currImage.width())).
                               append(QString(" * ")).
                               append(QString("%1").arg(currImage.height())).
                               append(QString("___")).
                               append(QString("%1").arg(position + 1)).append(QString(" of ")).
                               append(QString("%1").arg(fileInfoList.count())));
}

void MIViewer::fitImg() {

        this->imageLabel->adjustSize();
        double imgW = this->imageLabel->width();
        double imgH = this->imageLabel->height();
        double winW = this->centralWidget()->width() - 2;
        double winH = this->centralWidget()->height() - 2;
        QSize newSize;

        std::cout << "entering fitImg routine, img real size is: " << imgW << "*" << imgH << std::endl;

        if (imgW <= winW && imgH <= winH) {
            std::cout << "checkImgSize returns 0, displaying real size.\n";
            this->scrollArea->setWidgetResizable(false);
            this->imageLabel->adjustSize();
            printInfo();
        }
        else {
            std::cout << "checkImgSize returns 1, fitting to window.\n";
            if (winW * imgH <= winH * imgW) {
                newSize.setWidth(winW);             // hard code new width to avoid scalefactor issues
                newSize.setHeight(imgH * winW / imgW);
                std::cout << "resizing according to width, setting img width to window's width.\n";
                this->imageLabel->resize(newSize);
                scaleFactor = winW / imgW;
                printInfo();
                //continue;
            }
            else {
                //QSize newSize;
                newSize.setHeight(winH);
                newSize.setWidth(imgW * winH / imgH);
                std::cout << "resizing according to height, setting img width to window's height.\n";
                this->imageLabel->resize(newSize);
                scaleFactor = winH / imgH;
                printInfo();
            }

        }


}


void MIViewer::createItemList() {
    if (fileInfoList.isEmpty())
        return;

    this->goBackForm->showItems(fileInfoList);
}

void MIViewer::adjustScrollBar(QScrollBar *scrollBar, double factor) {

    scrollBar->setValue(int(factor * scrollBar->value()
                             + ((factor - 1) * scrollBar->pageStep()/2)));
}


void MIViewer::printInfo() {
    std::cout << "****************** begin printInfo ***********************************\n";
    std::cout << "size of mainwindow is: " << this->width() << "*" << this->height() << std::endl;
    std::cout << "size of centralWidget is: " << this->centralWidget()->width() << "*" << this->centralWidget()->height() << std::endl;
    std::cout << "real size of image is: " << this->currImage.width() << "*" << this->currImage.height()<< std::endl;
    std::cout << "size of imagelabel is: " << this->imageLabel->width() << "*" << this->imageLabel->height() << std::endl;
    std::cout << "********************* end printInfo ********************************\n";

}

/****************************************************************************************************/





void MIViewer::on_actionHorizontal_Flip_triggered()
{
    if (currImage.isNull()) {       // if loading failed, interrupt
        return;
    }

    currImage = currImage.mirrored();

    QTransform myTransform;
    myTransform.rotate(180);
    currImage = currImage.transformed(myTransform);

    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));   // display the image
    fitImg();

    statusUpdate();
    printInfo();
}

void MIViewer::on_actionVertical_Flip_triggered()
{
    if (currImage.isNull()) {       // if loading failed, interrupt
        return;
    }

    currImage = currImage.mirrored();

    this->imageLabel->setPixmap(QPixmap::fromImage(currImage));   // display the image
    fitImg();

    statusUpdate();
    printInfo();
}

void MIViewer::on_actionHide_Show_ToolBar_triggered(bool checked)
{
    ui->mainToolBar->setHidden(checked);
}

void MIViewer::on_actionHide_StatusBar_triggered(bool checked)
{
    ui->statusBar->setHidden(checked);
}



void MIViewer::setupStatusBar() {

//    this->statusBarLabel = new QLabel("something here.");
//    //this->statusBarLabel->setAlignment(Qt::AlignTop);
//    this->statusBarLabel->setAlignment(Qt::AlignTop);
//    this->statusBarLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
//    this->statusBarLabel->setLineWidth(0);
//    this->statusBarLabel->setMargin(2);
//    this->statusBarLabel->setMidLineWidth(0);
//    this->statusBarLabel->setFixedHeight(32);
//    //this->statusBarLabel->setText(tr("something here."));
//    ui->statusBar->addWidget(this->statusBarLabel, 20);

//        QFrame *status_frame = new QFrame();
//        status_frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);

//        QHBoxLayout *layout = new QHBoxLayout(status_frame);
//        layout->setContentsMargins(0, 0, 0, 0);
//        QProgressBar *bar = new QProgressBar(status_frame);
//        bar->setMaximumHeight(10);
//        bar->setMaximumWidth(100);


//        QCheckBox *box = new QCheckBox(tr("Check Mode"), status_frame);
//        box->setChecked(true);

//        layout->addWidget(bar);
//        layout->addWidget(box);
//        ui->statusBar->insertWidget(1, status_frame);

}



void MIViewer::on_actionFull_Screen_triggered(bool checked)     // this will remove title bar, tool bar, status bar
{                                                           // but not menubar, since it'll disable all short cut defined on menubar
    if (checked) {
        this->showFullScreen();
        MIViewer::on_actionHide_Show_ToolBar_triggered(true);
        MIViewer::on_actionHide_StatusBar_triggered(true);

    }
    else {
        this->showNormal();
        MIViewer::on_actionHide_Show_ToolBar_triggered(false);
        MIViewer::on_actionHide_StatusBar_triggered(false);

    }
}
