#ifndef MY_PIPELINE_H
#define MY_PIPELINE_H

#include <vector>
#include <vision/VisionPipeline.h>
#include <wpi/raw_ostream.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

class MyPipeline : public frc::VisionPipeline
{
public:
    double returnedDistance = -1;
    double returnedHorizAngle = -1;

    MyPipeline();
    void Process(cv::Mat &mat) override;

private:
    double targetDistance = 0;       // dx in image
    double targetTheta = 0;          // Red theta in image
    double boundingBoxMidpointX = 0; // Midpoint of the bounding box on the X axis
    double imageMidpointX = 0;       // Midpoint of the image on the X axis
    double yawOffset = 0;            // Yaw offset in degrees
	
    static const double targetHeight;
    static const double angleOffset;
    static const double verticalFOV;
    static const double horizontalFOV;

    static const double thresh_hue[2];
    static const double thresh_sat[2];
    static const double thresh_val[2];

    static const int erosionSize;
    static const int dilationSize;

    double boundingPoints[4];

    std::vector<std::vector<cv::Point>> contourResults;

    void thresholdHSV(cv::Mat &input, cv::Mat &output,
                      const double hue[], const double sat[], const double val[]);

    void erodeSize(cv::Mat &input, cv::Mat &output, int size);

    void dilateSize(cv::Mat &input, cv::Mat &output, int size);

    void drawAndUpdate(cv::Mat &input, std::vector<std::vector<cv::Point>> &output);

    void trajectory(cv::Mat &input);

	double pxToDegrees(double pixel, int orientation, int imageWidth, int imageHeight);

    double getDistance(int imageWidth, int imageHeight);

    double getHorizontalAngle(int imageWidth, int imageHeight);
};

#endif
