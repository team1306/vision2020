#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
// #include <vision/VisionPipeline.h>

#include "header.h"

using namespace cv;
using namespace std;

// example pipeline
class MyPipeline
{
public:
    int val = 0;

    int iLowH = 65;
    int iHighH = 81;
    int iLowS = 0;
    int iHighS = 255;
    int iLowV = 40;
    int iHighV = 255;
    int erosionSize = 0;
    int dialationSize = 2;

    bool toErase = false;
    int areaMin = 30; // get rid of tiny noise
    int areaMax = 1000;
    int verticesMin = 25;
    int verticesMax = 5000;
    int aspectMin = 0;
    int aspectMax = 100;
    int contour_area = 0;
    int contour_verts = 0;
    int contour_aspect = 0;

    int epsilonModifier = .5;
    int epsilon = 0;

    vector<vector<Point>> contours;
    vector<vector<Point>> polyContours;
    vector<Vec4i> hierarchy;

    const double targetHeight = 78.5;  // dy in image - from target height to camera height (e.g. 500cm)
    const double angleOffset = 60.0;   // Angle that the camera is set to relative the ground (e.g. perpendicular to the ground is 90deg)
    const double verticalFOV = 48.8;   // Vertical field of view of the camera
    const double horizontalFOV = 62.2; // Horizontal field of view of the camera
    //local vars
    double targetDistance = 0;       // dx in image
    double targetTheta = 0;          // Red theta in image
    int camTheta = 0;                // Pixel distance between the bottom of the bounding box and bottom of the frame
    double boundingBoxMidpointX = 0; // Midpoint of the bounding box on the X axis
    double imageMidpointX = 0;       // Midpoint of the image on the X axis
    double yawOffset = 0;            // Yaw offset in degrees

    //returned vars
    double returnedDistance = 0;
    double returnedHorizontalAngle = 0;
    void Process(cv::Mat &mat)
    {

        //thresholding
        // Convert the captured frame from BGR to HSV
        cvtColor(mat, mat, COLOR_BGR2HSV);
        // Threshold the image
        inRange(mat, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), mat);
        //erode
        erode(mat, mat, getStructuringElement(MORPH_ELLIPSE, Size(2 * erosionSize + 1, 2 * erosionSize + 1), Point(erosionSize, erosionSize)));
        //dialate
        dilate(mat, mat, getStructuringElement(MORPH_ELLIPSE, Size(2 * dialationSize + 1, 2 * dialationSize + 1), Point(dialationSize, dialationSize)));

        //contour stuff
        Mat imgDraw = Mat::zeros(mat.size(), CV_8UC3);
        findContours(mat, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        // have to declare here so that contours.size != 0
        vector<Rect> boundRect(contours.size());
        polyContours = contours;

        for (long unsigned i = 0; i < contours.size(); i++)
        {
            cout << "Processing contour " << i + 1 << " out of " << contours.size() << endl;
            boundRect[i] = boundingRect(contours[i]);
            epsilon = (epsilonModifier / 10000) * arcLength(contours[i], true); //value 1-10000 / 100000 == 0.50 presicion
            approxPolyDP(contours[i], polyContours[i], epsilon, true);
            contour_area = contourArea(contours[i]);
            contour_verts = polyContours[i].size();
            contour_aspect = (boundRect[i].width / boundRect[i].height);
            toErase = true;
            if (contour_area > areaMin && contour_area < areaMax)
            {
                toErase = false;
            }
            else if (contour_verts > verticesMin && contour_verts < verticesMax)
            {
                toErase = false;
            }
            else if (contour_aspect / 100 > aspectMin && contour_aspect / 100 < aspectMax)
            { // dividing by 100 to make 1-100 turn into 0-1
                toErase = false;
            }
            else
            {
                toErase = true;
            }
            if (contours.size() == 0)
            {
                sendLed(MAGENTA); //LED ERROR MESSAGE - MAGENTA
            }
            else if (contours.size() == 1)
            {
                try
                { /* */
                    //                 *---*
                    //                 |   |
                    // (tl.x, br.y) -> *---* <- (br.x,br.y)
                    boundingPoints[0] = boundRect[0].tl().x;
                    boundingPoints[1] = boundRect[0].br().y;
                    boundingPoints[2] = boundRect[0].br().x;
                    boundingPoints[3] = boundRect[0].br().y;

                    imageWidth = imgDraw.size().width;
                    imageHeight = imgDraw.size().height;

                    cout << "Header variables set" << endl;

                    sendLed(GREEN); //LED ERROR MESSAGE - GREEN
                }
                catch (...)
                {
                    cout << "Exception caught while setting header variables" << endl;
                }
            }
            else if (contours.size() > 1)
            {
                sendLed(BLUE); //LED ERROR MESSAGE - BLUE

                cout << "Bounding rectangle points couldn't be sent because there is more than one contour" << endl;
            }
        }

        //math
        if (contours.size() != 1)
        {
            cout << "Can't do math, no contours" << endl;
        }
        else
        {
            camTheta = imageHeight - boundingPoints[3];
            targetTheta = pxToDegrees(camTheta, 1) + angleOffset;
            targetDistance = targetHeight * atan(targetTheta);
            returnedDistance = targetDistance;

            boundingBoxMidpointX = (boundingPoints[0] + boundingPoints[3]) / 2;
            imageMidpointX = imageWidth / 2; //this might break bc even # of pixels
            yawOffset = imageMidpointX - boundingBoxMidpointX;
            returnedHorizontalAngle = pxToDegrees(yawOffset, 0);
        }
        ++val;
    }
};