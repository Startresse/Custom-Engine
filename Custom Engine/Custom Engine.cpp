#include <vector>

#include "App.h"
#include "App1.h"

typedef unsigned int uint;

// Window dimensions
typedef std::pair<uint, uint> resolution_t;
const std::vector<resolution_t> standard_resolutions =
{
    {640, 480},
    {1024, 576},
    {1280, 720},
    {1366, 768},
    {1600, 900},
    {1920, 1080},
    {2560, 1440},
    {3840, 2160}
};


int main()
{
    uint resolution_type = 3;
    resolution_t resolution = standard_resolutions[resolution_type];

    App1 app(resolution.first, resolution.second);
    app.run();

    return 0;
}
