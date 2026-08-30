#include "pch.hpp"
#include "AppWindow.hpp"
#include "GFX.hpp"
#include <SDL_ttf.h>
#include <iostream>


TTF_Font* AppScreen::getFont(int size)
{
    if (fonts.contains(size))
        return fonts[size];

    TTF_Font* font = TTF_OpenFont("Assets/font/wolfenstein.ttf", size);
    if (!font) {
        std::cout << "Font load failed: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    fonts[size] = font;
    return font;
}

bool AppScreen::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_ShowCursor(SDL_DISABLE);

    if (TTF_Init() == -1) {
        std::cout << "TTF_Init failed: " << TTF_GetError() << std::endl;
        SDL_Quit();   // important cleanup
        return false;
    }

    window = SDL_CreateWindow("My Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GFX::SCREEN_WIDTH, GFX::SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void AppScreen::destroy() {
    for (auto& [size, font] : fonts) {
        TTF_CloseFont(font);
    }
    fonts.clear();

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
}

void AppScreen::clear(SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}
void AppScreen::fade(Uint8 alpha) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);

    SDL_Rect fullscreen = { 0, 0, 1440, 960 };
    SDL_RenderFillRect(renderer, &fullscreen);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
void AppScreen::update() {

    SDL_RenderPresent(renderer);
}

void AppScreen::renderText(int x, int y, int fontSize,
    const std::string& text,
    const SDL_Color& color)
{
    TTF_Font* font = getFont(fontSize);
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cout << "TTF_RenderText failed: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cout << "CreateTexture failed: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dst = {
        x - surface->w / 2,
        y - surface->h / 2,
        surface->w,
        surface->h
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}




void AppScreen::blitTexture(SDL_Texture* tex, int x, int y) {
    SDL_Rect dst = { x, y, 0, 0 };
    SDL_QueryTexture(tex, nullptr, nullptr, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, tex, nullptr, &dst);
}
void AppScreen::blitTextureScale(SDL_Texture* tex, int x, int y, int w, int h) {
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    if (w == 0 && h == 0) {
        // No scaling at all
        dst.w = texW;
        dst.h = texH;
    }
    else if (w == 0) {
        // Scale width to preserve aspect ratio
        dst.h = h;
        dst.w = (texW * h) / texH;
    }
    else if (h == 0) {
        // Scale height to preserve aspect ratio
        dst.w = w;
        dst.h = (texH * w) / texW;
    }
    else {
        // Explicit scaling
        dst.w = w;
        dst.h = h;
    }

    SDL_RenderCopy(renderer, tex, nullptr, &dst);
}
void AppScreen::blitTextureScale(SDL_Texture* tex, const SDL_Rect& rect) {
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst;
    dst.x = rect.x;
    dst.y = rect.y;

    if (rect.w == 0 && rect.h == 0) {
        // No scaling at all
        dst.w = texW;
        dst.h = texH;
    }
    else if (rect.w == 0) {
        // Scale width to preserve aspect ratio
        dst.h = rect.h;
        dst.w = (texW * rect.h) / texH;
    }
    else if (rect.h == 0) {
        // Scale height to preserve aspect ratio
        dst.w = rect.w;
        dst.h = (texH * rect.w) / texW;
    }
    else {
        // Explicit scaling
        dst.w = rect.w;
        dst.h = rect.h;
    }

    SDL_RenderCopy(renderer, tex, nullptr, &dst);
}

void AppScreen::blitPixelsFromTextureScale(SDL_Texture* tex, const SDL_Rect& src, const SDL_Rect& dst, bool flipHor, bool flipVer) {
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst2;
    dst2.x = dst.x;
    dst2.y = dst.y;

    int w = dst.w;
    int h = dst.h;

    if (w == 0 && h == 0) {
        dst2.w = texW;
        dst2.h = texH;
    }
    else if (w == 0) {
        dst2.h = h;
        dst2.w = (texW * h) / texH;
    }
    else if (h == 0) {
        dst2.w = w;
        dst2.h = (texH * w) / texW;
    }
    else {
        dst2.w = w;
        dst2.h = h;
    }

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flipHor) flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);
    if (flipVer) flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

    SDL_RenderCopyEx(renderer, tex, &src, &dst2, 0.0, nullptr, flip);
}
void AppScreen::blitPixelsFromTextureScaleRotate(SDL_Texture* tex,const SDL_Rect& src,const SDL_Rect& dst,float angle,bool flipHor,bool flipVer)
{
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst2;
    dst2.x = dst.x;
    dst2.y = dst.y;

    int w = dst.w;
    int h = dst.h;

    // Handle automatic aspect ratio scaling
    if (w == 0 && h == 0) {
        dst2.w = texW;
        dst2.h = texH;
    }
    else if (w == 0) {
        dst2.h = h;
        dst2.w = (texW * h) / texH;
    }
    else if (h == 0) {
        dst2.w = w;
        dst2.h = (texH * w) / texW;
    }
    else {
        dst2.w = w;
        dst2.h = h;
    }

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (flipHor)
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_HORIZONTAL);

    if (flipVer)
        flip = (SDL_RendererFlip)(flip | SDL_FLIP_VERTICAL);

    // Rotate around center
    SDL_Point center = { dst2.w / 2, dst2.h / 2 };

    SDL_RenderCopyEx(
        renderer,
        tex,
        &src,
        &dst2,
        angle,
        &center,
        flip
    );
}

