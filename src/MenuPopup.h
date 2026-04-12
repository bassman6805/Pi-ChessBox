#ifndef MENUPOPUP_H
#define MENUPOPUP_H

#include "Component.h"
#include "ImageButton.h"
#include "Button.h"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <map>

class MenuPopup : public Component {
public:
    MenuPopup(int screenW, int screenH, TTF_Font* font)
        : Component("menupopup", 0, 0, screenW, screenH)
        , m_font(font), m_visible(false), m_selectedId("")
        , m_slideX(screenW), m_targetX(screenW)
        , m_sliding(false), m_closing(false)
        , m_screenW(screenW), m_screenH(screenH)
        , m_popupW(screenW), m_popupH(screenH) {}

    virtual ~MenuPopup() { clearButtons(); }

    void setFont(TTF_Font* font) { m_font = font; }
    void setButtonLabel(const std::string& oldId, const std::string& newId) {
        for (int i = 0; i < (int)m_items.size(); i++) {
            if (m_items[i] == oldId) { m_items[i] = newId; break; }
        }
        for (auto* btn : m_buttons) {
            if (btn->id() == oldId) {
                btn->setId(newId);
                TextButton* tb = dynamic_cast<TextButton*>(btn);
                if (tb) tb->setLabel(newId);
                break;
            }
        }
    }

    void buildButtons(SDL_Renderer* renderer,
                      const std::map<std::string,std::string>& imageMap,
                      const char* assetBase,
                      const char* levelLabel = "Lvl:5",
                      const char* backLabel  = "Back",
                      const char* depthLabel = "Dpt:8",
                      const char* timeLabel  = "T:3s") {
        clearButtons();
        m_items = {
            levelLabel, backLabel, depthLabel, timeLabel,
            "Load",     "Export",  "Clock",    "Inspect",
            "Mark",     "W_P",    "B_P",      "WB_P",
            "BB_P"
        };

        int cols = 4;
        int rows = ((int)m_items.size() + cols - 1) / cols;
        int bw = 55, bh = 59, pad = 4;
        int gridW = cols * bw + (cols + 1) * pad;
        int gridH = rows * bh + (rows + 1) * pad;
        // Center the grid vertically, leave room for close button at bottom
        int closeBtnH = 50;
        int gridY = (m_popupH - gridH - closeBtnH - 20) / 2;
        if (gridY < 40) gridY = 40;
        int gridX = (m_popupW - gridW) / 2;

        for (int i = 0; i < (int)m_items.size(); i++) {
            int row = i / cols, col = i % cols;
            int bx = gridX + pad + col * (bw + pad);
            int by = gridY + pad + row * (bh + pad);
            const std::string& id = m_items[i];

            if (imageMap.count(id) && renderer) {
                ImageButton* ib = new ImageButton(id.c_str(), imageMap.at(id).c_str(), bx, by, bw, bh);
                ib->loadImage(renderer);
                m_buttons.push_back(ib);
            } else if (!id.empty()) {
                TextButton* tb = new TextButton(id.c_str(), id.c_str(), bx, by, bw, bh, m_font);
                SDL_Color limeGreen = {22, 35, 52, 255};
                tb->setBackgroundColor(limeGreen);
                m_buttons.push_back(tb);
            }
        }

        // Close button at bottom center
        int cbw = 120, cbh = 44;
        int cbx = (m_popupW - cbw) / 2;
        int cby = gridY + gridH + 10;
        TextButton* closeBtn = new TextButton("CLOSE", "Close Menu", cbx, cby, cbw, cbh, m_font);
        SDL_Color red = {180, 30, 30, 255};
        closeBtn->setBackgroundColor(red);
        m_buttons.push_back(closeBtn);

        m_gridX = gridX; m_gridY = gridY;
    }

    void show() {
        m_visible  = true;
        m_closing  = false;
        m_sliding  = true;
        m_slideX   = m_screenW;   // start off-screen right
        m_targetX  = 0;           // slide to full screen
        m_selectedId = "";
    }

    void hide() {
        // Slide out to the right
        m_closing  = true;
        m_sliding  = true;
        m_targetX  = m_screenW;
    }

    bool isVisible() const { return m_visible; }
    std::string getSelectedId() const { return m_selectedId; }
    void clearSelection() { m_selectedId = ""; }

    virtual void update(long ticks) override {
        if (!m_visible) return;

        if (m_sliding) {
            // Slide speed: full screen width in ~250ms
            float speed = m_screenW / 0.25f; // pixels per second
            float delta = speed * (ticks / 1000.0f);

            if (m_closing) {
                m_slideX += delta;
                if (m_slideX >= m_screenW) {
                    m_slideX  = m_screenW;
                    m_sliding = false;
                    m_visible = false;
                    m_closing = false;
                }
            } else {
                m_slideX -= delta;
                if (m_slideX <= 0) {
                    m_slideX  = 0;
                    m_sliding = false;
                }
            }
        }
    }

    virtual void draw(SDL_Renderer* renderer) override {
        if (!m_visible) return;

        int ox = (int)m_slideX;

        // Use SDL viewport to shift all drawing by ox
        SDL_Rect viewport = {ox, 0, m_screenW, m_screenH};
        SDL_RenderSetViewport(renderer, &viewport);

        // Full screen dark background
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_Rect bg = {0, 0, m_popupW, m_popupH};
        SDL_RenderFillRect(renderer, &bg);

        // Title
        if (m_font) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* s = TTF_RenderText_Blended(m_font, "Menu", white);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_Rect dst = {m_popupW/2 - s->w/2, 12, s->w, s->h};
                SDL_RenderCopy(renderer, t, nullptr, &dst);
                SDL_DestroyTexture(t);
                SDL_FreeSurface(s);
            }
        }

        // Buttons drawn in viewport space (no offset needed)
        for (auto* b : m_buttons)
            b->draw(renderer);

        // Reset viewport
        SDL_RenderSetViewport(renderer, nullptr);
    }

    virtual Component* mouseEvent(SDL_Event* event) override {
        if (!m_visible || m_sliding) return nullptr;
        if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

        // Adjust click coordinates for viewport offset
        SDL_Event shifted = *event;
        shifted.button.x = event->button.x - (int)m_slideX;
        shifted.button.y = event->button.y;

        for (auto* b : m_buttons) {
            if (b->mouseEvent(&shifted)) {
                if (b->id() == "CLOSE") {
                    hide();
                } else {
                    m_selectedId = b->id();
                    hide();
                }
                return b;
            }
        }
        return this;
    }

    std::vector<Button*>& getButtons() { return m_buttons; }

private:
    void clearButtons() {
        for (auto* b : m_buttons) delete b;
        m_buttons.clear();
    }

    TTF_Font*            m_font;
    bool                 m_visible;
    std::string          m_selectedId;
    std::vector<std::string> m_items;
    std::vector<Button*> m_buttons;
    int m_screenW, m_screenH, m_popupW, m_popupH;
    int m_gridX = 0, m_gridY = 0;

    // Animation
    float m_slideX;
    float m_targetX;
    bool  m_sliding;
    bool  m_closing;
};

#endif