#ifndef MOVESPANEL_H
#define MOVESPANEL_H

#include "Component.h"
#include <vector>
#include <string>
#include <SDL2/SDL_ttf.h>

class MovesPanel : public Component {
public:
    MovesPanel(const char* id, int x, int y, int w, int h, TTF_Font* font);
    virtual ~MovesPanel() {}

    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}

    void setFont(TTF_Font* font);
    void addMove(const char* lan);
    void clear();
    void removeLastTwo();
    const std::vector<std::string>& getMoves() const { return m_moves; }

private:
    void renderText(SDL_Renderer* renderer, const char* text,
                    SDL_Color color, int x, int y, int maxW);
    int visibleRows() const { return m_rect.h / rowHeight(); }
    int rowHeight() const { return 20; }

    TTF_Font*                m_font;
    std::vector<std::string> m_moves;
    bool                     m_dirty;
};

#endif