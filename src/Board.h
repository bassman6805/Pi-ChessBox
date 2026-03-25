#ifndef BOARD_H
#define BOARD_H

#include "Component.h"
#include "Sprite.h"
#include "thc.h"
#include <SDL.h>
#include <vector>
#include <string>

class Board : public Component {
public:
    Board(int x, int y, int w, int h);
    virtual ~Board();

    // GUI Helpers
    bool isFlipped() const { return m_flipped; }
    bool isWhiteToMove() { return m_rules.white; }
    bool isHighlighted(int square) { 
        return (square >= 0 && square < 64) ? m_highlight[square] : false; 
    }

    // Logic and Rendering
    void loadPieces(SDL_Renderer* renderer, const char* setName);
    void playMove(const char* sanLong);
    const char* getFen();
    void Forsyth(const char *fen);
    void flip(bool flip);
    void clearHighlights();
    void setHighlight(int square, bool on) {
        if (square >= 0 && square < 64) m_highlight[square] = on;
    }
    void setHighlight(const char* lan, bool on) {
        if (!lan || !lan[0] || !lan[1]) return;
        int col = tolower(lan[0]) - 'a';
        int row = '8' - lan[1];
        setHighlight(row * 8 + col, on);
    }
    thc::ChessRules& getRules() { return m_rules; }
    bool isInCheck();
    bool isCheckmate() {
        try {
            if (!isInCheck()) return false;
            std::vector<thc::Move> moves;
            m_rules.GenLegalMoveList(moves);
            return moves.empty();
        } catch (...) { return false; }
    }
    std::string getKingSquare();

    void draw(SDL_Renderer* renderer) override;
    Component* mouseEvent(SDL_Event* event) override;

private:
    void drawSquares(SDL_Renderer* renderer);
    void drawPieces(SDL_Renderer* renderer);

    thc::ChessRules m_rules;
    bool m_highlight[64];
    Sprite* m_pieces[12];
    bool m_flipped;
    SDL_Color m_whiteColor, m_blackColor;
};

#endif