#ifndef OPENCVDETECTOR_HPP
#define OPENCVDETECTOR_HPP

#include <cmath>
#include <vector>
#include <string>

#include <QFile>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QtGlobal>
#include <QFileInfo>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <opencv2/ml.hpp>
#include <opencv2/core.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "utils.hpp"
#include "solver.hpp"
#include "mat_and_qimage.hpp"

using std::vector;

class OpenCvDetector {
 public:
    OpenCvDetector();
    cv::Mat QimageToMat(QImage &img);
    QImage MatToQimage(cv::Mat &openCvImg);
    void drawDetectedNums(cv::Mat &mat, vector<vector<int>> grid);
    void warpImage(const cv::Mat &inputImage, cv::Mat &outputImage, vector<cv::Point> &contour);
    void drawSolvedNums(cv::Mat &mat, vector<vector<int>> initGrid, vector<vector<int>> solvedGrid );

    void initSVM();
    static CvSVM SVM;
    static Sudoku sudoku;
    static bool initState;
    static cv::HOGDescriptor hog;
    QImage performSomeOp(const QImage img);
    bool isSameGrid(vector<vector<int>> grid);
    bool checkIfSolved(vector<vector<int>> grid);
    void saveCapturedImg(const cv::Mat matToSave);
    vector<vector<int>> svmDetect(const cv::Mat warpedImg);
    cv::Mat extractEclipseArea(cv::Mat &img, bool cloneImg=false);
    cv::Mat threshImg(cv::Mat &img, bool cloneImg=false, int imgMean = 0);
};

#endif // OPENCVDETECTOR_HPP
