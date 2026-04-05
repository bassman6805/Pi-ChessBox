#include "FilePickerPopup.h"
#include <cstdio>
#include <algorithm>
#include <dirent.h>
#include <SDL2/SDL.h>

FilePickerPopup::FilePickerPopup(int screenW, int screenH, TTF_Font* font)
    : Component("filepickerpopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_scrollOffset(0), m_highlightedIndex(-1),
      m_slideX(screenW), m_sliding(false), m_closing(false),
      m_screenW(screenW), m_screenH(screenH) {

    m_padding  = 10;
    m_btnH     = 52;
    m_popupW   = screenW;
    m_popupH   = screenH;
    m_popupX   = 0;
    m_popupY   = 0;
    m_visibleRows = (m_popupH - 50 - 50 - m_padding) / (m_btnH + m_padding);
}

void FilePickerPopup::loadFiles() {
    m_files.clear();
    DIR* dir = opendir(m_directory.c_str());
    if (!dir) return;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.size() > 4 && name.substr(name.size() - 4) == ".pgn")
            m_files.push_back(name);
    }
    closedir(dir);
    std::sort(m_files.begin(), m_files.end(), std::greater<std::string>()); // newest first
}

void FilePickerPopup::show(const std::string& directory) {
    m_directory = directory;
    m_selectedFile = "";
    m_scrollOffset = 0;
    m_highlightedIndex = -1;
    loadFiles();
    m_visible = true;
    m_closing = false;
    m_sliding = true;
    m_slideX  = m_screenW; // start off-screen right
}

SDL_Rect FilePickerPopup::itemRect(int visibleIndex) const {
    int y = m_popupY + 50 + m_padding + visibleIndex * (m_btnH + m_padding);
    return {m_popupX + m_padding, y, m_popupW - 2 * m_padding, m_btnH};
}

