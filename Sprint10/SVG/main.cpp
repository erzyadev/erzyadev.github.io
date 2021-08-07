#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>

using namespace std::literals;
using namespace svg;
using namespace shapes;
/*
Пример использования библиотеки. Он будет компилироваться и работать, когда вы реализуете
все классы библиотеки.
*/

namespace
{

    // Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays)
    // {
    //     Polyline polyline;
    //     for (int i = 0; i <= num_rays; ++i)
    //     {
    //         double angle = 2 * M_PI * (i % num_rays) / num_rays;
    //         polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
    //         if (i == num_rays)
    //         {
    //             break;
    //         }
    //         angle += M_PI / num_rays;
    //         polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
    //     }
    //     return polyline;
    // }

    // // Выводит приветствие, круг и звезду
    // void DrawPicture()
    // {
    //     Document doc;
    //     doc.Add(Circle().SetCenter({20, 20}).SetRadius(10));
    //     doc.Add(Text()
    //                 .SetFontFamily("Verdana"s)
    //                 .SetPosition({35, 20})
    //                 .SetOffset({0, 6})
    //                 .SetFontSize(15)
    //                 .SetFontWeight("bold"s)
    //                 .SetData("Hello C++ <&/"s));
    //     doc.Add(CreateStar({20, 50}, 10, 5, 5));
    //     doc.Render(std::cout);
    // }

} // namespace
template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer &target)
{
    for (auto it = begin; it != end; ++it)
    {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container &container, svg::ObjectContainer &target)
{
    using namespace std;
    DrawPicture(begin(container), end(container), target);
} // Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t
uint8_t Lerp(uint8_t from, uint8_t to, double t)
{
    return static_cast<uint8_t>(std::round((to - from) * t + from));
}

// Выполняет линейную интерполяцию Rgb цвета от from до to в зависимости от параметра t
svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t)
{
    return {Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t)};
}

int main()
{
    using namespace svg;
    using namespace std;

    Rgb start_color{0, 255, 30};
    Rgb end_color{20, 20, 150};

    const int num_circles = 10;
    Document doc;
    for (int i = 0; i < num_circles; ++i)
    {
        const double t = double(i) / (num_circles - 1);

        const Rgb fill_color = Lerp(start_color, end_color, t);

        doc.Add(Circle()
                    .SetFillColor(fill_color)
                    .SetStrokeColor("black"s)
                    .SetCenter({i * 20.0 + 40, 40.0})
                    .SetRadius(15));
    }
    doc.Render(cout);
}