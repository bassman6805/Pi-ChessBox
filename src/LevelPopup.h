#ifndef LEVELPOPUP_H
#define LEVELPOPUP_H

#include "Component.h"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>

class LevelPopup : public Component {
public:
    LevelPopup(int screenW, int screenH, TTF_Font* font);
    virtual ~LevelPopup() {}

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;

    void show(int currentLevel);
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }

    // Called when user selects a level. Returns -1 if nothing selected.
    int getSelectedLevel() const { return m_selectedLevel; }
    void clearSelection() { m_selectedLevel = -1; }

private:
    TTF_Font* m_font;
    bool m_visible;
    int m_selectedLevel;
    int m_currentLevel;
    int m_screenW, m_screenH;

    // Layout
    static const int COLS = 3;
    static const int ROWS = 7;   // 21 buttons (0-20), 3 cols x 7 rows
    int m_popupX, m_popupY, m_popupW, m_popupH;
    int m_btnW, m_btnH, m_padding;

    SDL_Rect buttonRect(int level) const;
};

#endif
