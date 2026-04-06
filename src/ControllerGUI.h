#ifndef CONTROLLER_GUI_H
#define CONTROLLER_GUI_H

#include "Window.h"
#include "Board.h"
#include "MovesPanel.h"
#include "LevelPopup.h"
#include "DepthPopup.h"
#include "TimePopup.h"
#include "PGNLoader.h"
#include "MenuPopup.h"
#include "FilePickerPopup.h"
#include "ClockPopup.h"
#include "Connector.h"
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include "json.hpp"
#include "UCIClient.h"

class ControllerGUI : public Window {
public:
    ControllerGUI(bool isServer, const char* host, int port, const char* enginePath, const char* pgnPath);
    virtual ~ControllerGUI();

    void startGame(); 
    void initComponents();
    void exportPGN();
    void undoLastTwoMoves();
    void loadPGN();
    void loadPGNFile(const std::string& path);
    void studyStep(int delta);
    void newGame();
    std::string simSq(const std::string& sq) const;
    std::string simLan(const std::string& lan) const;
    void simSend(const nlohmann::json& j) const;
    
    virtual void draw(SDL_Renderer* renderer) override;
    virtual void update(long ticks) override;
    virtual void processButtonClicked(Button* b) override;
    virtual Component* mouseEvent(SDL_Event* event) override;

    void setFont(TTF_Font* font) { m_font = font; }

private:
    long m_engineMoveCooldown = 0;
    Board* m_board;
    MovesPanel* m_movesPanel;
    LevelPopup* m_levelPopup;
    DepthPopup* m_depthPopup;
    TimePopup*  m_timePopup;
    MenuPopup*  m_menuPopup;
    FilePickerPopup* m_filePickerPopup;
    ClockPopup*      m_clockPopup;
    TTF_Font* m_font = nullptr;
    std::string m_pendingMoveStart;
    std::string m_pendingEngineMove;
    std::string m_lastHumanMove;
    std::string m_pendingHintMove;
    std::string m_undoTargetFen;
    std::string m_undoCurrentFen;
    bool m_engineMoveRequested = false;
    std::string m_rookTargetSquare = "";
    std::string m_lastEngineTarget = "";
    std::string m_lastEngineSource = "";
    bool m_hintMode = false;
    bool m_hintJustFired = false;
    bool m_gameOver = false;
    bool m_humanIsBlack = false;
    bool m_twoPlayer = false;       // true = two humans, no engine
    long m_syncTimer = 0;
    long m_engineStartDelay = 0;

    bool isEngineToMove() const {
        if (!m_board || m_twoPlayer) return false;
        return m_humanIsBlack ? m_board->isWhiteToMove() : !m_board->isWhiteToMove();
    }

    // Undo support
    std::vector<std::string> m_fenHistory;  // FEN before each move
    struct UndoConfirm {
        bool active = false;
        std::string move1From;  // white move src (needs piece back)
        std::string move1To;    // white move dst
        std::string move2From;  // black move src (needs piece back)
        std::string move2To;    // black move dst
        bool move1Done = false;
        bool move2Done = false;
    } m_undoConfirm;

    // Study mode
    PGNLoader                m_pgnLoader;
    std::vector<std::string> m_studyFens;
    std::vector<std::string> m_studyLanMoves;
    int                      m_studyIndex = -1;
    std::string              m_studyMoveFrom; // square flashing for pickup
    std::string              m_studyMoveTo;   // square flashing for destination
    bool                     m_studyWaitingConfirm = false; // waiting for physical move

    Connector* m_connector;
    std::string m_host;
    int m_port;
    bool m_isMirroring = false;
    bool m_running;
    bool m_waitingForRook = false;

    // Chess clock
    bool  m_clockEnabled = true;
    int   m_clockPresetIndex = 3;   // default 10+5
    bool  m_clockRunning = false;
    int   m_clockIncrement = 5;      // seconds added per move
    long  m_whiteTimeMs = 10*60*1000; // 10 minutes in ms
    long  m_blackTimeMs = 10*60*1000;
    bool  m_whiteTicking = true;     // true = white's clock running
    void  drawClock(SDL_Renderer* renderer);
    void  clockStartWhite();
    void  clockStartBlack();
    void  clockStop();
    void  clockReset();
    UCIClient* m_uciClient;
};

#endif