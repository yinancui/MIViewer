#ifndef MIVIEWER_H
#define MIVIEWER_H

#include <QMainWindow>
#include <QPrinter>
#include <QStringList>
#include <QDir>
#include <QSize>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QFileInfo>
#include "gobackform.h"
#include <QFileInfo>
#include <QListWidget>


namespace Ui {
    class MIViewer;
}

class GoBackForm;
class MIViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit MIViewer(QWidget *parent = 0);
    ~MIViewer();

    QFileInfoList   fileInfoList;
    //friend class GoBackForm;
    //int dummy;

private:
    Ui::MIViewer *ui;

    int  position;              // position of a image in the directory
    double scaleFactor;

    bool getDir();
    void getFileName();
    void statusUpdate();
    void fitImg();
    void adjustScrollBar(QScrollBar* scrollBar, double factor);
    void printInfo();
    void setupStatusBar();


    QImage      currImage;
    QPrinter    printer;
    QStringList files;
    QString     fileName;       // for displaying loading error message
    QDir        workingDir;     // for storing the application's operating directory

    QString     statusBarInfo;
    QScrollArea* scrollArea;
    QLabel*     imageLabel;
    QLabel*     statusBarLabel;

    QFileInfoList   getFileInfoList(QString dirPath);
    GoBackForm*     goBackForm;

private slots:

    void on_actionFull_Screen_triggered(bool checked);
    void on_actionHide_StatusBar_triggered(bool checked);
    void on_actionHide_Show_ToolBar_triggered(bool checked);
    void on_actionVertical_Flip_triggered();
    void on_actionHorizontal_Flip_triggered();
    void on_actionRotate_Right_triggered();
    void on_actionRotate_Left_triggered();
    void on_actionGo_Back_triggered();
    void on_actionFit_to_Window_triggered();
    void on_actionAbout_Qt_triggered();
    void on_actionAbout_MIV_triggered();
    void on_actionNormal_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_In_triggered();
    void on_actionNext_triggered();
    void on_actionPrevious_triggered();
    void on_actionQuit_triggered();
    void on_actionPrint_triggered();
    void on_actionOpen_triggered();
    void listWidget_itemDoubleClicked(QListWidgetItem *item);
    void createItemList();
};

#endif // MIVIEWER_H
