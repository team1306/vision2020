#include <string>

//Error colors and meanings
#define RED "255, 0, 0"       // Big error... not good
#define MAGENTA "255, 0, 255" // Target not found / searching
#define GREEN "0, 255, 0"     // Target Found
#define YELLOW "255, 255, 0"  // NT Error
#define BLUE "0, 0, 255"      // Found more than one contour
#define CYAN "0, 255, 255"    // Math error

extern double boundingPoints[4]; //make a 1d array to hold bounding points [(x1),(y1),(x2),(y2)]
extern int imageWidth;
extern int imageHeight;
extern std::string ledString;

int image(int argc, char **argv);
void calcAndSendTrajectory();
void sendLed(int r, int g, int b);
void sendLed(const std::string &ledString);
void sendTrajectoryValues(double heading, double distance);
double pxToDegrees(double pixel, int orientation);