#include "SDLWrapper.h"
#include "EventBus.h"
#include "InputSystem.h"
#include "RenderSystem.h"
#include "Events.h"
#include "GameTypes.h"
#include "Player.h"
#include "Assets.h"
#include "MapGen.h"
#include "Camera.h"
#include "Scene.h"
#include <memory>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#endif

double SDL_GetSecondsElapsed(int64_t old_counter, int64_t current_counter)
{
    return ((double)(current_counter - old_counter) / (double)(SDL_GetPerformanceFrequency()));
}

int main(int argc, char *argv[])
{
    srand(time(0));
#ifdef _WIN32
    if (timeBeginPeriod(1) == TIMERR_NOCANDO)
    {
        printf("Error calling timeBeginPeriod\n");
    }
    else
    {
        printf("Successfully set timer granularity to 1ms\n");
    }
#endif
    // initialize SDL
    SDLWrapper sdl;
    if (!sdl.initialize_SDL(800, 640))
    {
        printf("SDL failed to initialze.\n");
        return 1;
    }
    // Load assets
    Assets::load_assets_from_manifest(sdl.renderer, "assets/asset-manifest.txt");
    // initialize systems
    EventBus event_bus;
    RenderSystem render_system(sdl.renderer, &event_bus);
    InputSystem input_system(&event_bus);
    Color color = {0x11, 0x11, 0xFF, 0xFF};
    Player player(&event_bus, {0, 0}, color);
    // start game loop
    SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
    if (SDL_GetDisplayMode(0, 0, &mode) != 0)
    {
        SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
        return 1;
    }
    double ts = 1.f / 60.f;
    if (mode.refresh_rate > 0 && sdl.vsync)
    {
        ts = 1.f / (double)mode.refresh_rate;
    }
    printf("Initializing with ts: %f\n", ts);
    printf("Refresh rate: %d\n", mode.refresh_rate);
    int64_t last_counter = SDL_GetPerformanceCounter();

    // testing
    Rect camera = {0, 0, 800, 640};
    Camera::set_camera(camera);
    Scene scene(&event_bus);
    scene.load_from_file("assets/scene.txt");
    while (!input_system.quit)
    {

        event_bus.clear_input_events();
        input_system.collect_input_events();
        event_bus.notify_input_event_subscribers();

        event_bus.clear_render_events();
        player.update(ts);
        scene.update(ts);

        if (SDL_GetSecondsElapsed(last_counter, SDL_GetPerformanceCounter()) < ts)
        {
            int64_t time_to_sleep = ((ts - SDL_GetSecondsElapsed(last_counter, SDL_GetPerformanceCounter())) * 1000) - 1;
            if (time_to_sleep > 0)
            {
                SDL_Delay(time_to_sleep);
            }
            while (SDL_GetSecondsElapsed(last_counter, SDL_GetPerformanceCounter()) < ts)
            {
                // Waiting...
            }
        }
        int64_t end_counter = SDL_GetPerformanceCounter();

        event_bus.notify_render_event_subscribers(1.0);

        last_counter = end_counter;
    }
    return 0;
}