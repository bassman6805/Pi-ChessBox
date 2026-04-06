#include "ClockPopup.h"
#include <cstdio>
#include <SDL2/SDL.h>

// Presets: {time minutes, increment seconds, label}
static const int   PRESET_MINS[]  = {  1,  3,  5, 10, 15, 30,  0 };
static const int   PRESET_INCS[]  = {  0,  2,  3,  5, 10,  0,  0 };
static const char* PRESET_LABELS[]= {"1+0","3+2","5+3","10+5","15+10","30+0","Off"};

int ClockPopup::presetTimeMs(int index) {
    if (index < 0 || index >= NUM_PRESETS) return 10*60*1000;
    return PRESET_MINS[index] * 60 * 1000;
}

int ClockPopup::presetIncrementSec(int index) {
    if (index < 0 || index >= NUM_PRESETS) return 5;
    return PRESET_INCS[index];
}

const char* ClockPopup::presetLabel(int index) {
    if (index < 0 || index >= NUM_PRESETS) return "?";
    return PRESET_LABELS[index];
}

ClockPopup::ClockPopup(int screenW, int screenH, TTF_Font* font)
    : Component("clockpopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_selectedIndex(-1),
      m_currentIndex(3), m_screenW(screenW), m_screenH(screenH) {

    m_padding = 10;
    m_btnW    = 180;
    m_btnH    = 50;

    m_popupW = COLS * m_btnW + (COLS + 1) * m_padding;
    m_popupH = ROWS * m_btnH + (ROWS + 1) * m_padding + 50; // +50 for title

    m_popupX = (screenW - m_popupW) / 2;
    m_popupY = (screenH - m_popupH) / 2;
}

void ClockPopup::show(int currentIndex) {
    m_currentIndex  = currentIndex;
    m_selectedIndex = -1;
    m_visible       = true;
}

SDL_Rect ClockPopup::buttonRect(int index) const {
    int col = index % COLS;
    int row = index / COLS;
    int x = m_popupX + m_padding + col * (m_btnW + m_padding);
    int y = m_popupY + 50 + m_padding + row * (m_btnH + m_padding);
    return {x, y, m_btnW, m_btnH};
}

void ClockPopup::draw(SDL_Renderer* renderer) {
    if (!m_visible) return;

    // Dim background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, m_screenW, m_screenH};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Popup background
    SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255);
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    SDL_RenderFillRect(renderer, &popup);

    // Title bar
    SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
    SDL_Rect titleBar = {m_popupX, m_popupY, m_popupW, 44};
    SDL_RenderFillRect(renderer, &titleBar);

    // Title text
    if (m_font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, "Select Clock", white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect dst = {m_popupX + m_popupW/2 - surf->w/2, m_popupY + 10, surf->w, surf->h};
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    // Preset buttons
    for (int i = 0; i < NUM_PRESETS; i++) {
        SDL_Rect btn = buttonRect(i);
        bool isCurrent = (i == m_currentIndex);

        if (isCurrent) {
            SDL_SetRenderDrawColor(renderer, 180, 230, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 22, 35, 52, 255);
        }
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
        SDL_RenderDrawRect(renderer, &btn);

        if (m_font) {
            SDL_Color textColor = isCurrent ? SDL_Color{10, 14, 20, 255} : SDL_Color{200, 220, 240, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(m_font, PRESET_LABELS[i], textColor);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int tx = btn.x + (btn.w - surf->w) / 2;
                    int ty = btn.y + (btn.h - surf->h) / 2;
                    SDL_Rect dst = {tx, ty, surf->w, surf->h};
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }
    }

    // Popup border
    SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
    SDL_RenderDrawRect(renderer, &popup);
}

Component* ClockPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x;
    int my = event->button.y;

    for (int i = 0; i < NUM_PRESETS; i++) {
        SDL_Rect btn = buttonRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w &&
            my >= btn.y && my < btn.y + btn.h) {
            m_selectedIndex = i;
            m_currentIndex  = i;
            m_visible = false;
            return this;
        }
    }

    // Click outside = dismiss
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    if (mx < popup.x || mx >= popup.x + popup.w ||
        my < popup.y || my >= popup.y + popup.h) {
        m_visible = false;
    }
    return nullptr;
}
