#include "LevelPopup.h"
#include <cstdio>
#include <SDL2/SDL.h>

LevelPopup::LevelPopup(int screenW, int screenH, TTF_Font* font)
    : Component("levelpopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_selectedLevel(-1),
      m_currentLevel(20), m_screenW(screenW), m_screenH(screenH) {

    m_padding = 8;
    m_btnW = 70;
    m_btnH = 36;

    m_popupW = COLS * m_btnW + (COLS + 1) * m_padding;
    m_popupH = ROWS * m_btnH + (ROWS + 1) * m_padding + 40; // +40 for title

    m_popupX = (screenW - m_popupW) / 2;
    m_popupY = (screenH - m_popupH) / 2;
}

void LevelPopup::show(int currentLevel) {
    m_currentLevel = currentLevel;
    m_selectedLevel = -1;
    m_visible = true;
}

SDL_Rect LevelPopup::buttonRect(int level) const {
    int col = level % COLS;
    int row = level / COLS;
    int x = m_popupX + m_padding + col * (m_btnW + m_padding);
    int y = m_popupY + 40 + m_padding + row * (m_btnH + m_padding);
    return {x, y, m_btnW, m_btnH};
}

void LevelPopup::draw(SDL_Renderer* renderer) {
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
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, "Select Skill Level", white);
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

    // Level buttons
    for (int i = 0; i <= 20; i++) {
        SDL_Rect btn = buttonRect(i);

        // Button background
        if (i == m_currentLevel) {
            SDL_SetRenderDrawColor(renderer, 102, 217, 0, 255); // lime green = current
        } else {
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        }
        SDL_RenderFillRect(renderer, &btn);

        // Button border
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderDrawRect(renderer, &btn);

        // Button label
        if (m_font) {
            char label[8];
            snprintf(label, sizeof(label), "%d", i);
            SDL_Color textColor = (i == m_currentLevel) ? SDL_Color{0, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Blended(m_font, label, textColor);
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

Component* LevelPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x;
    int my = event->button.y;

    // Check each level button
    for (int i = 0; i <= 20; i++) {
        SDL_Rect btn = buttonRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w &&
            my >= btn.y && my < btn.y + btn.h) {
            m_selectedLevel = i;
            m_visible = false;
            return this;
        }
    }

    // Click outside popup = dismiss
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    if (mx < popup.x || mx >= popup.x + popup.w ||
        my < popup.y || my >= popup.y + popup.h) {
        m_visible = false;
    }

    return nullptr;
}
