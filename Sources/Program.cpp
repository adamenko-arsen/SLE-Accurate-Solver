#include <gtkmm.h>

#include "GUI.hpp"

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "ua.kpi.ip-35.course1-term2.lse-solver");

    GUISession guiSession{};

    guiSession.Init();
    return app->run(guiSession.GetWindowRef());
}
