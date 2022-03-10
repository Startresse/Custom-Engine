#include "Color.h"

float Color::power() const
{
    return (x + y + z) / 3.0f;
}

Color Color::black()        { return Color(0.0f, 0.0f, 0.0f); }
Color Color::white()        { return Color(1.0f, 1.0f, 1.0f); }

Color Color::red()          { return Color(1.0f, 0.0f, 0.0f); }
Color Color::green()        { return Color(0.0f, 1.0f, 0.0f); }
Color Color::blue()         { return Color(0.0f, 0.0f, 1.0f); }

Color Color::yellow()       { return Color(1.0f, 1.0f, 0.0f); }
Color Color::pink()         { return Color(1.0f, 0.0f, 1.0f); }
Color Color::cyan()         { return Color(0.0f, 1.0f, 1.0f); }

Color Color::grid_red()     { return Color(0.619f, 0.235f, 0.290f); }
Color Color::grid_green()   { return Color(0.396f, 0.552f, 0.145f); }
Color Color::grid_blue()    { return Color(0.215f, 0.352f, 0.513f); }
Color Color::grid_grey()    { return Color(0.294f, 0.294f, 0.294f); }
