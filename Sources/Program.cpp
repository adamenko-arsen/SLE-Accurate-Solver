#include <gtkmm.h>

#include "GUI.hpp"
#include "Convert.hpp"

#include <cmath>
#include <cstdint>
#include <string.h>

#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <vector>

#include <iostream>

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "ua.kpi.ip-35.course1-term2.lse-solver");

    GUISession guiSession{};

    guiSession.Init();
    return app->run(guiSession.GetWindowRef());
}
