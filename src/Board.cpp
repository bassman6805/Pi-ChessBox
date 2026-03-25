#include "Board.h"
#include <SDL_image.h>
#include <iostream>
#include "thc.h"

Board::Board(int x, int y, int w, int h) : Component("board", x, y, w, h) {
    m_flipped = false;
    m_whiteColor = {100, 160, 210, 255};
    m_blackColor = {0, 110, 175, 255};
    for(int i=0; i<12; i++) m_pieces[i] = nullptr;
    clearHighlights();
}

Board::~Board() {
    for(int i=0; i<12; i++) if(m_pieces[i]) delete m_pieces[i];
}

void Board::loadPieces(SDL_Renderer* renderer, const char* setName) {
    const char* names[] = {"wp","wn","wb","wr","wq","wk","bp","bn","bb","br","bq","bk"};
    for(int i=0; i<12; i++) {
        char path[256];
        sprintf(path, "%s/%s.png", setName, names[i]);
        m_pieces[i] = new Sprite(renderer, path);
    }
}

void Board::playMove(const char* sanLong) {
    thc::Move mv;
    if (mv.TerseIn(&m_rules, sanLong)) {
        clearHighlights();
        // THC uses a8=0 top-left, same layout as our display array - use dst directly
        m_highlight[(int)mv.dst] = true;
        m_rules.PlayMove(mv);
    }
}

void Board::draw(SDL_Renderer *renderer) {
    drawSquares(renderer);
    drawPieces(renderer);
}

void Board::drawSquares(SDL_Renderer *renderer) {
    for (int i = 0; i < 64; i++) {
        int di = m_flipped ? (63 - i) : i;
        int row = di / 8; int col = di % 8;
        SDL_Rect rect = { m_x + (col*(m_w/8)), m_y + (row*(m_h/8)), m_w/8, m_h/8 };
        if ((row + col) % 2 == 0)
            SDL_SetRenderDrawColor(renderer, 100, 160, 210, 255);
        else
            SDL_SetRenderDrawColor(renderer, 0, 110, 175, 255);
        if (m_highlight[i])
            SDL_SetRenderDrawColor(renderer, 180, 230, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Board::drawPieces(SDL_Renderer *renderer) {
    for (int i = 0; i < 64; i++) {
        char piece = m_rules.squares[i];
        if (piece == ' ' || piece == 0) continue;
        int index = -1;
        switch (piece) {
            case 'P': index = 0; break; case 'N': index = 1; break;
            case 'B': index = 2; break; case 'R': index = 3; break;
            case 'Q': index = 4; break; case 'K': index = 5; break;
            case 'p': index = 6; break; case 'n': index = 7; break;
            case 'b': index = 8; break; case 'r': index = 9; break;
            case 'q': index = 10; break; case 'k': index = 11; break;
        }
        if (index != -1 && m_pieces[index]) {
            int di = m_flipped ? (63 - i) : i;
            SDL_Rect dest = { m_x + ((di%8)*(m_w/8)), m_y + ((di/8)*(m_h/8)), m_w/8, m_h/8 };
            m_pieces[index]->draw(renderer, &dest);
        }
    }
}

Component* Board::mouseEvent(SDL_Event* event) { return this; }

bool Board::isInCheck() {
    try {
        char kingPiece = m_rules.white ? 'K' : 'k';
        for (int i = 0; i < 64; i++) {
            if (m_rules.squares[i] == kingPiece) {
                return m_rules.AttackedSquare((thc::Square)i, !m_rules.white);
            }
        }
    } catch (...) {}
    return false;
}

std::string Board::getKingSquare() {
    char kingPiece = m_rules.white ? 'K' : 'k';
    for (int i = 0; i < 64; i++) {
        if (m_rules.squares[i] == kingPiece) {
            char f = 'a' + (i % 8);
            char r = '8' - (i / 8);
            return std::string({f, r});
        }
    }
    return "";
}
const char* Board::getFen() {
    static std::string f; f = m_rules.ForsythPublish(); return f.c_str();
}
void Board::Forsyth(const char *fen) { m_rules.Forsyth(fen); }
void Board::clearHighlights() { for(int i=0; i<64; i++) m_highlight[i] = false; }
void Board::flip(bool flip) { m_flipped = flip; }