#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include "Button.h"
#include <SDL.h>
#include <SDL_image.h>
#include <string>

class ImageButton : public Button {
public:
    ImageButton(const char* id, const char* imagePath, int x, int y, int w, int h)
        : Button(id, x, y, w, h), m_texture(nullptr), m_imagePath(imagePath) {}

    virtual ~ImageButton() {
        if (m_texture) { SDL_DestroyTexture(m_texture); m_texture = nullptr; }
    }

    void loadImage(SDL_Renderer* renderer) {
        if (m_texture) { SDL_DestroyTexture(m_texture); m_texture = nullptr; }
        // Enable linear filtering for smooth scaling
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_Surface* surf = IMG_Load(m_imagePath.c_str());
        if (surf) {
            m_texture = SDL_CreateTextureFromSurface(renderer, surf);
            if (m_texture)
                SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
            SDL_FreeSurface(surf);
        }
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    }

    virtual void draw(SDL_Renderer* renderer) override {
        SDL_Rect r = { m_x, m_y, m_w, m_h };
        SDL_Color fill   = {22, 35, 52, 255};
        SDL_Color border = {50, 80, 110, 255};
        // Draw rounded background manually
        int radius = 10;
        // Fill center cross
        SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_Rect center = {r.x + radius, r.y, r.w - 2*radius, r.h};
        SDL_RenderFillRect(renderer, &center);
        SDL_Rect left2  = {r.x, r.y + radius, radius, r.h - 2*radius};
        SDL_Rect right2 = {r.x + r.w - radius, r.y + radius, radius, r.h - 2*radius};
        SDL_RenderFillRect(renderer, &left2);
        SDL_RenderFillRect(renderer, &right2);
        for (int dy = 0; dy < radius; dy++) {
            int dx = (int)(sqrt((double)(radius*radius - dy*dy)));
            SDL_RenderDrawLine(renderer, r.x+radius-dx, r.y+radius-dy, r.x+radius, r.y+radius-dy);
            SDL_RenderDrawLine(renderer, r.x+r.w-radius, r.y+radius-dy, r.x+r.w-radius+dx, r.y+radius-dy);
            SDL_RenderDrawLine(renderer, r.x+radius-dx, r.y+r.h-radius+dy, r.x+radius, r.y+r.h-radius+dy);
            SDL_RenderDrawLine(renderer, r.x+r.w-radius, r.y+r.h-radius+dy, r.x+r.w-radius+dx, r.y+r.h-radius+dy);
        }
        // Border
        SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
        SDL_RenderDrawRect(renderer, &r); // approximate border

        // Draw image with padding and alpha blending
        if (m_texture) {
            SDL_Rect dest = { m_x + 4, m_y + 4, m_w - 8, m_h - 8 };
            SDL_RenderCopy(renderer, m_texture, nullptr, &dest);
        }
    }

private:
    SDL_Texture* m_texture;
    std::string  m_imagePath;
};

#endif