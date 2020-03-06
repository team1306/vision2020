#include "MyPipeline.h"

const std::string cameraType = "lifecam_camera"; //changes fov and thesholding vars based on camera. Choices are 'lifecam_camera' 'pi_camera' and 'logitech_camera'

if(cameraType == "lifecam_camera"){
		//thresholding values, based on each camera
		const double MyPipeline::thresh_hue[] = {55, 63};
		const double MyPipeline::thresh_sat[] = {254, 255};
		const double MyPipeline::thresh_val[] = {33, 188};

		// Vertical and Horizontal FOV, based on each camera
		const double MyPipeline::verticalFOV = toRadians(36.2);                     // Vertical field of view of the camera
		const double MyPipeline::horizontalFOV = toRadians(64.4);                   // Horizontal field of view of the camera

		const double MyPipeline::angleOffset = toRadians(25); //angle offset, see example below 
}else if(cameraType == "pi_camera"){
		//thresholding values, based on each camera
		const double MyPipeline::thresh_hue[] = {55, 63};
		const double MyPipeline::thresh_sat[] = {254, 255};
		const double MyPipeline::thresh_val[] = {33, 188};

		// Vertical and Horizontal FOV, based on each camera
		const double MyPipeline::verticalFOV = toRadians(48.8);                     // Vertical field of view of the camera
		const double MyPipeline::horizontalFOV = toRadians(62.2);                   // Horizontal field of view of the camera

		const double MyPipeline::angleOffset = toRadians(25); //angle offset, see example below 
}else if(cameraType == "logitech_camera"){
		//thresholding values, based on each camera
		const double MyPipeline::thresh_hue[] = {55, 63};
		const double MyPipeline::thresh_sat[] = {254, 255};
		const double MyPipeline::thresh_val[] = {33, 188};

		// Vertical and Horizontal FOV, based on each camera
		const double MyPipeline::verticalFOV = toRadians(48.8);                     // Vertical field of view of the camera
		const double MyPipeline::horizontalFOV = toRadians(62.2);                   // Horizontal field of view of the camera

		const double MyPipeline::angleOffset = toRadians(25); //angle offset, see example below 
}else{
		//sets to pi camera values
		//thresholding values, based on each camera
		const double MyPipeline::thresh_hue[] = {55, 63};
		const double MyPipeline::thresh_sat[] = {254, 255};
		const double MyPipeline::thresh_val[] = {33, 188};

		// Vertical and Horizontal FOV, based on each camera
		const double MyPipeline::verticalFOV = toRadians(48.8);                     // Vertical field of view of the camera
		const double MyPipeline::horizontalFOV = toRadians(62.2);                   // Horizontal field of view of the camera

		const double MyPipeline::angleOffset = toRadians(25); //angle offset, see example below 
}


const double MyPipeline::targetToCameraHeight = 38.5;                       // dy in image - from target height to camera height (e.g. 50in)

const int MyPipeline::erosionSize = 0;
const int MyPipeline::dilationSize = 2;

/**
 *  \   60deg
 *   \
 *    \  <---camera tilted back 60deg from vertical... angleOffset = toRadians(60);
 *     \
 * 30deg\
 *  ---------
**/

MyPipeline::MyPipeline()
    : contourResults()
{
    robotHeading = nt::NetworkTableInstance::GetDefault().GetEntry("robot/heading");
    ledEntry = nt::NetworkTableInstance::GetDefault().GetEntry("vision/status color");
}

void MyPipeline::Process(cv::Mat &mat)
{
    size_t contours_count = -1;

    //grab heading for mat, defaulting to old heading
    imageCaptureHeading = robotHeading.GetDouble(imageCaptureHeading);

    // wpi::outs() << "Received frame with " << mat.channels() << " channels and size " << mat.cols << "x" << mat.rows << "\n";

    thresholdHSV(mat, mat, thresh_hue, thresh_sat, thresh_val);
    erodeSize(mat, mat, erosionSize);
    dilateSize(mat, mat, dilationSize);

    drawAndUpdate(mat, contourResults);

    trajectory(mat);

    contours_count = contourResults.size();
    wpi::outs() << "\n" << "Contours count: " << contours_count << "\n";
    if (contours_count == 1)
    {
        sendLed(0, 0, 255); //Blue
    }
    else if (contours_count > 1)
    {
        sendLed(255, 128, 255); //Purple
    }
    if (contours_count == 0)
    {
        sendLed(0, 0, 0); //Off
    }

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
    } //for loop

    if (bestScore > 0)
    {
        std::vector<cv::Rect> boundRect(1);
        boundRect[0] = boundingRect(goodContour);

        boundingPoints[0] = boundRect[0].tl().x;
        boundingPoints[1] = boundRect[0].tl().y;
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

double MyPipeline::pxToRadians(double pixel, int orientation, int imageWidth, int imageHeight) //0 for horizontal 1 for vertical
{
    if (orientation == 1) //vertical
    {
        return (pixel - (imageHeight / 2)) * (verticalFOV / imageHeight);
    }
    else if (orientation == 0) //horizontal
    {
        return (pixel - (imageWidth / 2)) * (horizontalFOV / imageWidth);
    }
    else
    {
        wpi::outs() << "pxToRadians called incorrectly\n";
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
    returnedHorizAngleDeg = toDegrees(getHorizontalAngleRad(imageWidth, imageHeight));

	wpi::outs() << "Distance: " << returnedDistance << "\n";
	wpi::outs() << "Angle: " << returnedHorizAngleDeg << "\n";
}

double MyPipeline::getDistance(int imageWidth, int imageHeight)
{
    int boxHeight = imageHeight - boundingPoints[3]; // Pixel distance between the bottom of the bounding box and bottom of the frame
    targetTheta = pxToRadians(boxHeight, 1, imageWidth, imageHeight) + angleOffset;
    if (tan(targetTheta) != 0)
    {
        targetDistance = targetToCameraHeight / tan(targetTheta); // TODO Catch 0 denominator
    }
    else
    {
        wpi::outs() << "tan(targetTheta) is equal to zero, this is not good\n";
    }
    return targetDistance;
}

double MyPipeline::toRadians(double deg)
{
    return deg * (PI / 180.0);
}

double MyPipeline::toDegrees(double rad)
{
    return rad * (180.0 / PI);
}

double MyPipeline::getHorizontalAngleRad(int imageWidth, int imageHeight)
{
    boundingBoxMidpointX = (boundingPoints[0] + boundingPoints[2]) / 2;
    imageMidpointX = imageWidth / 2.0;
    
    // yawOffset = imageMidpointX - boundingBoxMidpointX;
    yawOffset = boundingBoxMidpointX;

	return pxToRadians(yawOffset, 0, imageWidth, imageHeight);
}

void MyPipeline::sendLed(int r, int g, int b)
{
	//TODO: make this neater, right now we're just swapping the b and g since our leds take RBG not RGB
	ledString = std::to_string(r) + " " + std::to_string(b) + " " + std::to_string(g);
    wpi::outs() << "Settings LEDs to " << ledString << "\n";

    ledEntry.SetString(ledString);
}
