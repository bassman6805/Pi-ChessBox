#include "MarkPopup.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

static const char* LABELS[] = { "Return", "New Mark", "Clear" };

MarkPopup::MarkPopup(int screenW, int screenH, TTF_Font* font)
    : Component("markpopup", 0, 0, screenW, screenH)
    , m_font(font), m_visible(false), m_result(NONE)
    , m_screenW(screenW), m_screenH(screenH)
    , m_btnW(160), m_btnH(60), m_padding(16)
{
    m_popupW = m_btnW + m_padding * 2;
    m_popupH = 3 * m_btnH + 4 * m_padding + 40; // title + 3 buttons
    m_popupX = (screenW - m_popupW) / 2;
    m_popupY = (screenH - m_popupH) / 2;
}

SDL_Rect MarkPopup::buttonRect(int index) const {
    return { m_popupX + m_padding,
             m_popupY + 40 + m_padding + index * (m_btnH + m_padding),
             m_btnW, m_btnH };
}

void MarkPopup::show() {
    m_visible = true;
    m_result = NONE;
}

void MarkPopup::draw(SDL_Renderer* renderer) {
    if (!m_visible) return;

    // Background overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, m_screenW, m_screenH};
    SDL_RenderFillRect(renderer, &full);

    // Popup background
    SDL_SetRenderDrawColor(renderer, 22, 35, 52, 255);
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    SDL_RenderFillRect(renderer, &popup);
    SDL_SetRenderDrawColor(renderer, 100, 130, 180, 255);
    SDL_RenderDrawRect(renderer, &popup);

    if (!m_font) return;

    // Title
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* ts = TTF_RenderText_Blended(m_font, "Mark Options", white);
    if (ts) {
        SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
        SDL_Rect tr = {m_popupX + (m_popupW - ts->w)/2, m_popupY + 8, ts->w, ts->h};
        SDL_RenderCopy(renderer, tt, nullptr, &tr);
        SDL_DestroyTexture(tt);
        SDL_FreeSurface(ts);
    }

    // Buttons
    SDL_Color btnColors[3] = {
        {40, 120, 40, 255},   // Return - green
        {40, 80, 160, 255},   // New Mark - blue
        {160, 40, 40, 255}    // Clear - red
    };
    for (int i = 0; i < 3; i++) {
        SDL_Rect r = buttonRect(i);
        SDL_SetRenderDrawColor(renderer, btnColors[i].r, btnColors[i].g, btnColors[i].b, 255);
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &r);
        SDL_Surface* s = TTF_RenderText_Blended(m_font, LABELS[i], white);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect tr = {r.x + (r.w - s->w)/2, r.y + (r.h - s->h)/2, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &tr);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }
}

Component* MarkPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_FINGERDOWN) {
        int x, y;
        if (event->type == SDL_FINGERDOWN) {
            x = (int)(event->tfinger.x * m_screenW);
            y = (int)(event->tfinger.y * m_screenH);
        } else {
            x = event->button.x;
            y = event->button.y;
        }
        for (int i = 0; i < 3; i++) {
            SDL_Rect r = buttonRect(i);
            if (x >= r.x && x < r.x+r.w && y >= r.y && y < r.y+r.h) {
                m_result = (Result)i;
                m_visible = false;
                return this;
            }
        }
        // Click outside — dismiss
        m_visible = false;
    }
    return nullptr;
}
