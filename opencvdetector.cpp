#include "opencvdetector.hpp"

OpenCvDetector::OpenCvDetector() {
}

cv::Mat OpenCvDetector::QimageToMat(QImage &img) {
    return utils::QImageToCvMat(img,false);
}

QImage OpenCvDetector::MatToQimage(cv::Mat &openCvImg) {
    return utils::cvMatToQImage(openCvImg);
}

CvSVM OpenCvDetector::SVM;
Sudoku OpenCvDetector::sudoku;
cv::HOGDescriptor OpenCvDetector::hog;
bool OpenCvDetector::initState = false;

void OpenCvDetector::initSVM() {
    if (initState) return;
    QFile f_svm(":/SVM_model.xml");
    f_svm.open(QIODevice::ReadOnly);
    QString qstr = f_svm.readAll();
    f_svm.close();
    cv::FileStorage fs( qstr.toStdString(), cv::FileStorage::READ | cv::FileStorage::MEMORY);
    SVM.read(*fs,*fs.getFirstTopLevelNode());

    hog.winSize             = cv::Size(20,20);
    hog.blockSize           = cv::Size(10,10);
    hog.blockStride         = cv::Size(5,5);
    hog.cellSize            = cv::Size(10,10);
    hog.nbins               = 9;
    hog.derivAperture       = 1;
    hog.winSigma            = -1;
    hog.histogramNormType   = 0;
    hog.L2HysThreshold      = 0.2;
    hog.gammaCorrection     = 1;
    hog.nlevels             = 64;

    initState = true;
}

std::vector<std::vector<int>> OpenCvDetector::svmDetect(const cv::Mat warpedImg) {
    initSVM();
    cv::Mat sudokuGrid;
    cv::resize(warpedImg, sudokuGrid, cv::Size(20*9, 20*9), 0, 0, cv::INTER_CUBIC);
                    
    sudokuGrid =  cv::Scalar::all(255) - sudokuGrid;
    cv::cvtColor(sudokuGrid, sudokuGrid, cv::COLOR_RGBA2GRAY);
//    threshImg(sudokuGrid);
//    cv::Canny(sudokuGrid, sudokuGrid, 35, 90);

    int cropedAreaWidth = 20;
    int cropedAreaHeight = 20;
    cv::Mat tmp;
    cv::Mat hogMat = cv::Mat::zeros(20, 20, CV_8UC1);
    std::vector<std::vector<int>> greed(9);
    vector<float> descriptor;
    cv::Mat matDescriptor= cv::Mat::zeros(1, 20*20, CV_32F);
    for (int rows{0}; rows < 9; ++rows) {
        for (int cols{0}; cols < 9; ++cols) {
            tmp = sudokuGrid(cv::Rect(20*cols, 20*rows, cropedAreaWidth, cropedAreaHeight)).clone();
            extractEclipseArea(tmp);
//            tmp.convertTo(tmp, CV_32FC1);
//            tmp = tmp.reshape(0,1);
            hog.compute(tmp, descriptor);
            matDescriptor = cv::Mat(1, 81, CV_32FC1, (float*)descriptor.data());
            greed[rows].push_back(SVM.predict(matDescriptor));
        }
    }
    return greed;
}

