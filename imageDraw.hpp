#ifndef IMAGEITEM_HHP
#define IMAGEITEM_HHP

#include <QImage>
#include <QThread>
#include <QPainter>
#include <QQuickItem>
#include <QFutureWatcher>
#include <QQuickPaintedItem>
#include <QtConcurrent/QtConcurrent>

class ImageDraw : public QQuickPaintedItem
{
Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
public:
    ImageDraw(QQuickItem *parent = nullptr);
    Q_INVOKABLE void setImage(const QImage &image);
    void paint(QPainter *painter);
    QImage image() const;
    bool isActive() const { return m_active;}
    void setActive(bool active) { m_active = active;}

signals:
    void imageChanged();


private:
    bool m_active{true};
    QImage current_image;
};


#endif // IMAGEITEM_HHP
