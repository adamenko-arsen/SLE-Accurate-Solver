#include <gtkmm.h>
#include <iostream>
#include <optional>

std::optional<std::pair<double, double>> IntersectionOfLines(double a1, double b1, double c1, double a2, double b2, double c2) {
    double determinant = a1 * b2 - a2 * b1;

    if (determinant == 0) {
        // Lines are parallel, no intersection
        return std::nullopt;
    }

    double x = (c1 * b2 - c2 * b1) / determinant;
    double y = (a1 * c2 - a2 * c1) / determinant;

    return std::pair{x, y};
}

class TwoEquationsCanvas : public Gtk::DrawingArea {
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override
    {
        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();

        double center_x = width / 2.0;
        double center_y = height / 2.0;

        // Draw X and Y axes
        cr->set_line_width(2.0);
        cr->set_source_rgb(0.0, 1.0, 0.0);

        cr->move_to(0.0, center_y); // Move to the center of Y axis
        cr->line_to(width, center_y); // Draw X axis

        cr->move_to(center_x, 0.0); // Move to the center of X axis
        cr->line_to(center_x, height); // Draw Y axis
        cr->stroke();

        // Draw the first line (Ax + By = C)
        double A1 = 2.0;
        double B1 = 3.0;
        double C1 = 500.0;

        double A2 = -1.0;
        double B2 = 2.0;
        double C2 = 500.0;

        double m1 = -A1 / B1;
        double b1 = C1 / B1;

        double m2 = -A2 / B2;
        double b2 = C2 / B2;

        double max_y = std::max(
              std::max(std::abs(-center_x * m1 + b1), std::abs(center_x * m1 + b1))
            , std::max(std::abs(-center_x * m1 + b2), std::abs(center_x * m2 + b2))
        );

        double koeff = std::min(width, height) / max_y / 3;

        // Draw the first line
        cr->set_source_rgb(1.0, 0.0, 0.0);
        cr->move_to(0, center_y - (-center_x * m1 + b1) * koeff);
        cr->line_to(width, center_y - (center_x * m1 + b1) * koeff);
        cr->stroke();

        // Draw the second line
        cr->set_source_rgb(0.0, 0.0, 1.0);
        cr->move_to(0, center_y - (-center_x * m2 + b2) * koeff);
        cr->line_to(width, center_y - (center_x * m2 + b2) * koeff);
        cr->stroke();

        // Draw X and Y axises text.
        cr->set_font_size(18);
        cr->move_to(center_x + 16, 18);

        cr->set_source_rgb(1, 1, 1);
        cr->show_text("X");

        cr->move_to(width - 18, center_y + 32);
        cr->show_text("Y");

        auto inters = IntersectionOfLines(A1, B1, C1, A2, B2, C2).value();

        // Draw coordinate
        cr->set_font_size(12);
        cr->move_to(center_x - inters.first * koeff, center_y - inters.second * koeff);
        cr->show_text("(x=" + std::to_string(inters.first) + " y=" + std::to_string(inters.second) + ")");

        return true;
    }
};

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create(argc, argv);

    Gtk::Window window;
    window.set_default_size(400, 400);

    TwoEquationsCanvas drawing_area;
    window.add(drawing_area);
    drawing_area.show();

    return app->run(window);
}