QImage OpenCvDetector::performSomeOp(const QImage img) {
    //  img format : 4

    cv::Mat mat = utils::QImageToCvMat(img);
    cv::Mat preProc = mat.clone();
    cv::cvtColor(preProc,preProc,cv::COLOR_RGB2RGBA);

    cv::cvtColor(mat,mat,cv::COLOR_BGR2GRAY);
    
    // mat size:===============================>>>>> 960  :  540
    // Setup a rectangle to define your region of interest
    cv::Rect myROI(40, 100, 460, 400);

    // Crop the full image to that image contained by the rectangle myROI
    // Note that this doesn't copy the data
    cv::Mat croppedImage = mat(myROI);

    cv::Mat thresh;
    cv::adaptiveThreshold(croppedImage, thresh, 255, 1, 1, 11, 15);

    // find contours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    // Find contours
    cv::findContours( thresh, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    //biggest rectangle
    double size_rectangle{0};
    double size_rectangle_max{0};
    double sizeThresh{51000};
    std::vector<cv::Point> approximation;
    std::vector<cv::Point> big_rectangle;

    for (int i{}; i < contours.size(); ++i) {

        //aproximate countours to polygons
        approxPolyDP(contours[i], approximation, 4, true);
        //has the polygon 4 sides?

        if (approximation.size() != 4) continue;

        //is the polygon convex ?
        if(!isContourConvex(approximation)) continue;

        //area of the polygon
        size_rectangle = cv::contourArea(approximation);
        if(size_rectangle < sizeThresh) continue;

        //store the biggest
        if (size_rectangle > size_rectangle_max) {
            size_rectangle_max = size_rectangle;
            big_rectangle = approximation;
        }
    }

    contours.clear();
    contours.push_back(big_rectangle);

    if(size_rectangle_max<sizeThresh) return QImage();
    cv::Mat drawing = cv::Mat::zeros( mat.size(), CV_8UC4);

    cv::Mat out(croppedImage.rows, croppedImage.cols, CV_8UC4);
    std::vector<cv::Mat> channels;
        channels.push_back(croppedImage);
        channels.push_back(croppedImage);
        channels.push_back(croppedImage);
        channels.push_back(croppedImage);

    cv::merge(channels, out);

    cv::Mat drawingCropped = drawing(myROI);

    cv::Scalar color = cv::Scalar(  255, 102, 0, 255 );
    cv::drawContours( drawingCropped, contours, 0, color, 1, 8);

    cv::Rect warpedROI(54, 520, 432, 432);
    // Crop the full image to that image contained by the rectangle myROI
    // Note that this doesn't copy the data
    cv::Mat warpedImage = drawing(warpedROI);
    // cv::Mat fillGreen(warpedImage.size(),CV_8UC4,cv::Scalar(0,0,255,255));
    // fillGreen.copyTo(warpedImage);
    warpImage(preProc(myROI), warpedImage, big_rectangle);

    auto initGrid = svmDetect(warpedImage);
    auto solvedGrid = sudoku.solveSudoku(initGrid);

    if (solvedGrid.size()) drawSolvedNums(warpedImage, initGrid, solvedGrid);
    if (checkIfSolved(solvedGrid) && !isSameGrid(solvedGrid)) saveCapturedImg(warpedImage);
    drawDetectedNums(warpedImage, initGrid);


    cv::Mat mat1 = drawing.clone();
    //  cv::Mat mat1 = out.clone();
    QImage image(mat1.data,
                mat1.cols, mat1.rows,
                static_cast<int>(mat1.step),
                QImage::Format_RGBA8888);

    // saveCapturedImg(warpedImage);

    return image.scaled(1920,1080, Qt::KeepAspectRatio);
}

void OpenCvDetector::saveCapturedImg(const cv::Mat matToSave) {
    static int imgCount{0};

    QImage image(matToSave.data,
                matToSave.cols, matToSave.rows,
                static_cast<int>(matToSave.step),
                QImage::Format_RGBA8888);

    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (path.isEmpty()) return;
    path.append("/SudokuSolver");
    QString qstr = "/capturedImg_";
    qstr += QString::number(imgCount);
    qstr += ".png";
    path.append(qstr);
    image.save(path);
    ++imgCount;
}

void OpenCvDetector::warpImage(const cv::Mat &inputImage, cv::Mat &outputImage,  std::vector<cv::Point> &contour) {
    //check quadrants
    //smallest sum is left upper quadrant
    int smallestSum = contour[0].x + contour[0].y;
    int smallestIndex{0};
    for (int i{1}; i < 4; ++i) {
        if( (contour[i].x + contour[i].y) < smallestSum) smallestIndex = i;
    }

    if (smallestIndex !=0) {

        cv::Point tmp0 = contour[0];
        cv::Point tmp1 = contour[1];
        cv::Point tmp2 = contour[2];
        cv::Point tmp3 = contour[3];

        contour[0] = tmp1;
        contour[1] = tmp2;
        contour[2] = tmp3;
        contour[3] = tmp0;
    }

    // warping
    // Create vectors to store the corners
    vector<cv::Point2f> originalCorners;
    vector<cv::Point2f> destinationCorners;

    // Output image size of 450x450
    int ouputImageWidth = outputImage.cols;
    int outputImageHeight = outputImage.rows;

    // Put the Sudoku corners in your originalCorners
    originalCorners.clear();
    originalCorners.push_back({ static_cast<float>(contour[0].x), static_cast<float>(contour[0].y)});
    originalCorners.push_back({ static_cast<float>(contour[1].x), static_cast<float>(contour[1].y)});
    originalCorners.push_back({ static_cast<float>(contour[2].x), static_cast<float>(contour[2].y)});
    originalCorners.push_back({ static_cast<float>(contour[3].x), static_cast<float>(contour[3].y)});


    // Create an empty image (450x450) to output your transformation result in
    cv::Mat transformedOutputImage(ouputImageWidth, outputImageHeight, CV_8UC4);

    // Now put the corners of the output image so the warp knows where to warp to
    destinationCorners.clear();
    destinationCorners.push_back(cv::Point2f(0, 0));
    destinationCorners.push_back(cv::Point2f(0, outputImageHeight));
    destinationCorners.push_back(cv::Point2f(ouputImageWidth, outputImageHeight));
    destinationCorners.push_back(cv::Point2f(ouputImageWidth, 0));

    // Now we have all corners sorted, so we can create the warp matrix
    cv::Mat warpMatrix = cv::getPerspectiveTransform(originalCorners, destinationCorners);

    // And now we can warp the Sudoku in the new image
    warpPerspective(inputImage, outputImage, warpMatrix, cv::Size(ouputImageWidth, outputImageHeight));
}

void OpenCvDetector::drawDetectedNums(cv::Mat &mat, vector<vector<int>> grid) {   
    int height = mat.rows;
    int width = mat.cols;
    int cropedAreaHeight = height/9;
    int cropedAreaWidth = width/9;
    cv::Scalar color = cv::Scalar(  0, 0, 0, 255 );
    QString num;
    for (int rows{0}; rows < 9; ++rows) {
        for (int cols{0}; cols < 9; ++cols) {
            num = QString::number(grid[rows][cols]);
            if (grid[rows][cols]==0) continue;
            cv::putText(mat,num.toStdString(),cv::Point(cols*cropedAreaWidth,
                                                        rows*cropedAreaHeight + cropedAreaHeight),
                                                        cv::FONT_HERSHEY_SCRIPT_COMPLEX,
                                                        1,
                                                        color);
        }
    }
}

bool OpenCvDetector::isSameGrid(vector<vector<int>> grid) {
    static vector<vector<int>> prevGrid;
    if ( prevGrid.size() == 0 || grid.size() == 0) {
        prevGrid = grid;
        return false;
    }
    for (size_t row{}; row < 9; ++row) {
        for (size_t col{}; col < 9 ; ++col) {
            if (prevGrid[row][col] != grid[row][col]) {
                prevGrid = grid;
                return false;
            }
        }
    }
    return true;
}


bool OpenCvDetector::checkIfSolved(vector<vector<int>> grid) {
    if (grid.size() == 0) return false;
    for (size_t row{0}; row < 9; ++row) {
        for (size_t col{0}; col < 9; ++col) {
            if (grid[row][col]==0) return 0;
        }
    }
    return true;
}

void OpenCvDetector::drawSolvedNums(cv::Mat &mat, vector<vector<int>> initGrid,
                                    vector<vector<int>> solvedGrid ) {
    int height = mat.rows;
    int width = mat.cols;
    int cropedAreaHeight = height/9;
    int cropedAreaWidth = width/9;
    int offset = 15;
    int backGroundOffset = 4;
    cv::Scalar color = cv::Scalar(  255, 102, 0, 255 );
    QString num;
    cv::Mat tmp;
    for (size_t rows{0}; rows < 9; ++rows) {
        for (size_t cols{0}; cols < 9; ++cols) {
            num = QString::number(solvedGrid[rows][cols]);
            if (initGrid[rows][cols] == solvedGrid[rows][cols]) continue;
            tmp = mat(cv::Rect( cols*cropedAreaWidth + backGroundOffset,
                                rows*cropedAreaHeight + backGroundOffset,
                                cropedAreaWidth - backGroundOffset,
                                cropedAreaHeight - backGroundOffset));
            tmp =  cv::Scalar(0, 0, 0, 255);

            cv::putText(mat,num.toStdString(),cv::Point(cols*cropedAreaWidth + offset,
                                                        rows*cropedAreaHeight + cropedAreaHeight - offset),
                                                        cv::FONT_HERSHEY_SCRIPT_COMPLEX,
                                                        1,
                                                        color);
        }
    }
}


cv::Mat OpenCvDetector::extractEclipseArea(cv::Mat &img, bool cloneImg) {
    cv::Mat clone;
    cloneImg ? clone = img.clone() : clone = img;

    int height = clone.rows;
    int width = clone.cols;
    int centerX = width / 2;
    int centerY = height / 2;

    // Vertical Major Axis
    // x^2/a^2 + y^2/b^2 = 1
    // a < b
    double a = 0.7 * centerX;
    double b = 0.75 * centerY;

    int x, y;

    vector<uchar> eclipseMask(height*width); 
    for (int row{0}; row < height; ++row)
    {
        for (int col{0}; col < width ; ++col)
        {
            x = col - centerX;
            y = row - centerY;
            if ( ( std::pow(x, 2)/std::pow(a, 2) + std::pow(y, 2)/std::pow(b, 2) ) < 1) {
                 eclipseMask[row*height + col] = 1;
            } 
        }
    }

    int channels = clone.channels();
    int nRows = clone.rows;
    int nCols = clone.cols * channels;

    if (clone.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    int i, j;
    uchar* p;
    for( i = 0; i < nRows; ++i) {
        p = clone.ptr<uchar>(i);
        for ( j = 0; j < nCols; ++j) {
            p[j] *= eclipseMask[i*height + j];
        }
    }

    return clone;
}

cv::Mat OpenCvDetector::threshImg(cv::Mat &img, bool cloneImg, int imgMean) {
    static double meanMul = 1.08;
    meanMul += 0.04;
    if (meanMul > 1.2) meanMul = 1.12;
    cv::Mat clone;
    cloneImg ? clone = img.clone() : clone = img;

    int height = clone.rows;
    int width = clone.cols;
    
    if( imgMean == 0 ) {
        cv::Scalar tempVal = cv::mean( clone );
        imgMean = meanMul * tempVal.val[0]; // 1.17
    }

    std::vector<uchar> threshMask(height*width);
    uchar *ptr;
    ptr = clone.data;
    for (int row{0}; row < height; ++row)
    {
        for (int col{0}; col < width ; ++col)
        {
            if ( *ptr++ > imgMean ) {
                 threshMask[row*height + col] = 1;
            } 
        }
    }

    int channels = clone.channels();
    int nRows = clone.rows;
    int nCols = clone.cols * channels;

    if (clone.isContinuous()) {
        nCols *= nRows;
        nRows = 1;
    }

    int i, j;
    uchar* p;
    for( i = 0; i < nRows; ++i) {
        p = clone.ptr<uchar>(i);
        for ( j = 0; j < nCols; ++j) {
            p[j] *= threshMask[i*height + j];
        }
    }

    return clone;
}
