#pragma once
#include <SDL_scancode.h>
#include <vector>

class Input
{
public:
    bool update();

    bool pressed(SDL_Scancode sc) const;
    bool released(SDL_Scancode sc) const;
    bool held(SDL_Scancode sc) const;

    SDL_Scancode rangePressed(const std::vector<SDL_Scancode>& scancodes) const;
    SDL_Scancode rangeReleased(const std::vector<SDL_Scancode>& scancodes) const;
    SDL_Scancode rangeHeld(const std::vector<SDL_Scancode>& scancodes) const;

    SDL_Scancode keyPressed() const;
    SDL_Scancode keyReleased() const;
    SDL_Scancode keyHeld() const;

    bool mousePressed(Uint8 button) const;
    bool mouseReleased(Uint8 button) const;
    bool mouseHeld(Uint8 button) const;

private:
    bool mouseCurrent[8]{};
    bool mousePrevious[8]{};
    bool current[SDL_NUM_SCANCODES];
    bool previous[SDL_NUM_SCANCODES];
};