#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "Component.h"
#include <SDL_ttf.h>
#include <string>

class TextField : public Component {
public:
    TextField(std::string id, int x, int y, int w, int h);
    virtual ~TextField() { invalidateTexture(); }
    
    void setText(const char* s);
    const char* text() { return m_text.c_str(); }

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override;
    void invalidateTexture();

protected:
    std::string m_text;
    TTF_Font* m_font = nullptr;
    SDL_Texture* m_fontTexture = nullptr;
    SDL_Rect m_fontRect;
};

#endif