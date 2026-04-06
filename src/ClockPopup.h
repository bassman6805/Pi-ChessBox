#ifndef CLOCKPOPUP_H
#define CLOCKPOPUP_H
#include "Component.h"
#include <SDL2/SDL_ttf.h>

class ClockPopup : public Component {
public:
    ClockPopup(int screenW, int screenH, TTF_Font* font);
    virtual ~ClockPopup() {}
    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;

    void show(int currentIndex);
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }

    int getSelectedIndex() const { return m_selectedIndex; }  // -1 = nothing selected
    void clearSelection() { m_selectedIndex = -1; }

    // Preset data accessors
    static int presetTimeMs(int index);      // starting time in ms (0 = unlimited)
    static int presetIncrementSec(int index); // increment in seconds
    static const char* presetLabel(int index);
    static const int NUM_PRESETS = 7;

private:
    TTF_Font* m_font;
    bool      m_visible;
    int       m_selectedIndex;
    int       m_currentIndex;
    int       m_screenW, m_screenH;

    int m_popupX, m_popupY, m_popupW, m_popupH;
    int m_btnW, m_btnH, m_padding;

    SDL_Rect buttonRect(int index) const;

    static const int COLS = 2;
    static const int ROWS = 4;  // 7 presets, 2 cols = 4 rows (last row has 1)
};
#endif
