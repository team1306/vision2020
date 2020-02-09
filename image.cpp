#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "header.h"

using namespace cv;
using namespace std;

/*
 * TODO
 *  =>> Maybe mess with CHAIN_APPROX_SIMPLE
 *  =>> To erase case being hit for some reason. Changing hsv after contours generated might not be working as well as previously thought
 * BUGS
    =>> When running with a GUI, you have to close the GUI in order to run the calcuations.This is because
 * 
 * Compile using `g++ image.cpp -g -Wall -Wextra -pedantic -o opencv -I/usr/include/opencv4 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs`
 * Run using `./image <i or v> <path to image or video> <y or n>
 * E.g. ./image i image.jpg
 *      ./image v video.avi
 *      ./image 
 * 
 * Flags:
 * - i for image
 * - v for video
 * - y/n to run with/without gui
 */

bool usingImage = true;
VideoCapture cap(0);

//rest of the Mats
Mat imgOriginal;
Mat imgGreyscale[3];
Mat imgHSV;
Mat imageMat;
Mat goodContoursMat;
// Mat imgThresholded;
// Mat imgEroded;
// Mat imgDialated;
// Mat imgThresholdedCanny;

RNG rng(12345);
bool drawAndUpdateCalledBefore = false;
bool withGUI = false;
double boundingPoints[4];
int imageWidth;
int imageHeight;

int pos;
void *data;

// set highs and lows for hsv  - original reference images
// int iLowH = 65;
// int iHighH = 80;
// int iLowS = 27;
// int iHighS = 149;
// int iLowV = 211;
// int iHighV = 255;
// int erosionSize = 0;
// int dialationSize = 2;

// set highs and lows for hsv - underexposed pi camera images
int iLowH = 55;
int iHighH = 63;
int iLowS = 254;
int iHighS = 255;
int iLowV = 33;
int iHighV = 137;
int erosionSize = 0;
int dialationSize = 2;

bool toErase = false;
int areaMin = 480; // get rid of tiny noise
int areaMax = 15000;
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
vector<vector<Point>> lastContours;
vector<vector<Point>> goodContours;
vector<Vec4i> hierarchy;

//init the function here so it exists
void drawAndUpdate(int pos, void *data);
void shouldCallDrawAndUpdate(int pos, void *data);

int image(int argc, char **argv)
{
  if (argc == 4)
  {
    if (std::string(argv[3]) == "y")
    {
      withGUI = true;
    }
    else if (std::string(argv[3]) == "n")
    {
      withGUI = false;
    }
    else
    {
      cout << "specify if running with gui pls";
    }
    if (std::string(argv[1]) == "i")
    {
      //open file as image
      usingImage = true;
      imageMat = imread(argv[2], IMREAD_COLOR); // Read the file
    }
    else if (std::string(argv[1]) == "v")
    {
      //open file as video
      usingImage = false;
      cap = VideoCapture(argv[2]);
    }
    else
    {
      cout << "Please enter \"i\" or \"v\", for image or video as the second param"
           << endl;
    }
  }
  else if (argc == 2)
  {
    if (std::string(argv[3]) == "y")
    {
      withGUI = true;
    }
    else if (std::string(argv[3]) == "n")
    {
      withGUI = false;
    }
    else
    {
      cout << "specify if running with gui pls";
    }
    // no image passed, use camera feed
    usingImage = false;
    if (!cap.isOpened())
    { // if not success, exit program{
      cout << "Cannot open the web cam" << endl;
      return -1;
    }
  }
  else
  {
    cout << "Params not entered correcty" << endl;
  }
  if (withGUI)
  {
    namedWindow("Control", WINDOW_KEEPRATIO);

    // Trackbars for thresholding
    createTrackbar("LowH", "Control", &iLowH, 179, shouldCallDrawAndUpdate); // Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179, shouldCallDrawAndUpdate);
    createTrackbar("LowS", "Control", &iLowS, 255, shouldCallDrawAndUpdate); // Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255, shouldCallDrawAndUpdate);
    createTrackbar("LowV", "Control", &iLowV, 255, shouldCallDrawAndUpdate); // Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255, shouldCallDrawAndUpdate);

    // Trackbars for morphological operations
    createTrackbar("Dialate", "Control", &dialationSize, 2, shouldCallDrawAndUpdate);
    createTrackbar("Erode", "Control", &erosionSize, 2, shouldCallDrawAndUpdate);

    // Trackbar for epsilon (draw polydp)
    createTrackbar("Epsilon Modifier", "Control", &epsilonModifier, 10000, drawAndUpdate);

    // Trackbars for picking contours
    createTrackbar("Area Min", "Control", &areaMin, 15000, drawAndUpdate);
    createTrackbar("Area Max", "Control", &areaMax, 15000, drawAndUpdate);
    createTrackbar("Verticies Min", "Control", &verticesMin, 5000, drawAndUpdate);
    createTrackbar("Verticies Max", "Control", &verticesMax, 5000, drawAndUpdate);
    createTrackbar("Aspect Ratio Min", "Control", &aspectMin, 100, drawAndUpdate);
    createTrackbar("Aspect Ratio Max", "Control", &aspectMax, 100, drawAndUpdate);
  }
  if (withGUI)
  {
    while (true)
    {

      if (usingImage)
      {
        imgOriginal = imageMat.clone();
      }
      else
      {
        cap.read(imgOriginal); // read a new frame from video
      }

      // Convert the captured frame from BGR to HSV
      cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
      // Threshold the image
      inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgHSV);
      //erode
      erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(2 * erosionSize + 1, 2 * erosionSize + 1), Point(erosionSize, erosionSize)));
      //dialate
      dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(2 * dialationSize + 1, 2 * dialationSize + 1), Point(dialationSize, dialationSize)));

      if (!withGUI)
      {
        drawAndUpdate(pos, data);
      }

      // std::cout << "imgHSV empty? " << imgHSV.empty() << '\n';
      if (withGUI)
      {
        if (!imgHSV.empty())
        {
          namedWindow("Thresholded Image", WINDOW_KEEPRATIO);
          imshow("Thresholded Image", imgHSV);
        }
        // show the original image

        namedWindow("Original", WINDOW_KEEPRATIO);
        imshow("Original", imgOriginal);
      }
      // quit when you press esc
      if (waitKey(30) == 27)
      {
        break;
      }

    } // end while true
  }
  else if (!withGUI)
  {
    if (usingImage)
    {
      imgOriginal = imageMat.clone();
    }
    else
    {
      cap.read(imgOriginal); // read a new frame from video
    }

    // Convert the captured frame from BGR to HSV
    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
    // Threshold the image
    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgHSV);
    //erode
    erode(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(2 * erosionSize + 1, 2 * erosionSize + 1), Point(erosionSize, erosionSize)));
    //dialate
    dilate(imgHSV, imgHSV, getStructuringElement(MORPH_ELLIPSE, Size(2 * dialationSize + 1, 2 * dialationSize + 1), Point(dialationSize, dialationSize)));

    drawAndUpdate(pos, data);
  }
  return 0;
}

