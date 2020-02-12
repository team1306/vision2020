#include <iostream>

#include "header.h"

using namespace std;
std::string ledString;

void sendLed(int r, int g, int b)
{
    ledString = std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
    cout << "Setting Leds to " << ledString << endl;
    try
    {
//       nt::NetworkTableInstance::GetInstance().GetTable("vision").PutString("status color", ledString);
    }
    catch (...)
    {
        sendLed(YELLOW); //Won't work bc case is reached when NT doesn't work
        cout << "Couldn't send LED over NT :(" << endl;
    }
}

void sendLed(const std::string &ledString)
{
    cout << "Setting Leds to " << ledString << endl;
    try
    {
//        nt::NetworkTableInstance::GetInstance().GetTable("vision").PutString("status color", ledString);
    }
    catch (...)
    {
        sendLed(YELLOW); //Won't work bc case is reached when NT doesn't work
        cout << "Couldn't send LED over NT :(" << endl;
    }
}

void sendTrajectoryValues(double heading, double distance)
{
    try
    {
    //    nt::NetworkTableInstance::GetInstance().GetTable("vision").PutNumber("heading", heading);
    //    nt::NetworkTableInstance::GetInstance().GetTable("vision").PutNumber("distance", distance);
    }
    catch (...)
    {
        sendLed(YELLOW);
        cout << "Can't send Trajectory values over NT :(" << endl;
    }
}
