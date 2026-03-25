#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include "ControllerGUI.h"

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    if (TTF_Init() == -1) return 1;

    // 1. Create GUI
    ControllerGUI gui(false, "127.0.0.1", 9999, argv[1], "game.pgn");

    // 2. Load and Set Font
#ifdef _WIN32
    TTF_Font* globalFont = TTF_OpenFont("C:/Users/omart/CLionProjects/chessbox/controller-gui/assets/fonts/FreeSans.ttf", 14);
#else
    TTF_Font* globalFont = TTF_OpenFont("/home/pi/chessbox/assets/fonts/FreeSans.ttf", 14);
#endif
    if (globalFont) {
        gui.setFont(globalFont);
    }

    // 3. Init buttons AFTER font is set
    gui.initComponents();

    // 4. Run
    gui.startGame();

    if (globalFont) TTF_CloseFont(globalFont);
    TTF_Quit();
    SDL_Quit();
    return 0;
}