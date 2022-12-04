#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../../include/cvutils.h"

using namespace cv;

int convertImageDataToGrayScale(char *data, size_t* dataLen, char* pathToSave)
{
    Mat image = imdecode(Mat(1, *dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    Mat gray_image;
    cvtColor(image, gray_image, COLOR_BGR2GRAY);
    imwrite( pathToSave, gray_image );

    return 0;
}

int viewImage(char *data, size_t dataLen)
{
    Mat image = imdecode(Mat(1, dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
  
    namedWindow("Image", WINDOW_AUTOSIZE);
    imshow("Image", image);
    waitKey(0);
    return 0;
}

int viewImageWithPath(char* path)
{
    Mat image = imread(path);
    if(!image.data){
        printf("No image data \n");
        return -1;
    }

    imshow("Original Image", image);
    return 0;
}