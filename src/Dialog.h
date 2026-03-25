#ifndef DIALOG_H
#define DIALOG_H

#include "Component.h"
#include "Button.h" // Needed because we have actual TextButton objects, not pointers

enum DialogType {
    DIALOG_TYPE_OK,
    DIALOG_TYPE_OK_CANCEL,
    DIALOG_TYPE_YES_NO
};

enum DialogSelection {
    DIALOG_SELECTED_NONE,
    DIALOG_SELECTED_OKAY,
    DIALOG_SELECTED_CANCEL
};

class Dialog : public Component {
public:
    Dialog(const char* title, const char* msg, int type);
    virtual ~Dialog();

    virtual void draw(SDL_Renderer* r) override;
    virtual void update(long ticks) override;
    int show(SDL_Renderer* renderer);
    int processMouseEvent(SDL_Event* event);

protected:
    std::string m_title;
    std::string m_message;
    TextButton m_okayButton;
    TextButton m_cancelButton;
    int m_type;
    bool m_showing;
    int m_selection;
    void* m_font;
    SDL_Texture* m_fontTexture;
};

#endif