void FilePickerPopup::update(long ticks) {
    if (!m_visible) return;
    if (m_sliding) {
        float speed = m_screenW / 0.35f;
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

void FilePickerPopup::draw(SDL_Renderer* renderer) {
    if (!m_visible) return;

    int ox = (int)m_slideX;
    SDL_Rect viewport = {ox, 0, m_screenW, m_screenH};
    SDL_RenderSetViewport(renderer, &viewport);

    // Main background
    SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255);
    SDL_Rect full = {0, 0, m_screenW, m_screenH};
    SDL_RenderFillRect(renderer, &full);

    // Title bar
    SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
    SDL_Rect titleBar = {0, 0, m_screenW, 48};
    SDL_RenderFillRect(renderer, &titleBar);

    // Title text
    if (m_font) {
        SDL_Color white = {255, 255, 255, 255};
        const char* title = m_files.empty() ? "No PGN files found" : "Select Game to Load";
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, title, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect dst = {m_screenW/2 - surf->w/2, 12, surf->w, surf->h};
                SDL_RenderCopy(renderer, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    // File list items
    for (int i = 0; i < m_visibleRows; i++) {
        int fileIdx = m_scrollOffset + i;
        if (fileIdx >= (int)m_files.size()) break;
        SDL_Rect btn = itemRect(i);
        bool isHighlighted = (fileIdx == m_highlightedIndex);
        bool isEven = (i % 2 == 0);
        if (isHighlighted) {
            SDL_SetRenderDrawColor(renderer, 180, 230, 0, 255);
        } else if (isEven) {
            SDL_SetRenderDrawColor(renderer, 22, 35, 52, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 55, 90, 255);
        }
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
        SDL_RenderDrawRect(renderer, &btn);

        if (m_font) {
            SDL_Color textColor = isHighlighted ? SDL_Color{10, 14, 20, 255} : SDL_Color{200, 220, 240, 255};
            std::string display = m_files[fileIdx];
            if (display.size() > 44) display = display.substr(0, 41) + "...";
            SDL_Surface* surf = TTF_RenderText_Blended(m_font, display.c_str(), textColor);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int ty = btn.y + (btn.h - surf->h) / 2;
                    SDL_Rect dst = {btn.x + 12, ty, std::min(surf->w, btn.w - 24), surf->h};
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }
    }

    // Load and Cancel buttons
    int btnRowY = m_popupH - 60;
    SDL_Rect loadBtn   = {m_padding,                         btnRowY, (m_popupW - 3*m_padding)/2, 50};
    SDL_Rect cancelBtn = {m_padding + loadBtn.w + m_padding, btnRowY, loadBtn.w, 50};

    bool canLoad = (m_highlightedIndex >= 0 && m_highlightedIndex < (int)m_files.size());
    SDL_SetRenderDrawColor(renderer, canLoad ? 0 : 15, canLoad ? 110 : 30, canLoad ? 175 : 50, 255);
    SDL_RenderFillRect(renderer, &loadBtn);
    SDL_SetRenderDrawColor(renderer, canLoad ? 100 : 40, canLoad ? 160 : 80, canLoad ? 210 : 100, 255);
    SDL_RenderDrawRect(renderer, &loadBtn);

    SDL_SetRenderDrawColor(renderer, 80, 20, 20, 255);
    SDL_RenderFillRect(renderer, &cancelBtn);
    SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);
    SDL_RenderDrawRect(renderer, &cancelBtn);

    if (m_font) {
        auto drawLabel = [&](SDL_Rect& r, const char* label, SDL_Color col) {
            SDL_Surface* s = TTF_RenderText_Blended(m_font, label, col);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                if (t) {
                    SDL_Rect d = {r.x + (r.w - s->w)/2, r.y + (r.h - s->h)/2, s->w, s->h};
                    SDL_RenderCopy(renderer, t, nullptr, &d);
                    SDL_DestroyTexture(t);
                }
                SDL_FreeSurface(s);
            }
        };
        SDL_Color white = {220, 235, 255, 255};
        drawLabel(loadBtn,   "Load",   canLoad ? white : SDL_Color{80, 100, 120, 255});
        drawLabel(cancelBtn, "Cancel", white);
    }

    SDL_RenderSetViewport(renderer, nullptr);
}

Component* FilePickerPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible || m_sliding) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x - (int)m_slideX;
    int my = event->button.y;

    // Load / Cancel buttons
    int btnRowY = m_popupY + m_popupH - 48;
    SDL_Rect loadBtn   = {m_popupX + m_padding,                          btnRowY, (m_popupW - 3*m_padding)/2, 40};
    SDL_Rect cancelBtn = {m_popupX + m_padding + loadBtn.w + m_padding,  btnRowY, loadBtn.w, 40};

    if (mx >= loadBtn.x && mx < loadBtn.x + loadBtn.w && my >= loadBtn.y && my < loadBtn.y + loadBtn.h) {
        if (m_highlightedIndex >= 0 && m_highlightedIndex < (int)m_files.size()) {
            m_selectedFile = m_directory + "/" + m_files[m_highlightedIndex];
            m_closing = true;
            m_sliding = true;
            return this;
        }
        return this;
    }
    if (mx >= cancelBtn.x && mx < cancelBtn.x + cancelBtn.w && my >= cancelBtn.y && my < cancelBtn.y + cancelBtn.h) {
        m_closing = true;
        m_sliding = true;
        return this;
    }

    // File items — tap to highlight
    for (int i = 0; i < m_visibleRows; i++) {
        int fileIdx = m_scrollOffset + i;
        if (fileIdx >= (int)m_files.size()) break;
        SDL_Rect btn = itemRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w && my >= btn.y && my < btn.y + btn.h) {
            m_highlightedIndex = fileIdx;
            return this;
        }
    }

    // Click outside = slide out
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    if (mx < popup.x || mx >= popup.x + popup.w || my < popup.y || my >= popup.y + popup.h) {
        m_closing = true;
        m_sliding = true;
    }
    return nullptr;
}
