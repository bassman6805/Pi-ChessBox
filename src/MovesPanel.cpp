#include "MovesPanel.h"
#include <cstdio>
#include <SDL2/SDL.h>

MovesPanel::MovesPanel(const char* id, int x, int y, int w, int h, TTF_Font* font)
    : Component(id, x, y, w, h), m_font(font), m_dirty(true) {
}

void MovesPanel::setFont(TTF_Font* font) { m_font = font; m_dirty = true; }
void MovesPanel::addMove(const char* lan) {
    if (lan && lan[0]) m_moves.push_back(std::string(lan));
    m_dirty = true;
}
void MovesPanel::clear() { m_moves.clear(); m_dirty = true; }
void MovesPanel::removeLastTwo() {
    if (m_moves.size() >= 2) m_moves.resize(m_moves.size() - 2);
    else m_moves.clear();
    m_dirty = true;
}

void MovesPanel::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 13, 17, 23, 255);  // same as window bg
    SDL_RenderFillRect(renderer, &m_rect);

    int rh      = rowHeight();
    int dataRows = m_rect.h / rh;
    int totalSlots = dataRows * 2;

    struct Pair { int num; std::string white; std::string black; };
    std::vector<Pair> pairs;
    for (size_t i = 0; i < m_moves.size(); i += 2) {
        Pair p;
        p.num   = (int)(i / 2) + 1;
        p.white = m_moves[i];
        p.black = (i + 1 < m_moves.size()) ? m_moves[i + 1] : "";
        pairs.push_back(p);
    }

    int startPair = (int)pairs.size() - totalSlots;
    if (startPair < 0) startPair = 0;

    int colW = m_rect.w / 2;
    SDL_Color stripeLight = {24, 36, 48, 255};   // slightly lighter row
    SDL_Color stripeDark  = {13, 17, 23, 255};   // same as bg
    SDL_Color textColor   = {200, 215, 230, 255}; // bright blue-white text

    for (int slot = 0; slot < totalSlots; slot++) {
        int pairIdx = startPair + slot;
        int col     = slot / dataRows;
        int row     = slot % dataRows;
        int xx      = m_rect.x + col * colW;
        int yy      = m_rect.y + row * rh;

        // Stripe
        SDL_Color bg = (row % 2 == 0) ? stripeLight : stripeDark;
        SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
        SDL_Rect stripe = {xx, yy, colW, rh};
        SDL_RenderFillRect(renderer, &stripe);

        if (!m_font || pairIdx >= (int)pairs.size()) continue;

        const Pair& p = pairs[pairIdx];
        char buf[32];
        snprintf(buf, sizeof(buf), "%d.%s %s", p.num, p.white.c_str(), p.black.c_str());
        renderText(renderer, buf, textColor, xx + 4, yy + 2, colW - 8);
    }

    // Column divider
    SDL_SetRenderDrawColor(renderer, 32, 48, 64, 255);
    SDL_RenderDrawLine(renderer, m_rect.x + colW, m_rect.y,
                                 m_rect.x + colW, m_rect.y + m_rect.h);
}

void MovesPanel::renderText(SDL_Renderer* renderer, const char* text,
                             SDL_Color color, int x, int y, int maxW) {
    if (!m_font || !text) return;
    SDL_Surface* surf = TTF_RenderText_Blended(m_font, text, color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        if (dst.w > maxW) dst.w = maxW;
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}