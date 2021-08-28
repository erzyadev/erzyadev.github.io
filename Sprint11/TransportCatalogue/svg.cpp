#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
        out << " r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);

        out << "/>"sv;
    }

    std::ostream &OstreamColorPrinter::operator()(std::monostate) const {
        return out << "none";
    }

    std::ostream &OstreamColorPrinter::operator()(const std::string &root) const {
        return out << root;
    }

    std::ostream &OstreamColorPrinter::operator()(svg::Rgb roots) const {
        return out << "rgb("sv << unsigned(roots.red) << ","sv << unsigned(roots.green) << "," << unsigned(roots.blue) << ")"sv;
    }

    std::ostream &OstreamColorPrinter::operator()(svg::Rgba roots) const {
        return out << "rgba("sv << unsigned(roots.red) << ","sv << unsigned(roots.green) << "," << unsigned(roots.blue) << ","sv << roots.opacity << ")"sv;
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineCap line_cap) {
        switch (line_cap)
        {
            case StrokeLineCap::BUTT:
                return out << "butt";
            case StrokeLineCap::ROUND:
                return out << "round";
            case StrokeLineCap::SQUARE:
                return out << "square";
            default:
                throw std::invalid_argument("Incorrect stroke-linecap parameter");
        }
    }

    std::ostream &operator<<(std::ostream &out, StrokeLineJoin line_join) {
        switch (line_join)
        {
            case StrokeLineJoin::ARCS:
                return out << "arcs";
            case StrokeLineJoin::BEVEL:
                return out << "bevel";
            case StrokeLineJoin::MITER:
                return out << "miter";
            case StrokeLineJoin::MITER_CLIP:
                return out << "miter-clip";
            case StrokeLineJoin::ROUND:
                return out << "round";
            default:
                throw std::invalid_argument("Incorrect stroke-linejoin parameter");
        }
    }

    Polyline &Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<polyline points=\"";
        if (!points_.empty())
        {
            out << points_[0];
            for (size_t i = 1; i < points_.size(); ++i)
            {
                out << ' ' << points_[i];
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>";
    }

    Text &Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text &Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text &Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text &Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text &Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text &Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext &context) const {
        auto &out = context.out;
        out << "<text";
        RenderAttrs(context.out);

        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\"";
        out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";

        out << " font-size=\"" << size_ << "\"";

        if (!font_family_.empty())
            out << " font-family=\"" << font_family_ << "\"";
        if (!font_weight_.empty())
            out << " font-weight=\"" << font_weight_ << "\"";
        out << '>';
        out << TextConverter(data_);
        out << "</text>";
    }

    void Document::AddPtr(std::unique_ptr<Object> &&obj) {
        object_ptrs_.push_back(move(obj));
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (auto &object_ptr : object_ptrs_)
        {
            object_ptr->Render(RenderContext{out});
        }

        out << "</svg>"sv;
    }
} // namespace svg
void shapes::Triangle::Draw(svg::ObjectContainer &container) const {
    container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

void shapes::Star::Draw(svg::ObjectContainer &container) const {
    container.Add(CreateStar());
}

svg::Polyline shapes::Star::CreateStar() const {
    using namespace svg;
    Polyline polyline;
    for (int i = 0; i <= num_rays_; ++i)
    {
        double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});
        if (i == num_rays_)
        {
            break;
        }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
    }
    std::string fill = "red";
    std::string stroke = "black";
    return polyline.SetFillColor(fill).SetStrokeColor(stroke);
}

void shapes::Snowman::Draw(svg::ObjectContainer &container) const {
    std::string fill = "rgb(240,240,240)";
    std::string stroke = "black";

    svg::Point bottom_center = svg::Point{head_center_.x, head_center_.y + 5 * head_radius_};
    double bottom_radius = 2 * head_radius_;
    container.Add(svg::Circle().SetCenter(bottom_center).SetRadius(bottom_radius).SetFillColor(fill).SetStrokeColor(stroke));

    svg::Point middle_center = svg::Point{head_center_.x, head_center_.y + 2 * head_radius_};
    double middle_radius = 1.5 * head_radius_;
    container.Add(svg::Circle().SetCenter(middle_center).SetRadius(middle_radius).SetFillColor(fill).SetStrokeColor(stroke));

    container.Add(svg::Circle().SetCenter(head_center_).SetRadius(head_radius_).SetFillColor(fill).SetStrokeColor(stroke));
}
