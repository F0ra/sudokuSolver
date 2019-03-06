#include "imageDraw.hpp"

ImageDraw::ImageDraw(QQuickItem *parent) : QQuickPaintedItem(parent)
{
//    QImage image(200, 200, QImage::Format_RGB32);
//    image.fill(QColor("magenta").rgba());
//    this->current_image = image;
}

void ImageDraw::paint(QPainter *painter)
{
    QRectF bounding_rect = boundingRect();
    QImage scaled = this->current_image.scaledToHeight(bounding_rect.height());
    QPointF center = bounding_rect.center() - scaled.rect().center();

    if(center.x() < 0)
        center.setX(0);
    if(center.y() < 0)
        center.setY(0);
   painter->drawImage(center, scaled);
}

QImage ImageDraw::image() const
{    return this->current_image;
}

void ImageDraw::setImage(const QImage &image)
{
    this->current_image = image;
    update();
}
