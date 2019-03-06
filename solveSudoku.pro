TEMPLATE = app
TARGET = solveSudoku

QT += quick multimedia

SOURCES = main.cpp \
    imageDraw.cpp \
    opencvdetector.cpp \
    mat_and_qimage.cpp

HEADERS += imageDraw.hpp \
    utils.hpp \
    opencvdetector.hpp \
    rgbframehelper.hpp \
    rgbframehelper.hpp \
    mat_and_qimage.hpp \
    solver.hpp

RESOURCES += img.qrc\
             qmlvideofilter_opencl.qrc

OTHER_FILES = main.qml

target.path = $$[QT_INSTALL_EXAMPLES]/multimedia/video/qmlvideofilter_opencl
INSTALLS += target

#linux: {
#    # using pkg-config
#    CONFIG += link_pkgconfig
#    PKGCONFIG += opencv
#}


android {
ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources

# Change this paths to your own OpenCV for Android installation
INCLUDEPATH += /media/f0ra/4246FDD146FDC627/code/androidOpencvSdk/opencv-2.4.13.6-android-sdk/sdk/native/jni/include

# Change the last part (armeabi-v7a) according to your build
OPENCV3RDPARTYLIBS = /media/f0ra/4246FDD146FDC627/code/androidOpencvSdk/opencv-2.4.13.6-android-sdk/sdk/native/3rdparty/libs/armeabi-v7a
OPENCVNATIVELIBS = /media/f0ra/4246FDD146FDC627/code/androidOpencvSdk/opencv-2.4.13.6-android-sdk/sdk/native/libs/armeabi-v7a


LIBS += $$OPENCV3RDPARTYLIBS/libIlmImf.a \
$$OPENCV3RDPARTYLIBS/liblibjasper.a \
$$OPENCV3RDPARTYLIBS/liblibjpeg.a \
$$OPENCV3RDPARTYLIBS/liblibpng.a \
$$OPENCV3RDPARTYLIBS/liblibtiff.a

LIBS += $$OPENCVNATIVELIBS/libopencv_androidcamera.a \
$$OPENCVNATIVELIBS/libopencv_calib3d.a \
$$OPENCVNATIVELIBS/libopencv_contrib.a \
$$OPENCVNATIVELIBS/libopencv_core.a \
$$OPENCVNATIVELIBS/libopencv_features2d.a \
$$OPENCVNATIVELIBS/libopencv_flann.a \
$$OPENCVNATIVELIBS/libopencv_highgui.a \
$$OPENCVNATIVELIBS/libopencv_imgproc.a \
$$OPENCVNATIVELIBS/libopencv_info.so \
$$OPENCVNATIVELIBS/libopencv_java.so \
$$OPENCVNATIVELIBS/libopencv_legacy.a \
$$OPENCVNATIVELIBS/libopencv_ml.a \
$$OPENCVNATIVELIBS/libopencv_objdetect.a \
$$OPENCVNATIVELIBS/libopencv_ocl.a \
$$OPENCVNATIVELIBS/libopencv_photo.a \
$$OPENCVNATIVELIBS/libopencv_stitching.a \
$$OPENCVNATIVELIBS/libopencv_superres.a \
$$OPENCVNATIVELIBS/libopencv_ts.a \
$$OPENCVNATIVELIBS/libopencv_video.a \
$$OPENCVNATIVELIBS/libopencv_videostab.a \
}
DISTFILES += \


