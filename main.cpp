#include "header.h"

int main(int argc, char **argv)
{
    try
    {
        image(argc, argv);
        calcAndSendTrajectory();
    }
    catch (...)
    {
        sendLed(RED); //LED ERROR MESSAGE - RED
    }
}