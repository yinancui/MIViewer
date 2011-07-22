#include "gobackform.h"
#include "ui_gobackform.h"
#include <iostream>
#include <QVBoxLayout>
#include <QImage>
#include <QtCore>   // o.w. qtconcurrent::mapped error



/********************************************* constants for icons and thumbnails ****************************************************/
const int WIDTH_ICON = 100;         // think about these configs
const int HEIGHT_ICON = 90;

const int WIDTH_IMAGE = 90;
const int HEIGHT_IMAGE = 65;
/*************************************************************************************************/


/******************************* ctor and dtor ******************************************************************/
GoBackForm::GoBackForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GoBackForm)
{
    ui->setupUi(this);
    listWidget = new QListWidget(this);
    this->listWidget->setIconSize(QSize(100, 100)); // setting the size of the thumbnails in the listwidget

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setSpacing(10);
    vbox->addWidget(this->listWidget);
    this->setLayout(vbox);

    this->listWidget->setViewMode(QListView::IconMode);
    this->listWidget->setFlow(QListView::LeftToRight);
    //this->listWidget->setSpacing(10);
    //this->listWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //this->listWidget->setSizePolicy;

    imagesShow_ = new QFutureWatcher<QImage>(this);
    connect(imagesShow_, SIGNAL(resultReadyAt(int)), SLOT(setItemInList(int)));
    connect(imagesShow_, SIGNAL(finished()), SLOT(finished()));

}

GoBackForm::~GoBackForm()
{
    delete ui;
}
/*************************************************************************************************/


/************************************** some functions ***********************************************************/
void GoBackForm::waitFuture() {
    if (imagesShow_->isRunning()) {
        imagesShow_->cancel();
        imagesShow_->waitForFinished();
    }
}


void GoBackForm::setItemInList(int index)
{
    this->listWidget->item(index)->setIcon(QIcon(QPixmap::fromImage(imagesShow_->resultAt(index))));

}


void GoBackForm::finished() {

}
/*************************************************************************************************/



/**************************************** fileList processing function *********************************************/

void GoBackForm::showItems(const QFileInfoList &fileInfoList) {
    waitFuture();

    this->listWidget->clear();
    int count = 0;
    count  = fileInfoList.count();

    std::cout << "number of image files here: " << fileInfoList.count() << std::endl;

    for (int i = 0; i < count; i++) {

        QListWidgetItem* item = new QListWidgetItem;
        item->setIcon(QIcon(prepareIcon(fileInfoList.at(i))));

        //item->setText(fileInfoList.at(i).fileName());          // cannot pass fileName back to MW?
        //item->text() = fileInfoList.at(i).fileName().toStdString();
        //item->setData();

//        QFontMetrics font_metrics(painter.font());
//        QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, WIDTH_IMAGE);
//        painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

        item->setData(Qt::WhatsThisRole, QString(fileInfoList.at(i).filePath()));
        item->setData(Qt::AccessibleDescriptionRole, QVariant(i));  // saving the sequence number of this item in fileInfoList
        this->listWidget->addItem(item);
    }

    imagesShow_->setFuture(QtConcurrent::mapped(fileInfoList, prepareImage));

}
/*************************************************************************************************/



/**************************************** creating the icons and images in the listWidget **********************************/

QPixmap prepareIcon(const QFileInfo &infoFile)          // this part is taken directly without mod, needs more looking into
{
    QImage imageIcon(QLatin1String(":images/qt-logo.png"));
    imageIcon = imageIcon.scaledToHeight(HEIGHT_IMAGE, Qt::SmoothTransformation);

    int image_width = imageIcon.width();
    int image_height = imageIcon.height();

    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QPixmap pixDraw(QSize(WIDTH_ICON, HEIGHT_ICON));
    QPainter painter(&pixDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON), Qt::white);


    painter.setPen(Qt::black);
    painter.drawImage(target, imageIcon, source);
    painter.drawRect(target);


    painter.setPen(Qt::darkBlue);

    QRect rect((WIDTH_ICON - image_width) / 2, image_height + 2, image_width, HEIGHT_ICON - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, image_width);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    return pixDraw;
}




QImage prepareImage(const QFileInfo &infoFile)
{
    QImageReader imageReader(infoFile.filePath());
    QSize size;
    int image_width = WIDTH_IMAGE;
    int image_height = HEIGHT_IMAGE;

    if (imageReader.supportsOption(QImageIOHandler::Size))
    {
        size = imageReader.size();
        image_width = size.width();
        image_height = size.height();
    }

    double ratio = (double)image_width / (double)image_height;
    image_height = HEIGHT_IMAGE;
    image_width = ratio * image_height;

    imageReader.setScaledSize(QSize(image_width, image_height));
    QImage image = imageReader.read();



    if (image.isNull())
    {
        QImage imageIcon(QLatin1String(":images/qt-logo.png"));
        image = imageIcon;
        image = image.scaledToHeight(HEIGHT_IMAGE, Qt::SmoothTransformation);

        image_width = image.width();
        image_height = image.height();
    }


    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QImage imgDraw(QSize(WIDTH_ICON, HEIGHT_ICON), QImage::Format_RGB32);
    QPainter painter(&imgDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON), Qt::white);


    painter.setPen(Qt::black);
    painter.drawImage(target, image, source);
    painter.drawRect(target);

    painter.setPen(Qt::darkBlue);

    QRect rect((WIDTH_ICON - WIDTH_IMAGE) / 2, image_height + 2, WIDTH_IMAGE, HEIGHT_ICON - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, WIDTH_IMAGE);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    return (imgDraw);
}

/*************************************************************************************************/

