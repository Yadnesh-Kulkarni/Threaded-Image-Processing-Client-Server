#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include "../../include/cvutils.h"

using namespace cv;

// Bitwise_not
int convertImageDataToInverted(char *data, size_t* dataLen, char* pathToSave)
{
    Mat image = imdecode(Mat(1, *dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    Mat processedImage;
    bitwise_not(image, processedImage);
    imwrite( pathToSave, processedImage);

    return 0;
}

// Blur
int convertImageDataToBlur(char *data, size_t* dataLen, char* pathToSave)
{
    Mat image = imdecode(Mat(1, *dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    Mat processedImage;
    GaussianBlur(image, processedImage, Size(19 , 19), 0);
    imwrite( pathToSave, processedImage );

    return 0;
}

// BilateralFilter
int convertImageDataToBorderedImage(char *data, size_t* dataLen, char* pathToSave)
{
    Mat image = imdecode(Mat(1, *dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    Mat processedImage;
    copyMakeBorder(image, processedImage, 100,100,100,100, BORDER_REPLICATE);
    imwrite( pathToSave, processedImage );

    return 0;
}

// Grayscale Conversion
int convertImageDataToGrayScale(char *data, size_t* dataLen, char* pathToSave)
{
    Mat image = imdecode(Mat(1, *dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    Mat processedImage;
    cvtColor(image, processedImage, COLOR_BGR2GRAY);
    imwrite( pathToSave, processedImage );

    return 0;
}

int viewImage(char *data, size_t dataLen)
{
    Mat image = imdecode(Mat(1, dataLen, CV_8UC1, data),IMREAD_UNCHANGED);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }
    
    namedWindow("Image");
    resizeWindow("Image", Size(640, 480));
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

    namedWindow("Original Image");
    resizeWindow("Original Image", Size(640, 480));
    imshow("Original Image", image);
    return 0;
}