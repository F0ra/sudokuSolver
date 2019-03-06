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

import QtQuick 2.0
import QtMultimedia 5.5
import qmlvideofilter.cl.test 1.0
import imageDraw 1.0

Rectangle{
    anchors.fill: parent
    color: "black"
    Item {
        width: 1080
        height: 1920
    //    anchors.fill: parent
        Camera {
            id: camera
        }

        VideoOutput {
            id: output
            source: camera
            filters: [ solverFilter ]
            anchors.fill: parent
            autoOrientation: true
            fillMode: VideoOutput.Stretch
            MouseArea {
                anchors.fill: parent;
                onClicked: camera.searchAndLock()
            }
        }

        SolverFilter {
            // This filter does not change the image. Instead, it provides some results calculated from the frame.
            id: solverFilter
            onFinished: {
                info.res = result.frameResolution.width + "x" + result.frameResolution.height;
                info.type = result.handleType;
                info.fmt = result.pixelFormat;
                liveImageItem.setImage(result.imReady);
            }
            onImageReady: {
                info.welcome = "gnanDeParr!";
                liveImageItem.setImage(imReady);
            }
        }

        ImageDraw {
                      id: liveImageItem
                   // anchors.fill: parent
                      width: 1080//2
                      height: 1920//2
                    }

        Text {
            id: info
            font.pointSize: 12
            color: "green"
            property string welcome
            property string res
            property string type
            property int fmt
            text:   welcome + " Input resolution: " + res// + " Input frame type: " + type + (fmt ? " Pixel format: " + fmt : "")
        }

        Item {
            id : zoomControl
            x : 0
            y : 0
            property real currentZoom : camera.digitalZoom
            property real maximumZoom : Math.min(4.0, camera.maximumDigitalZoom)
            signal zoomTo(real value)
            anchors.fill: parent
            onZoomTo: camera.setDigitalZoom(value)
            Text {
                    id: zoomText
                    anchors.horizontalCenter: parent.horizontalCenter
                    y : 0.05*parent.height
                    text: "x" + Math.round(zoomControl.currentZoom * 100) / 100 + "  Zoom";
                    font.bold: true
                    color: "orange"
                    style: Text.Raised; styleColor: "black"
                    opacity: 0.85
                    font.pixelSize: 48
            }
            BorderImage {
                id: border
                source: "qrc:/border3.png"
                anchors.fill: parent
//                width: 100; height: 100

            }
            MouseArea {
                   id : mouseArea
                   anchors.fill: parent
                   property real initialZoom : 0
                   property real initialPos : 0

                   onPressed: {
                       initialPos = mouseY
                       initialZoom = zoomControl.currentZoom
                       camera.searchAndLock()
                   }

                   onPositionChanged: {
                       if (pressed) {
                           var target = initialZoom * Math.pow(10, (initialPos-mouseY)/zoomControl.height);
                           target = Math.max(1, Math.min(target, zoomControl.maximumZoom))
                           zoomControl.zoomTo(target)
                       }
                   }
               }
        }
    }
}


/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/
