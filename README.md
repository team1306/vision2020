# vision2020
Trying to get vision working for the 2020 season

Compiling using g++:
`g++ cv.cpp -g -Wall -Wextra -pedantic -o opencv -I/usr/include/opencv4 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs`

Use your webcam:
`./opencv`

Use an image:
`./opencv [image]`
