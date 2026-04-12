#ifndef MARKPOPUP_H
#define MARKPOPUP_H
#include "Component.h"
#include <SDL2/SDL_ttf.h>
#include <string>
class MarkPopup : public Component {
public:
    enum Result { NONE=-1, RETURN_TO_MARK=0, NEW_MARK=1, CLEAR=2 };
    MarkPopup(int screenW, int screenH, TTF_Font* font);
    virtual ~MarkPopup() {}
    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;
    void show();
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }
    Result getResult() const { return m_result; }
    void clearResult() { m_result = NONE; }
private:
    TTF_Font* m_font;
    bool      m_visible;
    Result    m_result;
    int       m_screenW, m_screenH;
    int       m_popupX, m_popupY, m_popupW, m_popupH;
    int       m_btnW, m_btnH, m_padding;
    SDL_Rect buttonRect(int index) const;
};
#endif
