#include "header.h"

//#include <networktables/NetworkTableInstance.h>

int main(int argc, char **argv)
{
//    auto ntinst = nt::NetworkTableInstance::GetDefault();

//    wpi::outs() << "Setting up NetworkTables client for team " << team << '\n';
//    ntinst.StartClientTeam(team);

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
