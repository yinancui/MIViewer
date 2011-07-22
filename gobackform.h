#ifndef GOBACKFORM_H
#define GOBACKFORM_H

#include <QWidget>
#include "miviewer.h"
#include <QWidgetItem>
#include <QListWidget>
#include <QPainter>
#include <QFuture>
#include <QFutureWatcher>
#include <QImageReader>
#include <QImageIOHandler>
#include <QFileIconProvider>
#include <QImage>

namespace Ui {
    class GoBackForm;
}

class GoBackForm : public QWidget
{
    Q_OBJECT

public:
    explicit GoBackForm(QWidget *parent = 0);
    ~GoBackForm();
    void    showItems(const QFileInfoList& fileInfoList);
    QListWidget* listWidget;
    //void    createItems(const QFileInfoList& fileInfoList);


private:
    Ui::GoBackForm *ui;
    QFutureWatcher<QImage> *imagesShow_;
    void waitFuture();

private Q_SLOTS:
    void setItemInList(int index);
    void finished();

};


QPixmap prepareIcon(const QFileInfo &infoFile);
QImage prepareImage(const QFileInfo &infoFile);

#endif // GOBACKFORM_H
