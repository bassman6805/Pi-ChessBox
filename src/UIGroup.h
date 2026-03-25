#ifndef UIGROUP_H
#define UIGROUP_H

#include "Component.h"
#include <list>

class UIGroup : public Component {
public:
    UIGroup(std::string id, int x, int y, int w, int h);
    
    void add(Component* c) { m_components.push_back(c); }
    void remove(Component* c); // Declaration for the .cpp
    
    Component* find(const char* id); // Declaration for the .cpp
    
    void draw(SDL_Renderer* renderer) override;
    void update(long ticks) override;
    Component* mouseEvent(SDL_Event* event) override;

    std::list<Component*>::iterator begin() { return m_components.begin(); }
    std::list<Component*>::iterator end() { return m_components.end(); }

protected:
    std::list<Component*> m_components;
};

#endif