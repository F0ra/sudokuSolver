/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Multimedia module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QThread>
#include <QFileInfo>
#include <QQuickView>
#include <QQmlContext>
#include <QFutureWatcher>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QAbstractVideoFilter>
#include <QtConcurrent/QtConcurrent>

#include "imageDraw.hpp"
#include "opencvdetector.hpp"
#include "rgbframehelper.hpp"


// grubbed QImage format - QImage::Format_RGBA8888

class InfoFilterResult : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QImage imReady READ imReady)
    Q_PROPERTY(int pixelFormat READ pixelFormat)
    Q_PROPERTY(QString handleType READ handleType)
    Q_PROPERTY(QSize frameResolution READ frameResolution)

public:
    QImage imReady() const { return m_img;}
    InfoFilterResult() : m_pixelFormat(0) { }
    int pixelFormat() const { return m_pixelFormat; }
    QString handleType() const { return m_handleType; }
    QSize frameResolution() const { return m_frameResolution; }

private:
    QImage m_img;
    int m_pixelFormat;
    QString m_handleType;
    QSize m_frameResolution;
    friend class SolverFilter;
    friend class InfoFilterRunnable;
};

class SolverFilter : public QAbstractVideoFilter
{
    Q_OBJECT
    Q_PROPERTY(QImage imReady READ imReady)
public:
    QVideoFilterRunnable *createFilterRunnable() override;
    QImage imReady() const { return m_img;}
    bool isActive() const { return m_active;}
    void setActive(bool active) { m_active = active;}
signals:
    void imageReady(QImage *img);
    void finished(QObject *result);
private:
    bool m_active;
    QFuture<void> m_future;
    OpenCvDetector detector;
    friend class InfoFilterRunnable;

    void test(QImage img){
        setActive(false);
        m_future = QtConcurrent::run([=](){

            QPixmap pixmap=QPixmap::fromImage(img.convertToFormat(QImage::Format_Grayscale8));
            QMatrix rm;
            rm.rotate(90);
            pixmap = pixmap.transformed(rm);
            m_infoFilter.m_img = detector.performSomeOp(pixmap.toImage().scaled(1080/2,1920/2, Qt::KeepAspectRatio));

            m_infoFilter.m_frameResolution = QSize(1920,1080);
            m_infoFilter.m_handleType = QLatin1String("RGB");
//            m_infoFilter.m_pixelFormat;

            if(!m_infoFilter.m_img.isNull()) {
                emit this->finished(&m_infoFilter);
            }

            this->setActive(true);
        });
    }
    QImage m_img;
    InfoFilterResult m_infoFilter;

};

class InfoFilterRunnable : public QVideoFilterRunnable
{
public:
    InfoFilterRunnable(SolverFilter *filter) : m_filter(filter) { }
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

private:
    SolverFilter *m_filter;
};

QVideoFilterRunnable *SolverFilter::createFilterRunnable()
{
    return new InfoFilterRunnable(this);
}

QVideoFrame InfoFilterRunnable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);

    if (m_filter->isActive()) {
    m_filter->test(imageWrapper(*input));}

    return *input;
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<SolverFilter>("qmlvideofilter.cl.test", 1, 0, "SolverFilter");
    qmlRegisterType<ImageDraw>("imageDraw", 1, 0, "ImageDraw");
    QQuickView view;
    view.setSource(QUrl("qrc:///main.qml"));
    view.show();
    return app.exec();
}

#include "main.moc"
