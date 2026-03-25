#include "TextField.h"
#include <iostream>

using namespace std;

TextField::TextField(string id, int x, int y, int w, int h)
    : Component(id, x, y, w, h),
    m_text(""),
    m_fontTexture(nullptr),
    m_font(nullptr)
{
    m_bgColor = {255, 255, 255, 255};
    // Default font path - ensure this exists or set via a setter
    m_font = TTF_OpenFont("assets/fonts/Inconsolata-Medium.ttf", 16);
}

void TextField::setText(const char *s) {
    m_text = s;
    invalidateTexture();
}

void TextField::invalidateTexture() {
    if(m_fontTexture) {
        SDL_DestroyTexture(m_fontTexture);
        m_fontTexture = nullptr;
    }
}

void TextField::draw(SDL_Renderer *renderer) {
    // 1. Draw background
    SDL_SetRenderDrawColor(renderer, m_bgColor.r, m_bgColor.g, m_bgColor.b, 255);
    SDL_RenderFillRect(renderer, &m_rect);

    if (!m_font || m_text.empty()) return; 

    if (!m_fontTexture) {
        SDL_Color textColor = { 0, 0, 0, 255 };
        SDL_Surface* surface = TTF_RenderText_Blended(m_font, m_text.c_str(), textColor);
        if (surface) {
            m_fontTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(m_fontTexture, NULL, NULL, &m_fontRect.w, &m_fontRect.h);
            m_fontRect.x = m_rect.x + 5;
            m_fontRect.y = m_rect.y + (m_rect.h - m_fontRect.h) / 2;
            SDL_FreeSurface(surface);
        }
    }

    if (m_fontTexture) {
        SDL_RenderCopy(renderer, m_fontTexture, NULL, &m_fontRect);
    }
}

void TextField::update(long ticks) {}