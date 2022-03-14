#include <vector>

#include "App.h"
#include "App1.h"

// Window dimensions
namespace resolution
{
    typedef std::pair<unsigned int, unsigned int> type;

    constexpr type r640 = { 640, 480 };
    constexpr type r1024 = { 1024, 576 };
    constexpr type r1280 = { 1280, 720 };
    constexpr type r1366 = { 1366, 768 };
    constexpr type r1600 = { 1600, 900 };
    constexpr type r1920 = { 1920, 1080 };
    constexpr type r2560 = { 2560, 1440 };
    constexpr type r3840 = { 3840, 2160 };
}


int main()
{
    resolution::type resolution = resolution::r1366;

    App1 app(resolution.first, resolution.second);
    app.run();

    return 0;
}
