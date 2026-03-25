#include "Button.h"
#include <cmath>

// Helper: draw filled rounded rectangle
static void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect r, int radius, SDL_Color fill, SDL_Color border) {
    if (radius < 1) radius = 1;
    if (radius > r.w/2) radius = r.w/2;
    if (radius > r.h/2) radius = r.h/2;

    // Fill: center + cross
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_Rect center = {r.x + radius, r.y, r.w - 2*radius, r.h};
    SDL_RenderFillRect(renderer, &center);
    SDL_Rect left  = {r.x, r.y + radius, radius, r.h - 2*radius};
    SDL_Rect right = {r.x + r.w - radius, r.y + radius, radius, r.h - 2*radius};
    SDL_RenderFillRect(renderer, &left);
    SDL_RenderFillRect(renderer, &right);

    // Fill corners with circles
    for (int dy = 0; dy < radius; dy++) {
        int dx = (int)(sqrt((double)(radius*radius - dy*dy)));
        // top-left
        SDL_RenderDrawLine(renderer, r.x + radius - dx, r.y + radius - dy,
                                     r.x + radius,      r.y + radius - dy);
        // top-right
        SDL_RenderDrawLine(renderer, r.x + r.w - radius, r.y + radius - dy,
                                     r.x + r.w - radius + dx, r.y + radius - dy);
        // bottom-left
        SDL_RenderDrawLine(renderer, r.x + radius - dx, r.y + r.h - radius + dy,
                                     r.x + radius,      r.y + r.h - radius + dy);
        // bottom-right
        SDL_RenderDrawLine(renderer, r.x + r.w - radius, r.y + r.h - radius + dy,
                                     r.x + r.w - radius + dx, r.y + r.h - radius + dy);
    }

    // Border
    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    for (int i = 0; i <= radius; i++) {
        int dx = (int)(sqrt((double)(radius*radius - i*i)));
        // top arc
        SDL_RenderDrawPoint(renderer, r.x + radius - dx, r.y + radius - i);
        SDL_RenderDrawPoint(renderer, r.x + r.w - radius + dx - 1, r.y + radius - i);
        // bottom arc
        SDL_RenderDrawPoint(renderer, r.x + radius - dx, r.y + r.h - radius + i - 1);
        SDL_RenderDrawPoint(renderer, r.x + r.w - radius + dx - 1, r.y + r.h - radius + i - 1);
    }
    // Straight edges
    SDL_RenderDrawLine(renderer, r.x + radius, r.y, r.x + r.w - radius - 1, r.y);
    SDL_RenderDrawLine(renderer, r.x + radius, r.y + r.h - 1, r.x + r.w - radius - 1, r.y + r.h - 1);
    SDL_RenderDrawLine(renderer, r.x, r.y + radius, r.x, r.y + r.h - radius - 1);
    SDL_RenderDrawLine(renderer, r.x + r.w - 1, r.y + radius, r.x + r.w - 1, r.y + r.h - radius - 1);
}

// --- Button Base ---
Button::Button(std::string id, int x, int y, int w, int h)
    : Component(id, x, y, w, h) {}

Button::~Button() {}

void Button::update(long ticks) {}

Component* Button::mouseEvent(SDL_Event* event) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mx = event->button.x, my = event->button.y;
        if (mx >= m_rect.x && mx <= m_rect.x + m_rect.w &&
            my >= m_rect.y && my <= m_rect.y + m_rect.h)
            return this;
    }
    return nullptr;
}

void Button::draw(SDL_Renderer* renderer) {}

// --- TextButton ---
TextButton::TextButton(std::string id, std::string text, int x, int y, int w, int h, TTF_Font* font)
    : Button(id, x, y, w, h) {
    m_text = text;
    m_font = font;
    m_textColor = {255, 255, 255, 255};
}

void TextButton::draw(SDL_Renderer* renderer) {
    SDL_Color border = {50, 80, 110, 255};
    drawRoundedRect(renderer, m_rect, 10, m_bgColor, border);

    if (m_font && !m_text.empty()) {
        SDL_Surface* surface = TTF_RenderText_Blended(m_font, m_text.c_str(), m_textColor);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            int tw, th;
            SDL_QueryTexture(texture, NULL, NULL, &tw, &th);
            SDL_Rect textRect = {
                m_rect.x + (m_rect.w - tw) / 2,
                m_rect.y + (m_rect.h - th) / 2,
                tw, th
            };
            SDL_RenderCopy(renderer, texture, NULL, &textRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    }
}