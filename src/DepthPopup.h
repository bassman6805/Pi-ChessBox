#ifndef DEPTHPOPUP_H
#define DEPTHPOPUP_H

#include "Component.h"
#include <SDL2/SDL_ttf.h>

class DepthPopup : public Component {
public:
    DepthPopup(int screenW, int screenH, TTF_Font* font);
    virtual ~DepthPopup() {}

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;

    void show(int currentDepth);
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }

    int getSelectedDepth() const { return m_selectedDepth; }
    void clearSelection() { m_selectedDepth = -1; }

private:
    TTF_Font* m_font;
    bool m_visible;
    int m_selectedDepth;
    int m_currentDepth;
    int m_screenW, m_screenH;

    // 15 options (depth 1-15), 3 cols x 5 rows
    static const int COLS = 3;
    static const int ROWS = 5;
    static const int MAX_DEPTH = 15;
    int m_popupX, m_popupY, m_popupW, m_popupH;
    int m_btnW, m_btnH, m_padding;

    SDL_Rect buttonRect(int index) const;
};

#endif
