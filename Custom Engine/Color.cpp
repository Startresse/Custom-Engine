#include "Color.h"

float Color::power() const
{
    return (x + y + z) / 3.0f;
}

const Color Color::black = Color(0.0f, 0.0f, 0.0f);
const Color Color::white = Color(1.0f, 1.0f, 1.0f);

const Color Color::red = Color(1.0f, 0.0f, 0.0f);
const Color Color::green = Color(0.0f, 1.0f, 0.0f);
const Color Color::blue = Color(0.0f, 0.0f, 1.0f);

const Color Color::yellow = Color(1.0f, 1.0f, 0.0f);
const Color Color::pink = Color(1.0f, 0.0f, 1.0f);
const Color Color::cyan = Color(0.0f, 1.0f, 1.0f);

const Color Color::grid_red = Color(0.619f, 0.235f, 0.290f);
const Color Color::grid_green = Color(0.396f, 0.552f, 0.145f);
const Color Color::grid_blue = Color(0.215f, 0.352f, 0.513f);
const Color Color::grid_grey = Color(0.294f, 0.294f, 0.294f);

const Color Color::background_grey = Color(0.212f, 0.212f, 0.212f);
