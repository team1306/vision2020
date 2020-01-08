#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
  // if you pass an arg (an image to analyze) then look at the image and not the
  // webcam

  bool usingImage = true;
  VideoCapture cap(0);

  Mat imgOriginal;
  Mat imgThresholded;
  Mat imgContoured;
  Mat imgThresholdedCanny;
  Mat imgHSV;
  Mat image;

  if (argc > 1) { // has at least one passed var (image)
    usingImage = true;
    image = imread(argv[1], IMREAD_COLOR); // Read the file
  } else {
    // no image passed, use camera feed
    usingImage = false;
    if (!cap.isOpened()) { // if not success, exit program{
      cout << "Cannot open the web cam" << endl;
      return -1;
    }
  }
  namedWindow("Control", WINDOW_NORMAL); // create a window called "Control"
  // set highs and lows for hsv
  int iLowH = 0;
  int iHighH = 179;
  int iLowS = 0;
  int iHighS = 255;
  int iLowV = 0;
  int iHighV = 255;
  int cannyThreshold = 100;

  // make a slider for each value for hsv
  createTrackbar("LowH", "Control", &iLowH, 179); // Hue (0 - 179)
  createTrackbar("HighH", "Control", &iHighH, 179);
  createTrackbar("LowS", "Control", &iLowS, 255); // Saturation (0 - 255)
  createTrackbar("HighS", "Control", &iHighS, 255);
  createTrackbar("LowV", "Control", &iLowV, 255); // Value (0 - 255)
  createTrackbar("HighV", "Control", &iHighV, 255);
  createTrackbar("Canny Threshold", "Control", &cannyThreshold, 255);

  vector<vector<Point>> contours;
  vector<Vec4i> hierarchy;
  RNG rng(0);

  while (true) {

    // set imgOriginal to the image that we started with or the current frame of
    // the camera
    if (usingImage) {
      imgOriginal = image.clone();
    } else {
      cap.read(imgOriginal); // read a new frame from video
    }

    // Convert the captured frame from BGR to HSV
    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

    // Threshold the image
    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV),
            imgThresholded);

    // morphological opening (remove small objects from the foreground)
    erode(imgThresholded, imgThresholded,
          getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
    dilate(imgThresholded, imgThresholded,
           getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

    // edge detection using the Canny edge detection alg
    Canny(imgThresholded, imgThresholdedCanny, cannyThreshold,
          cannyThreshold * 2);

    findContours(imgThresholdedCanny, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    cout << "Contours size: " << contours.size() << endl;
    for (long unsigned i = 0; i < contours.size(); i++) {
      imgContoured = imgThresholdedCanny.clone();
      Scalar color(255, 255, 255);
      drawContours(imgContoured, contours, (int) i, color, FILLED, LINE_8, hierarchy);
    }
    cout << "imgContoured is empty? " << imgContoured.empty() << endl;

    // show the thresholded image
    if(!imgContoured.empty()) imshow("Thresholded Image", imgContoured);

    // show the original image
    imshow("Original", imgOriginal);

    // quit when you press any key
    if (waitKey(30) > 0) {
      break;
    }

  } // end while true
  return 0;
}
