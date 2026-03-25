#include "Label.h"
#include <iostream>

using namespace std;

Label::Label(string id, int x, int y, int w, int h) : Component(id, x, y, w, h), m_text("") {
    m_fontTexture = nullptr;
    m_textColor = {0, 0, 0, 255};
    // Note: If you have a FontManager, you can initialize m_font here
}

void Label::setFont(TTF_Font* font) {
    m_font = font;
    invalidateTexture();
}

void Label::setText(const char *s) {
    m_text = s;
    invalidateTexture();
}

void Label::appendText(const char *s) {
    m_text += s;
    invalidateTexture();
}

void Label::invalidateTexture() {
    if(m_fontTexture) {
        SDL_DestroyTexture(m_fontTexture);
        m_fontTexture = nullptr;
    }
}

void Label::draw(SDL_Renderer *renderer) {
    if (m_text.empty() || !m_font) return;

    if (!m_fontTexture) {
        SDL_Surface* surface = TTF_RenderText_Blended(m_font, m_text.c_str(), m_textColor);
        if (surface) {
            m_fontTexture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_QueryTexture(m_fontTexture, NULL, NULL, &m_fontRect.w, &m_fontRect.h);
            m_fontRect.x = m_rect.x;
            m_fontRect.y = m_rect.y;
            SDL_FreeSurface(surface);
        }
    }
    
    if (m_fontTexture) {
        SDL_RenderCopy(renderer, m_fontTexture, NULL, &m_fontRect);
    }
}

void Label::setColor(SDL_Color c) {
    m_textColor = c;
    invalidateTexture();
}