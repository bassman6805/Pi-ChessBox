#include "TimePopup.h"
#include <cstdio>
#include <SDL2/SDL.h>

const int TimePopup::PRESETS[NUM_PRESETS]       = { 500, 1000, 2000, 3000, 5000, 10000, 0 };
const char* TimePopup::LABELS[NUM_PRESETS]      = { "0.5s", "1s", "2s", "3s", "5s", "10s", "None" };

TimePopup::TimePopup(int screenW, int screenH, TTF_Font* font)
    : Component("timepopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_selectedMs(-1),
      m_currentMs(0), m_screenW(screenW), m_screenH(screenH) {

    m_padding = 8;
    m_btnW = 90;
    m_btnH = 40;

    int rows = (NUM_PRESETS + COLS - 1) / COLS;
    m_popupW = COLS * m_btnW + (COLS + 1) * m_padding;
    m_popupH = rows * m_btnH + (rows + 1) * m_padding + 40;

    m_popupX = (screenW - m_popupW) / 2;
    m_popupY = (screenH - m_popupH) / 2;
}

void TimePopup::show(int currentMs) {
    m_currentMs = currentMs;
    m_selectedMs = -1;
    m_visible = true;
}

SDL_Rect TimePopup::buttonRect(int index) const {
    int col = index % COLS;
    int row = index / COLS;
    int x = m_popupX + m_padding + col * (m_btnW + m_padding);
    int y = m_popupY + 40 + m_padding + row * (m_btnH + m_padding);
    return {x, y, m_btnW, m_btnH};
}

void TimePopup::draw(SDL_Renderer* renderer) {
    if (!m_visible) return;

    // Dim background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, m_screenW, m_screenH};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Popup background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    SDL_RenderFillRect(renderer, &popup);

    // Popup border
    SDL_SetRenderDrawColor(renderer, 102, 217, 0, 255);
    SDL_RenderDrawRect(renderer, &popup);

    // Title
    if (m_font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, "Max Think Time", white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect dst = {m_popupX + m_padding, m_popupY + 8, surf->w, surf->h};
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    // Preset buttons
    for (int i = 0; i < NUM_PRESETS; i++) {
        SDL_Rect btn = buttonRect(i);

        bool isCurrent = (PRESETS[i] == m_currentMs);
        if (isCurrent)
            SDL_SetRenderDrawColor(renderer, 102, 217, 0, 255);
        else
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderFillRect(renderer, &btn);

        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderDrawRect(renderer, &btn);

        if (m_font) {
            SDL_Color textColor = isCurrent
                ? SDL_Color{0, 0, 0, 255}
                : SDL_Color{255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(m_font, LABELS[i], textColor);
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
}

Component* TimePopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x;
    int my = event->button.y;

    for (int i = 0; i < NUM_PRESETS; i++) {
        SDL_Rect btn = buttonRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w &&
            my >= btn.y && my < btn.y + btn.h) {
            m_selectedMs = PRESETS[i];
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
