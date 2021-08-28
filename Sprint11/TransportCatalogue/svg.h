#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#define _USE_MATH_DEFINES
#include <cmath>
#include <variant>

using namespace std::literals;
namespace svg
{

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x), y(y)
        {
        }
        double x = 0;
        double y = 0;
    };

    inline std::ostream &operator<<(std::ostream &out, Point p)
    {
        return out << p.x << ',' << p.y;
    }
    /*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
    struct RenderContext
    {
        RenderContext(std::ostream &out)
            : out(out)
        {
        }

        RenderContext(std::ostream &out, int indent_step, int indent = 0)
            : out(out), indent_step(indent_step), indent(indent)
        {
        }

        RenderContext Indented() const
        {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba
    {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
            : red(red), green(green), blue(blue), opacity(opacity) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1;
    };
    using Color = std::variant<std::monostate, Rgb, Rgba, std::string>;
    inline const auto NoneColor = std::monostate{};

    struct OstreamColorPrinter
    {
        std::ostream &out;
        std::ostream &operator()(std::monostate) const;
        std::ostream &operator()(const std::string &root) const;
        std::ostream &operator()(svg::Rgb roots) const;

        std::ostream &operator()(svg::Rgba roots) const;
    };
    /*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
    class Object
    {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::ostream &operator<<(std::ostream &out, StrokeLineCap line_cap);
    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
     std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join);

    template <typename Owner>
    class PathProps
    {
    public:
        Owner &SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner &SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner &SetStrokeWidth(double stroke_width)
        {
            stroke_width_ = stroke_width;
            return AsOwner();
        }
        Owner &SetStrokeLineCap(StrokeLineCap line_cap)
        {
            line_cap_ = line_cap;
            return AsOwner();
        }
        Owner &SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream &out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv;
                std::visit(OstreamColorPrinter{out}, *fill_color_) << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv;
                std::visit(OstreamColorPrinter{out}, *stroke_color_) << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_)
            {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_)
            {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner &AsOwner()
        {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner &>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };
    /*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
    class Polyline : public Object, public PathProps<Polyline>
    {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);

        void RenderObject(const RenderContext &context) const override;
        /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
    private:
        std::vector<Point> points_;
    };

    /*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
    class Text : public Object, public PathProps<Text>
    {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);
        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);
        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t size);
        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);
        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);
        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);
        // Прочие данные и методы, необходимые для реализации элемента <text>
        void RenderObject(const RenderContext &context) const override;

    private:
        Point pos_ = Point{};
        Point offset_ = Point{};
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;

        struct TextConverter
        {
            const std::string &data;
            TextConverter(const std::string &data) : data(data)
            {
            }
            friend std::ostream &operator<<(std::ostream &out, TextConverter &&converter)
            {
                for (char c : converter.data)
                {
                    switch (c)
                    {
                    case '\"':
                        out << "&quot;";
                        break;
                    case '\'':
                        out << "&apos;";
                        break;
                    case '<':
                        out << "&lt;";
                        break;
                    case '>':
                        out << "&gt;";
                        break;
                    case '&':
                        out << "&amp;";
                        break;
                    default:
                        out << c;
                    }
                }
                return out;
            }
        };
    };

    class ObjectContainer
    {
    public:
        template <typename ObjectType>
        void Add(const ObjectType &object)
        {
            AddPtr(std::unique_ptr<Object>(std::make_unique<ObjectType>(object)));
        }
        virtual void AddPtr(std::unique_ptr<Object> &&obj) = 0;
    };

    class Document : public ObjectContainer
    {
    public:
        /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> object_ptrs_;
    };

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer &container) const = 0;
        virtual ~Drawable() = default;
    };
} // namespace svg

namespace shapes
{

    class Triangle : public svg::Drawable
    {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1), p2_(p2), p3_(p3)
        {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable
    { /* Реализуйте самостоятельно */
    public:
        Star(svg::Point center, double outer_radius, double inner_radius, int num_rays)
            : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {}

        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point center_;
        double outer_radius_;
        double inner_radius_;
        int num_rays_;
        svg::Polyline CreateStar() const;
    };

    class Snowman : public svg::Drawable
    {
    public:
        Snowman(svg::Point head_center, double head_radius)
            : head_center_(head_center), head_radius_(head_radius) {}
        void Draw(svg::ObjectContainer &container) const override;

    private:
        svg::Point head_center_;
        double head_radius_;
    };

} // namespace shapes