#ifndef SPRITE_H
#define SPRITE_H

#include "Component.h"
#include <SDL.h>

class Sprite : public Component {
public:
    // This constructor is needed for Board::loadPieces
    Sprite(SDL_Renderer* renderer, const char* path);
    Sprite(const char* id);

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override;

    void draw(SDL_Renderer* renderer, SDL_Rect* dest);
    void setPos(int x, int y);
    
    // FIX: Add this declaration to match Sprite.cpp
    void setSize(int w, int h);

    // Existing helpers
    void setFrame(int frame);
    void setDelay(Uint32 delayMilliSeconds);

private:
    SDL_Texture* m_texture;
    SDL_Rect m_sourceRect;
    std::string m_name;
    int m_frameCount;
    int m_currentFrame;
    Uint32 m_delay;
    const float SCALE = 1.0f; 
};

#endif