/**
 * Loops through all the contours, if they are within user defined range, draw them
 */
void drawAndUpdate(int pos, void *data)
{
  drawAndUpdateCalledBefore = true;
  //make the contoured mat cv_8uc3 in order to draw colored contours/boxes
  Mat imgDraw = Mat::zeros(imgHSV.size(), CV_8UC3);
  // Mat goodContoursMat = Mat::zeros(imgHSV.size(), CV_8U);
  findContours(imgHSV, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

  // have to declare here so that contours.size != 0
  vector<Rect> boundRect(contours.size());
  polyContours = contours;

  for (long unsigned i = 0; i < contours.size(); i++)
  {
    cout << "Processing contour " << i + 1 << " out of " << contours.size() << endl;
    Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));

    boundRect[i] = boundingRect(contours[i]);
    epsilon = (epsilonModifier / 10000) * arcLength(contours[i], true); //value 1-10000 / 100000 == 0.50 presicion
    approxPolyDP(contours[i], polyContours[i], epsilon, true);

    //defaulting to true might break it, fix if needed
    contour_area = contourArea(contours[i]);
    contour_verts = polyContours[i].size();
    contour_aspect = (boundRect[i].width / boundRect[i].height);
    // std::cout << "contours[]" << contours[i] << '\n'; //commented out for spee

    //print vars
    // cout << "contours.size()" << contours.size() << endl; //commented out for speed
    // cout << "Vertices: " << contour_verts << endl;
    // cout << "Area: " << contour_area << endl;

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

    //erasing
    if (!toErase)
    {
      if (withGUI)
      {
        // draw poly contour
        drawContours(imgDraw, polyContours, (int)i, color, FILLED, LINE_8, hierarchy);

        //draw minAreaRect
        rectangle(imgDraw, boundRect[i].tl(), boundRect[i].br(), color, 2);

        //draw centroid
        Point rectCenterPt = Point((boundRect[i].width * .5) + boundRect[i].x, (boundRect[i].height * .5) + boundRect[i].y);
        circle(imgDraw, rectCenterPt, 3, color - Scalar(40, 40, 40), 1, LINE_8);

        //draw vars on screen
        putText(imgDraw, "Area: " + std::to_string(contour_area), boundRect[i].tl() + Point(0, -5), FONT_HERSHEY_PLAIN, 2, color, 2, LINE_AA, false);
        putText(imgDraw, "Vertices: " + std::to_string(contour_verts), boundRect[i].tl() + Point(0, -30), FONT_HERSHEY_PLAIN, 2, color, 2, LINE_AA, false);

        //draw vertice points
        for (long unsigned j = 0; j < polyContours.size(); j++)
        {
          for (long unsigned k = 0; k < polyContours[j].size(); k++)
          {
            circle(imgDraw, polyContours[i][j], 3, Scalar(255, 20, 200), 1, LINE_8);
          }
        }
      }
    }
    else if (toErase)
    {
      std::cout << "toErase caseeeeee" << '\n';
      // contours.erase(contours.begin() + i);
    }

    //set bounding rect points... only works if there's 1 contour
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

    if (withGUI)
    {
      if (!imgDraw.empty())
      {
        namedWindow("BoundingRect + Midpoint + PolyDP", WINDOW_KEEPRATIO);
        imshow("BoundingRect + Midpoint + PolyDP", imgDraw);
      }
      // if(!goodContoursMat.empty()){imshow("Good Contours", goodContoursMat);}
      lastContours = contours;
    } //for loop
  }
}

//this needed to be implemented so that messing with hsv sliders would not start the contours pane
void shouldCallDrawAndUpdate(int pos, void *data)
{
  if (drawAndUpdateCalledBefore)
  {
    drawAndUpdate(pos, data); //I don't actually know what these vars are but I have to pass them into drawAndUpdate
  }
  else
  {
    cout << "drawAndUpdate should not be called" << endl;
  }
}
