#include "DepthPopup.h"
#include <cstdio>
#include <SDL2/SDL.h>

DepthPopup::DepthPopup(int screenW, int screenH, TTF_Font* font)
    : Component("depthpopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_selectedDepth(-1),
      m_currentDepth(5), m_screenW(screenW), m_screenH(screenH) {

    m_padding = 8;
    m_btnW = 70;
    m_btnH = 36;

    m_popupW = COLS * m_btnW + (COLS + 1) * m_padding;
    m_popupH = ROWS * m_btnH + (ROWS + 1) * m_padding + 40; // +40 for title

    m_popupX = (screenW - m_popupW) / 2;
    m_popupY = (screenH - m_popupH) / 2;
}

void DepthPopup::show(int currentDepth) {
    m_currentDepth = currentDepth;
    m_selectedDepth = -1;
    m_visible = true;
}

SDL_Rect DepthPopup::buttonRect(int index) const {
    // index 0 = depth 1, index 14 = depth 15
    int col = index % COLS;
    int row = index / COLS;
    int x = m_popupX + m_padding + col * (m_btnW + m_padding);
    int y = m_popupY + 40 + m_padding + row * (m_btnH + m_padding);
    return {x, y, m_btnW, m_btnH};
}

void DepthPopup::draw(SDL_Renderer* renderer) {
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
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, "Select Search Depth", white);
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

    // Depth buttons (1-15)
    for (int i = 0; i < MAX_DEPTH; i++) {
        int depth = i + 1;
        SDL_Rect btn = buttonRect(i);

        // Button background - highlight current
        if (depth == m_currentDepth)
            SDL_SetRenderDrawColor(renderer, 102, 217, 0, 255);
        else
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderFillRect(renderer, &btn);

        // Button border
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderDrawRect(renderer, &btn);

        // Label
        if (m_font) {
            char label[8];
            snprintf(label, sizeof(label), "%d", depth);
            SDL_Color textColor = (depth == m_currentDepth)
                ? SDL_Color{0, 0, 0, 255}
                : SDL_Color{255, 255, 255, 255};
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

Component* DepthPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x;
    int my = event->button.y;

    for (int i = 0; i < MAX_DEPTH; i++) {
        SDL_Rect btn = buttonRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w &&
            my >= btn.y && my < btn.y + btn.h) {
            m_selectedDepth = i + 1; // depth = index + 1
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
