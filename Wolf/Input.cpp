#include "pch.hpp"
#include "Input.hpp"
#include "SDL_events.h"

bool Input::update()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            return false;
    }


    // Keyboard
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        previous[i] = current[i];
        current[i] = keys[i] != 0;
    }


    // Mouse buttons
    for (int i = 0; i < 8; ++i)
        mousePrevious[i] = mouseCurrent[i];

    Uint32 mouse = SDL_GetMouseState(nullptr, nullptr);

    mouseCurrent[SDL_BUTTON_LEFT] =
        mouse & SDL_BUTTON(SDL_BUTTON_LEFT);

    mouseCurrent[SDL_BUTTON_RIGHT] =
        mouse & SDL_BUTTON(SDL_BUTTON_RIGHT);


    return true;
}

bool Input::mousePressed(Uint8 button) const
{
    return !mousePrevious[button] && mouseCurrent[button];
}

bool Input::mouseReleased(Uint8 button) const
{
    return mousePrevious[button] && !mouseCurrent[button];
}

bool Input::mouseHeld(Uint8 button) const
{
    return mouseCurrent[button];
}
bool Input::pressed(SDL_Scancode sc) const
{
    return !previous[sc] && current[sc];
}
bool Input::released(SDL_Scancode sc) const
{
    return previous[sc] && !current[sc];
}
bool Input::held(SDL_Scancode sc) const
{
    return current[sc];
}
SDL_Scancode Input::rangePressed(const std::vector<SDL_Scancode>& scancodes) const
{
    for (auto sc : scancodes)
        if (pressed(sc))
            return sc;

    return SDL_SCANCODE_UNKNOWN;
}
SDL_Scancode Input::rangeReleased(const std::vector<SDL_Scancode>& scancodes) const
{
    for (auto sc : scancodes)
        if (released(sc))
            return sc;

    return SDL_SCANCODE_UNKNOWN;
}
SDL_Scancode Input::rangeHeld(const std::vector<SDL_Scancode>& scancodes) const
{
    for (auto sc : scancodes)
        if (held(sc))
            return sc;

    return SDL_SCANCODE_UNKNOWN;
}
SDL_Scancode Input::keyPressed() const
{
    for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc)
        if (!previous[sc] && current[sc])
            return static_cast<SDL_Scancode>(sc);

    return SDL_SCANCODE_UNKNOWN;
}
SDL_Scancode Input::keyReleased() const
{
    for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc)
            return static_cast<SDL_Scancode>(sc);

    return SDL_SCANCODE_UNKNOWN;
}
SDL_Scancode Input::keyHeld() const
{
    for (int sc = 0; sc < SDL_NUM_SCANCODES; ++sc)
        if (current[sc])
            return static_cast<SDL_Scancode>(sc);

    return SDL_SCANCODE_UNKNOWN;
}