#include <gtkmm.h>

#include "GUI.hpp"
#include "CLI.hpp"
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

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--cli") == 0)
        {
            return CLIMain(argc, argv);
        }
    }

    ApplicationWindow appWin{};

    auto appData = std::make_shared<ApplicationData>();

    appWin.SetApplicationData(appData);
    appWin.ReadyWindow();

    return app->run(appWin);
}