void AppScreen::blitTextureScaleCenter(SDL_Texture* tex, int x, int y, int w, int h) {
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    if (w == 0 && h == 0) {
        // No scaling at all
        dst.w = texW;
        dst.h = texH;
    }
    else if (w == 0) {
        // Scale width to preserve aspect ratio
        dst.h = h;
        dst.w = (texW * h) / texH;
    }
    else if (h == 0) {
        // Scale height to preserve aspect ratio
        dst.w = w;
        dst.h = (texH * w) / texW;
    }
    else {
        // Explicit scaling
        dst.w = w;
        dst.h = h;
    }
    dst = { x - dst.w / 2, y - dst.h / 2, dst.w, dst.h };
    blitTextureScale(tex, dst.x, dst.y, dst.w, dst.h);
}
void AppScreen::drawRect(int x, int y, int w, int h, const SDL_Color& color, bool fill, int outLineSize) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (fill) {
        SDL_Rect r{ x, y, w, h };
        SDL_RenderFillRect(renderer, &r);
    }
    else {
        for (int i = 0; i < outLineSize; i++) {
            SDL_Rect r = { x - i, y - i, w + 2 * i, h + 2 * i };
            SDL_RenderDrawRect(renderer, &r);
        }
    }
}
void AppScreen::drawRect(const SDL_Rect& dstRect, const SDL_Color& color, bool fill, int outLineSize) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    if (fill) {
        SDL_RenderFillRect(renderer, &dstRect);
    }
    else {
        for (int i = 0; i < outLineSize; i++) {
            SDL_Rect r = { dstRect.x - i, dstRect.y - i, dstRect.w + 2 * i, dstRect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &r);
        }
    }
}
void AppScreen::drawRect(const SDL_FRect& dstRect, const SDL_Color& color, bool fill, int outLineSize) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = { (int)dstRect.x,(int)dstRect.y,(int)dstRect.w,(int)dstRect.h };
    if (fill) {
        SDL_RenderFillRect(renderer, &rect);
    }
    else {
        for (int i = 0; i < outLineSize; i++) {
            SDL_Rect r = { rect.x - i, rect.y - i, rect.w + 2 * i, rect.h + 2 * i };
            SDL_RenderDrawRect(renderer, &r);
        }
    }
}
//===============
void AppScreen::blitTextureScaleCenterRotate(SDL_Texture* tex, int x, int y, int w, int h, float angle)
{
    int texW, texH;
    SDL_QueryTexture(tex, nullptr, nullptr, &texW, &texH);

    SDL_Rect dst;

    if (w == 0 && h == 0) {
        dst.w = texW;
        dst.h = texH;
    }
    else if (w == 0) {
        dst.h = h;
        dst.w = (texW * h) / texH;
    }
    else if (h == 0) {
        dst.w = w;
        dst.h = (texH * w) / texW;
    }
    else {
        dst.w = w;
        dst.h = h;
    }

    // Center the rectangle at (x, y)
    dst.x = x - dst.w / 2;
    dst.y = y - dst.h / 2;

    // Rotate around center of destination rect
    SDL_Point center = { dst.w / 2, dst.h / 2 };

    SDL_RenderCopyEx(renderer, tex, nullptr, &dst,
        angle, &center, SDL_FLIP_NONE);
}

