#include "MyPipeline.h"

const double MyPipeline::thresh_hue[] = {65, 81};
const double MyPipeline::thresh_sat[] = {0, 255};
const double MyPipeline::thresh_val[] = {30, 255};

const int MyPipeline::erosionSize = 0;
const int MyPipeline::dilationSize = 2;

MyPipeline::MyPipeline()
    : contourResults()
{
}

void MyPipeline::Process(cv::Mat &mat)
{
    // wpi::outs() << "Received frame with " << mat.channels() << " channels and size " << mat.cols << "x" << mat.rows << "\n";

    thresholdHSV(mat, mat, thresh_hue, thresh_sat, thresh_val);
    erodeSize(mat, mat, erosionSize);
    dilateSize(mat, mat, dilationSize);

    pipelineContours(mat, contourResults);

    wpi::outs() << "Contours count: " << contourResults.size() << "\n";

    // Did the math...main.cpp will send these to RIO
    resultA = 0.25;
    resultB = 0.75;
}

void MyPipeline::thresholdHSV(cv::Mat &input, cv::Mat &output,
                              const double hue[], const double sat[], const double val[])
{
    cvtColor(input, output, CV_BGR2HSV);
    inRange(input, cv::Scalar(hue[0], sat[0], val[0]), cv::Scalar(hue[1], sat[1], val[1]), output);
}

void MyPipeline::erodeSize(cv::Mat &input, cv::Mat &output, int size)
{
    erode(input, output,
            getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2 * erosionSize + 1, 2 * erosionSize + 1),
                cv::Point(erosionSize, erosionSize)
            ));
}

void MyPipeline::dilateSize(cv::Mat &input, cv::Mat &output, int size)
{
    dilate(input, output,
            getStructuringElement(
                cv::MORPH_ELLIPSE,
                cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1),
                cv::Point(dilationSize, dilationSize)
            ));
}

void MyPipeline::pipelineContours(cv::Mat &input, std::vector<std::vector<cv::Point>> &output)
{
    cv::Mat copy = input.clone(); // findContours modifies its input
    std::vector<cv::Vec4i> hierarchy;
    findContours(copy, output, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
}
