#ifndef BUTTON_H
#define BUTTON_H

#include "Component.h"
#include <SDL_ttf.h>
#include <string>
#include <cmath>

class Button : public Component {
public:
    // Only declarations here (ending in ;)
    Button(std::string id, int x, int y, int w, int h);
    virtual ~Button();

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override; // This was missing
    virtual Component* mouseEvent(SDL_Event* event) override;
};

class TextButton : public Button {
public:
    TextButton(std::string id, std::string text, int x, int y, int w, int h, TTF_Font* font);
    
    void draw(SDL_Renderer* renderer) override;
    void setTextColor(SDL_Color c) { m_textColor = c; }
    void setLabel(const std::string& text) { m_text = text; }

protected:
    std::string m_text;
    TTF_Font*   m_font;
    SDL_Color   m_textColor = {255, 255, 255, 255};
};

#endif