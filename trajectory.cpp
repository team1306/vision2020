#include <iostream>
#include <string>
#include <cmath>

#include "header.h"

using namespace std;

// header vars
// double boundingPoints[4];
// int imageWidth;
// int imageHeight;
//               |
//plug these in \/
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

//declare function
double getDistance();
double getHorizAngle();

void calcTrajectory()
{
    cout << "The target is " << getDistance() << " inches away" << endl;
    cout << "The target is " << getHorizAngle() << " degrees away from center" << endl;
}

double pxToDegrees(double pixel, int orientation) //0 for horizontal 1 for vertical
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
        cout << "pxToDegrees called incorrectly" << endl;
        return -1;
    }
}

double getDistance()
{
    camTheta = imageHeight - boundingPoints[3];
    targetTheta = pxToDegrees(camTheta, 1) + angleOffset;
    targetDistance = targetHeight * atan(targetTheta);
    return targetDistance;
}

double getHorizAngle()
{
    boundingBoxMidpointX = (boundingPoints[0] + boundingPoints[3]) / 2;
    imageMidpointX = imageWidth / 2; //this might break bc even # of pixels
    yawOffset = imageMidpointX - boundingBoxMidpointX;
    return pxToDegrees(yawOffset, 0);
}