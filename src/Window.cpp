#include "Window.h"
#include "Button.h"
#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h> // Required for PNG pieces

Window::Window(const char* title, int x, int y, int w, int h) 
    : Component("window", x, y, w, h) {
    m_title = title;

    // If width is 0, skip SDL setup - caller will manage its own window/renderer
    if (w == 0) return;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        if (TTF_Init() == -1) {
            printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        }

        // Initialize SDL_image for PNG support
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        }

#ifdef _WIN32
        m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
#else
        // Borderless on Pi so title bar doesn't eat screen space
        m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
#endif
        if (m_window != nullptr) {
            m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        }
    }
}

Window::~Window() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window) SDL_DestroyWindow(m_window);
    if (m_window || m_renderer) {
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }
}

void Window::update(long ticks) {
    for (auto c : m_components) c->update(ticks);
    for (auto b : m_buttons) b->update(ticks);
}

void Window::addComponent(Component* c) { m_components.push_back(c); }
void Window::addButton(Button* b) { m_buttons.push_back(b); }

void Window::draw(SDL_Renderer* renderer) {
    for (auto c : m_components) c->draw(renderer);
    for (auto b : m_buttons) b->draw(renderer); 
}

void Window::processMouseEvent(SDL_Event* event) {
    for (auto b : m_buttons) {
        Component* caught = b->mouseEvent(event);
        if (caught) {
            this->processButtonClicked(static_cast<Button*>(caught));
        }
    }
}

void Window::processButtonClicked(Button* b) {
    if (!b) return;

    std::cout << "Window caught click: " << b->id() << std::endl;

    // Check if this is the connect button
    if (b->id() == "connect") {
        // You need a way to trigger the connection here.
        // Usually, this is done via a callback or by 
        // passing a pointer to your Connector/GUI class.
        std::cout << "DEBUG: Triggering connection logic..." << std::endl;
    }
}