#include "Color.h"

float Color::power() const
{
    return (r + g + b) / 3.0f;
}

Color black() { return Color(0, 0, 0); }
Color white() { return Color(1, 1, 1); }

Color red() { return Color(1, 0, 0); }
Color green() { return Color(0, 1, 0); }
Color blue() { return Color(0, 0, 1); }

Color yellow() { return Color(1, 1, 0); }
Color pink() { return Color(1, 0, 1); }
Color cyan() { return Color(0, 1, 1); }

Color grid_red() { return Color(0.619f, 0.235f, 0.290f); }
Color grid_green() { return Color(0.396f, 0.552f, 0.145f); }
Color grid_blue() { return Color(0.215f, 0.352f, 0.513f); }
Color grid_grey() { return Color(0.294f, 0.294f, 0.294f); }
