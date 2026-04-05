#include "FilePickerPopup.h"
#include <cstdio>
#include <algorithm>
#include <dirent.h>
#include <SDL2/SDL.h>

FilePickerPopup::FilePickerPopup(int screenW, int screenH, TTF_Font* font)
    : Component("filepickerpopup", 0, 0, screenW, screenH),
      m_font(font), m_visible(false), m_scrollOffset(0),
      m_screenW(screenW), m_screenH(screenH) {

    m_padding  = 8;
    m_btnH     = 44;
    m_popupW   = 420;
    m_visibleRows = 9;   // fits on 800px tall screen with title + scroll buttons
    m_popupH   = m_visibleRows * (m_btnH + m_padding) + m_padding + 50 + 50; // +50 title, +50 scroll buttons
    m_popupX   = (screenW - m_popupW) / 2;
    m_popupY   = (screenH - m_popupH) / 2;
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
    loadFiles();
    m_visible = true;
}

SDL_Rect FilePickerPopup::itemRect(int visibleIndex) const {
    int y = m_popupY + 50 + m_padding + visibleIndex * (m_btnH + m_padding);
    return {m_popupX + m_padding, y, m_popupW - 2 * m_padding, m_btnH};
}

void FilePickerPopup::scrollUp() {
    if (m_scrollOffset > 0) m_scrollOffset--;
}

void FilePickerPopup::scrollDown() {
    if (m_scrollOffset + m_visibleRows < (int)m_files.size()) m_scrollOffset++;
}

void FilePickerPopup::draw(SDL_Renderer* renderer) {
    if (!m_visible) return;

    // Dim background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, m_screenW, m_screenH};
    SDL_RenderFillRect(renderer, &full);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Popup background
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    SDL_RenderFillRect(renderer, &popup);

    // Popup border
    SDL_SetRenderDrawColor(renderer, 102, 217, 0, 255);
    SDL_RenderDrawRect(renderer, &popup);

    // Title
    if (m_font) {
        SDL_Color white = {255, 255, 255, 255};
        const char* title = m_files.empty() ? "No PGN files found" : "Select PGN File";
        SDL_Surface* surf = TTF_RenderText_Blended(m_font, title, white);
        if (surf) {
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect dst = {m_popupX + m_padding, m_popupY + 12, surf->w, surf->h};
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
        SDL_SetRenderDrawColor(renderer, 60, 60, 80, 255);
        SDL_RenderFillRect(renderer, &btn);
        SDL_SetRenderDrawColor(renderer, 100, 100, 140, 255);
        SDL_RenderDrawRect(renderer, &btn);
        if (m_font) {
            SDL_Color white = {220, 220, 220, 255};
            // Truncate filename if too long
            std::string display = m_files[fileIdx];
            if (display.size() > 40) display = display.substr(0, 37) + "...";
            SDL_Surface* surf = TTF_RenderText_Blended(m_font, display.c_str(), white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    int ty = btn.y + (btn.h - surf->h) / 2;
                    SDL_Rect dst = {btn.x + 8, ty, std::min(surf->w, btn.w - 16), surf->h};
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }
    }

    // Scroll up button
    int scrollBtnY = m_popupY + m_popupH - 48;
    SDL_Rect upBtn   = {m_popupX + m_padding,                    scrollBtnY, (m_popupW - 3*m_padding)/2, 40};
    SDL_Rect downBtn = {m_popupX + m_padding + upBtn.w + m_padding, scrollBtnY, upBtn.w, 40};

    SDL_SetRenderDrawColor(renderer, m_scrollOffset > 0 ? 80 : 50, 80, 80, 255);
    SDL_RenderFillRect(renderer, &upBtn);
    SDL_SetRenderDrawColor(renderer, 100, 120, 120, 255);
    SDL_RenderDrawRect(renderer, &upBtn);

    SDL_SetRenderDrawColor(renderer, (m_scrollOffset + m_visibleRows < (int)m_files.size()) ? 80 : 50, 80, 80, 255);
    SDL_RenderFillRect(renderer, &downBtn);
    SDL_SetRenderDrawColor(renderer, 100, 120, 120, 255);
    SDL_RenderDrawRect(renderer, &downBtn);

    if (m_font) {
        SDL_Color white = {200, 200, 200, 255};
        auto drawLabel = [&](SDL_Rect& r, const char* label) {
            SDL_Surface* s = TTF_RenderText_Blended(m_font, label, white);
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
        drawLabel(upBtn,   "^ Up");
        drawLabel(downBtn, "Down v");
    }
}

Component* FilePickerPopup::mouseEvent(SDL_Event* event) {
    if (!m_visible) return nullptr;
    if (event->type != SDL_MOUSEBUTTONDOWN) return nullptr;

    int mx = event->button.x;
    int my = event->button.y;

    // Scroll buttons
    int scrollBtnY = m_popupY + m_popupH - 48;
    SDL_Rect upBtn   = {m_popupX + m_padding,                          scrollBtnY, (m_popupW - 3*m_padding)/2, 40};
    SDL_Rect downBtn = {m_popupX + m_padding + upBtn.w + m_padding,    scrollBtnY, upBtn.w, 40};

    if (mx >= upBtn.x && mx < upBtn.x + upBtn.w && my >= upBtn.y && my < upBtn.y + upBtn.h) {
        scrollUp(); return this;
    }
    if (mx >= downBtn.x && mx < downBtn.x + downBtn.w && my >= downBtn.y && my < downBtn.y + downBtn.h) {
        scrollDown(); return this;
    }

    // File items
    for (int i = 0; i < m_visibleRows; i++) {
        int fileIdx = m_scrollOffset + i;
        if (fileIdx >= (int)m_files.size()) break;
        SDL_Rect btn = itemRect(i);
        if (mx >= btn.x && mx < btn.x + btn.w && my >= btn.y && my < btn.y + btn.h) {
            m_selectedFile = m_directory + "/" + m_files[fileIdx];
            m_visible = false;
            return this;
        }
    }

    // Click outside = dismiss
    SDL_Rect popup = {m_popupX, m_popupY, m_popupW, m_popupH};
    if (mx < popup.x || mx >= popup.x + popup.w || my < popup.y || my >= popup.y + popup.h) {
        m_visible = false;
    }
    return nullptr;
}
