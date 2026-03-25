#include "Component.h"

Component::Component(std::string id, int x, int y, int w, int h) 
    : m_id(id), m_x(x), m_y(y), m_w(w), m_h(h) {
    m_rect = {x, y, w, h};
}

void Component::update(long ticks) {
    // Default implementation: do nothing
}

Component* Component::mouseEvent(SDL_Event* event) {
    // Default implementation: return nullptr
    return nullptr;
}

void Component::copyRect(SDL_Rect* dest, SDL_Rect* src) {
    if (dest && src) {
        *dest = *src;
    }
}