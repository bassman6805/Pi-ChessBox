#ifndef LABEL_H
#define LABEL_H

#include "Component.h"
#include <SDL_ttf.h>
#include <string>

class Label : public Component {
public:
    Label(std::string id, int x, int y, int w, int h);
    virtual ~Label() { invalidateTexture(); }

    void draw(SDL_Renderer* renderer) override;
    
    void setFont(TTF_Font* font);
    void setText(const char* s);
    void appendText(const char* s);
    void setColor(SDL_Color c);
    void invalidateTexture();

protected:
    std::string m_text;
    TTF_Font* m_font = nullptr;
    SDL_Texture* m_fontTexture = nullptr;
    SDL_Rect m_fontRect;
    SDL_Color m_textColor = {0, 0, 0, 255};
};

#endif