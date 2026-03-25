#ifndef TIMEPOPUP_H
#define TIMEPOPUP_H

#include "Component.h"
#include <SDL2/SDL_ttf.h>

class TimePopup : public Component {
public:
    TimePopup(int screenW, int screenH, TTF_Font* font);
    virtual ~TimePopup() {}

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;

    void show(int currentMs);
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }

    int getSelectedMs() const { return m_selectedMs; }  // -1 = nothing, 0 = unlimited
    void clearSelection() { m_selectedMs = -1; }

private:
    TTF_Font* m_font;
    bool m_visible;
    int m_selectedMs;   // -1 = no selection, 0 = unlimited, >0 = ms
    int m_currentMs;    // 0 = unlimited
    int m_screenW, m_screenH;

    // Presets: 500, 1000, 2000, 3000, 5000, 10000, 0(unlimited)
    static const int NUM_PRESETS = 7;
    static const int PRESETS[NUM_PRESETS];
    static const char* LABELS[NUM_PRESETS];

    // 2 cols x 4 rows (7 buttons, last row has 1)
    static const int COLS = 2;
    int m_popupX, m_popupY, m_popupW, m_popupH;
    int m_btnW, m_btnH, m_padding;

    SDL_Rect buttonRect(int index) const;
};

#endif
