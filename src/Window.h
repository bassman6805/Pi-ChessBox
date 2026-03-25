#ifndef WINDOW_H
#define WINDOW_H

#include <vector>
#include <string>
#include <SDL.h>
#include "Component.h"

// Forward declaration
class Button;

class Window : public Component {
public:
    Window(const char* title, int x, int y, int w, int h);
    virtual ~Window();
	SDL_Renderer* getRenderer() { return m_renderer; }

    // These must be INSIDE the class braces
    void addComponent(Component* c);
    void addButton(Button* b);

    virtual void update(long ticks) override;
    virtual void draw(SDL_Renderer* renderer) override;
    virtual void processMouseEvent(SDL_Event* event);
    virtual void processButtonClicked(Button* b);

protected:
    std::string m_title;
    std::vector<Component*> m_components;
    std::vector<Button*> m_buttons;

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
}; // <--- This is where the class should actually end

#endif