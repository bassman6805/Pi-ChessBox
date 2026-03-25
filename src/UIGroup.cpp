#include "UIGroup.h"

UIGroup::UIGroup(std::string id, int x, int y, int w, int h) 
    : Component(id, x, y, w, h) {}

void UIGroup::draw(SDL_Renderer* renderer) {
    for(auto c : m_components) {
        SDL_Rect originalRect = *c->rect();
        
        // Offset for group positioning
        c->rect()->x += m_rect.x;
        c->rect()->y += m_rect.y;
        
        c->draw(renderer);
        
        // Restore original position
        *c->rect() = originalRect;
    }
}

void UIGroup::update(long ticks) {
    for(auto c : m_components) c->update(ticks);
}

Component* UIGroup::mouseEvent(SDL_Event* event) {
    for(auto c : m_components) {
        Component* caught = c->mouseEvent(event);
        if(caught) return caught;
    }
    return nullptr;
}