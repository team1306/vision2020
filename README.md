# vision2020
Trying to get vision working for the 2020 season

## Compiling image.cpp and trajectory.cpp
`g++ image.cpp trajectory.cpp -g -Wall -Wextra -pedantic -o vision -I/usr/include/opencv4 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs`

Running image.cpp and trajectory.cpp:
`./vision <image or video> <path to image or video> [Run with GUI]`>

## Compiling image.cpp using g++:
`g++ image.cpp trajectory.cpp -g -Wall -Wextra -pedantic -o image -I/usr/include/opencv4 -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs`

Syntax:
`./image <image or video> <path to image or video> [Run with GUI]`

Running image.cpp using your webcam:
`./image [y/n]`

Running image.cpp using an image or video:
`./image <i/v> <image> [y/n]`

## Compiling trajectory.cpp using g++:
`g++ trajectory.cpp -g -Wall -Wextra -pedantic -o trajectory`

Running trajectory.cpp:
`./trajectory`