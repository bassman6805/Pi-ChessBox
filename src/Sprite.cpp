#include "Sprite.h"
#include <SDL_image.h>
#include <iostream>

// Updated constructor to match the call in Board.cpp
Sprite::Sprite(SDL_Renderer* renderer, const char* path) : Component("sprite", 0, 0, 0, 0) {
    m_texture = IMG_LoadTexture(renderer, path);
    if (!m_texture) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
        return;
    }

    // Initialize source and destination rectangles
    m_sourceRect.x = 0;
    m_sourceRect.y = 0;
    SDL_QueryTexture(m_texture, NULL, NULL, &m_sourceRect.w, &m_sourceRect.h);
    
    // Default scaling or size setup
    m_rect.w = m_sourceRect.w;
    m_rect.h = m_sourceRect.h;
    m_rect.x = 0;
    m_rect.y = 0;
    
    m_frameCount = 1;
    m_delay = 100;
}

// Keep the original constructor for backwards compatibility if needed
Sprite::Sprite(const char *id) : Component(id, 0, 0, 0, 0) {
    m_texture = nullptr;
    m_frameCount = 1;
}

void Sprite::draw(SDL_Renderer* renderer) {
    if (m_texture) {
        SDL_RenderCopy(renderer, m_texture, &m_sourceRect, &m_rect);
    }
}

void Sprite::draw(SDL_Renderer* renderer, SDL_Rect* dest) {
    if (m_texture) {
        SDL_RenderCopy(renderer, m_texture, &m_sourceRect, dest);
    }
}

void Sprite::update(long ticks) {
    if (m_frameCount > 1) {
        m_sourceRect.x = m_sourceRect.w * int(((ticks / m_delay) % m_frameCount));
    }
}

void Sprite::setPos(int x, int y) {
    m_rect.x = x;
    m_rect.y = y;
}

// Helper to scale the piece to fit the board square
void Sprite::setSize(int w, int h) {
    m_rect.w = w;
    m_rect.h = h;
}