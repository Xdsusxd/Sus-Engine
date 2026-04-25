#include "core/Engine.h"

int main() {
    Engine::EngineConfig config;
    config.window.title  = "FY-Engine";
    config.window.width  = 1280;
    config.window.height = 720;
    config.logFile       = "fyengine.log";

    Engine::EngineApp app;

    if (!app.Init(config)) {
        return 1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}
