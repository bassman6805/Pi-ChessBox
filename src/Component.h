#ifndef COMPONENT_H
#define COMPONENT_H

#include <SDL2/SDL.h>
#include <string>

class Component {
public:
    // Only declarations here
    Component(std::string id, int x, int y, int w, int h);
    virtual ~Component() {}

    virtual void draw(SDL_Renderer* renderer) = 0;
    virtual void update(long ticks);
    virtual Component* mouseEvent(SDL_Event* event);

    std::string id() const { return m_id; }
    SDL_Rect* rect() { return &m_rect; }

    static void copyRect(SDL_Rect* dest, SDL_Rect* src);
    void setBackgroundColor(SDL_Color color) { m_bgColor = color; }

protected:
    std::string m_id;
    SDL_Rect m_rect;
    int m_x, m_y, m_w, m_h; 
    SDL_Color m_bgColor = { 255, 255, 255, 255 };
};

#endif