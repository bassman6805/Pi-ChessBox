#ifndef FILEPICKERPOPUP_H
#define FILEPICKERPOPUP_H
#include "Component.h"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

class FilePickerPopup : public Component {
public:
    FilePickerPopup(int screenW, int screenH, TTF_Font* font);
    virtual ~FilePickerPopup() {}
    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override {}
    virtual Component* mouseEvent(SDL_Event* event) override;

    void show(const std::string& directory);
    void hide() { m_visible = false; }
    bool isVisible() const { return m_visible; }
    void setFont(TTF_Font* font) { m_font = font; }

    // Returns selected file path, or empty string if nothing selected
    std::string getSelectedFile() const { return m_selectedFile; }
    void clearSelection() { m_selectedFile = ""; }

private:
    TTF_Font* m_font;
    bool m_visible;
    std::string m_selectedFile;
    std::string m_directory;
    std::vector<std::string> m_files;   // just filenames
    int m_scrollOffset;                 // first visible item index
    int m_highlightedIndex;             // currently selected item (-1 = none)
    int m_screenW, m_screenH;

    int m_popupX, m_popupY, m_popupW, m_popupH;
    int m_btnH, m_padding;
    int m_visibleRows;

    SDL_Rect itemRect(int visibleIndex) const;
    void loadFiles();
    void scrollUp();
    void scrollDown();
};
#endif
