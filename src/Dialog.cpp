#include "Dialog.h"
#include <iostream>

// Constructor
Dialog::Dialog(const char* title, const char* message, int type)
    : Component(title, 0, 0, 480, 800), 
      m_okayButton("ok", "OK", 100, 450, 80, 40, nullptr),
      m_cancelButton("cancel", "Cancel", 300, 450, 80, 40, nullptr)
{
    m_title = title;
    m_message = message; 
    m_type = type;
    m_showing = false;
    m_selection = DIALOG_SELECTED_NONE;
    
    setBackgroundColor({200, 200, 200, 255}); // Light grey
}

// Destructor
Dialog::~Dialog() {
    // Add cleanup here if you add font textures later
}

void Dialog::draw(SDL_Renderer* r) {
    if (!m_showing) return;

    // 1. Draw semi-transparent background overlay
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
    SDL_RenderFillRect(r, nullptr);

    // 2. Draw the actual dialog box center screen
    SDL_Rect box = { 40, 300, 400, 200 };
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255); // Black border
    SDL_RenderDrawRect(r, &box);

    // 3. Draw buttons [Fixed the cut-off logic here]
    m_okayButton.draw(r);
    
    if (m_type == DIALOG_TYPE_OK_CANCEL || m_type == DIALOG_TYPE_YES_NO) {
        m_cancelButton.draw(r);
    }
}

void Dialog::update(long ticks) {
    if (!m_showing) return;
    m_okayButton.update(ticks);
    m_cancelButton.update(ticks);
}

int Dialog::show(SDL_Renderer* renderer) {
    m_showing = true;
    m_selection = DIALOG_SELECTED_NONE;
    return m_selection;
}

int Dialog::processMouseEvent(SDL_Event* event) {
    if (!m_showing) return DIALOG_SELECTED_NONE;

    // Check Okay Button
    if (m_okayButton.mouseEvent(event)) {
        if (event->type == SDL_MOUSEBUTTONUP) {
            m_selection = DIALOG_SELECTED_OKAY;
            m_showing = false;
        }
    }
    
    // Check Cancel Button
    if (m_type == DIALOG_TYPE_OK_CANCEL || m_type == DIALOG_TYPE_YES_NO) {
        if (m_cancelButton.mouseEvent(event)) {
            if (event->type == SDL_MOUSEBUTTONUP) {
                m_selection = DIALOG_SELECTED_CANCEL;
                m_showing = false;
            }
        }
    }

    return m_selection;
}