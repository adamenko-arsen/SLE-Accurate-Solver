#include <gtkmm.h>
#include <vector>

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv);

    Gtk::Window window;
    window.set_default_size(300, 200);

    Gtk::Grid grid;

    window.add(grid);

    // Create buttons
    Gtk::Button button1("Button 1");
    Gtk::Button button2("Button 2");
    Gtk::Button button3("Button 3");
    Gtk::Button button4("Button 4");

    // Add buttons to grid
    std::vector<Gtk::Button> buttons(4);

    buttons[0] = Gtk::Button("A");
    buttons[1] = Gtk::Button("B");
    buttons[2] = Gtk::Button("C");
    buttons[3] = Gtk::Button("D");

    grid.attach(buttons[0], 0, 0, 1, 1);
    grid.attach(buttons[1], 1, 0, 1, 1);
    grid.attach(buttons[2], 0, 1, 1, 1);
    grid.attach(buttons[3], 1, 1, 1, 1);

    // for (int i = 1; i < 4; i++)
    // {
    //     grid.remove(buttons[i]);
    // }

    buttons.clear();

    window.show_all();

    return app->run(window);
}