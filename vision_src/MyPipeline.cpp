#include "MyPipeline.h"

const double MyPipeline::thresh_hue[] = {55, 63};
const double MyPipeline::thresh_sat[] = {254, 255};
const double MyPipeline::thresh_val[] = {33, 188};

const int MyPipeline::erosionSize = 0;
const int MyPipeline::dilationSize = 2;

const double MyPipeline::targetHeight = 78.5;  // dy in image - from target height to camera height (e.g. 500cm)
const double MyPipeline::angleOffset = 60.0;   // Angle that the camera is set to relative the ground (e.g. perpendicular to the ground is 90deg)
const double MyPipeline::verticalFOV = 48.8;   // Vertical field of view of the camera
const double MyPipeline::horizontalFOV = 62.2; // Horizontal field of view of the camera

MyPipeline::MyPipeline()
    : contourResults()
{
    robotHeading = nt::NetworkTableInstance::GetDefault().GetEntry("robot/heading");
}

void MyPipeline::Process(cv::Mat &mat)
{
    //grab heading for mat, defaulting to old heading
    imageCaptureHeading = robotHeading.GetDouble(imageCaptureHeading);

    // wpi::outs() << "Received frame with " << mat.channels() << " channels and size " << mat.cols << "x" << mat.rows << "\n";

    thresholdHSV(mat, mat, thresh_hue, thresh_sat, thresh_val);
    erodeSize(mat, mat, erosionSize);
    dilateSize(mat, mat, dilationSize);

    drawAndUpdate(mat, contourResults);

    trajectory(mat);
    wpi::outs() << "Contours count: " << contourResults.size() << "\n";

    // Did the math...main.cpp will send these to RIO
}

void MyPipeline::thresholdHSV(cv::Mat &input, cv::Mat &output,
                              const double hue[], const double sat[], const double val[])
{
    cvtColor(input, output, CV_BGR2HSV);
    inRange(output, cv::Scalar(hue[0], sat[0], val[0]), cv::Scalar(hue[1], sat[1], val[1]), output);
}

void MyPipeline::erodeSize(cv::Mat &input, cv::Mat &output, int size)
{
    erode(input, output,
          getStructuringElement(
              cv::MORPH_ELLIPSE,
              cv::Size(2 * erosionSize + 1, 2 * erosionSize + 1),
              cv::Point(erosionSize, erosionSize)));
}

void MyPipeline::dilateSize(cv::Mat &input, cv::Mat &output, int size)
{
    dilate(input, output,
           getStructuringElement(
               cv::MORPH_ELLIPSE,
               cv::Size(2 * dilationSize + 1, 2 * dilationSize + 1),
               cv::Point(dilationSize, dilationSize)));
}

void MyPipeline::drawAndUpdate(cv::Mat &input, std::vector<std::vector<cv::Point>> &output)
{
    std::vector<std::vector<cv::Point>> goodContours;
    cv::Mat copy = input.clone();
    std::vector<cv::Vec4i> hierarchy;
    findContours(copy, output, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> goodContour;

    int bestScore = 0;

    for (long unsigned i = 0; i < output.size(); i++)
    {
        int score = contourScore(output[i]);
        if (score > bestScore)
        {   
            //      if (contourArea(output[i]) >= 480)
            //      {
            goodContour = output[i];
            bestScore = score;
            //      }
        }
    }//for loop

    if (bestScore > 0)
    {
        std::vector<cv::Rect> boundRect(1);
        boundRect[0] = boundingRect(goodContour);

        boundingPoints[0] = boundRect[0].tl().x;
        boundingPoints[1] = boundRect[0].br().y;
        boundingPoints[2] = boundRect[0].br().x;
        boundingPoints[3] = boundRect[0].br().y;
    }
    else
    {
        wpi::outs() << "No contours, can't make a bounding box\n";
    }
}

/**
 * A method for evaluating the probability that a contour is the half-hexagon target.
 * 
 * Used for a selection sort to determine the best contour in drawAndUpdate(). If a hard-constraint
 * is not met, a negative score may be returned; if all contours score negative or zero, it will be
 * treated as if no contours were found.
 */
int MyPipeline::contourScore(std::vector<cv::Point> &contour)
{
    const int minArea = 500;
    return contourArea(contour) - minArea; //simple score, scaling directly with area
}

double MyPipeline::pxToDegrees(double pixel, int orientation, int imageWidth, int imageHeight) //0 for horizontal 1 for vertical
{
    if (orientation == 1) //vertical
    {
        return atan(pixel - (imageHeight / 2)) * (verticalFOV / imageHeight);
    }
    else if (orientation == 0) //horizontal
    {
        return atan(pixel - (imageWidth / 2)) * (horizontalFOV / imageWidth);
    }
    else
    {
        wpi::outs() << "pxToDegrees called incorrectly\n";
        return -1;
    }
}

void MyPipeline::trajectory(cv::Mat &input)
{
    double targetDistance = 0;       // dx in image
    double targetTheta = 0;          // Red theta in image
    double boundingBoxMidpointX = 0; // Midpoint of the bounding box on the X axis
    double imageMidpointX = 0;       // Midpoint of the image on the X axis
    double yawOffset = 0;            // Yaw offset in degrees

    int imageWidth = input.cols;  // image width
    int imageHeight = input.rows; // image height

    returnedDistance = getDistance(imageWidth, imageHeight);
    returnedHorizAngle = getHorizontalAngle(imageWidth, imageHeight);
}

double MyPipeline::getDistance(int imageWidth, int imageHeight)
{

    int camTheta = imageHeight - boundingPoints[3]; // Pixel distance between the bottom of the bounding box and bottom of the frame
    targetTheta = pxToDegrees(camTheta, 1, imageWidth, imageHeight) + angleOffset;
    targetDistance = targetHeight * atan(targetTheta);
    return targetDistance;
}

double MyPipeline::getHorizontalAngle(int imageWidth, int imageHeight)
{
    boundingBoxMidpointX = (boundingPoints[0] + boundingPoints[3]) / 2;
    imageMidpointX = imageWidth / 2; //this might break bc even # of pixels
    yawOffset = imageMidpointX - boundingBoxMidpointX;
    return pxToDegrees(yawOffset, 0, imageWidth, imageHeight);
}

void MyPipeline::sendLed(int r, int g, int b){
    ledString = std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
    wpi::outs() << "Settings LEDs to " << ledString << "\n";
    
    ledEntry.SetString(ledString);
}
