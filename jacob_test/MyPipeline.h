#ifndef MY_PIPELINE_H
#define MY_PIPELINE_H

#include <vector>
#include <vision/VisionPipeline.h>
#include <wpi/raw_ostream.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

class MyPipeline : public frc::VisionPipeline {
    public:
    double resultA = 0;
    double resultB = 0;
    
    MyPipeline();
    void Process(cv::Mat &mat) override;

    private:
    static const double thresh_hue[2];
    static const double thresh_sat[2];
    static const double thresh_val[2];

    static const int erosionSize;
    static const int dilationSize;

    std::vector<std::vector<cv::Point>> contourResults;
    
    void thresholdHSV(cv::Mat &input, cv::Mat &output,
                        const double hue[], const double sat[], const double val[]);
    
    void erodeSize(cv::Mat &input, cv::Mat &output, int size);

    void dilateSize(cv::Mat &input, cv::Mat &output, int size);
    
    void pipelineContours(cv::Mat &input, std::vector<std::vector<cv::Point>> &output);
};

#endif