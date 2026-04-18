#include "ControllerGUI.h"
#include "Button.h"
#include "ImageButton.h"
#include "MenuPopup.h"
#include "MarkPopup.h"
#include "Connector.h" 
#include "UCIClient.h"
#include "json.hpp"
#include "Label.h"
#include "PGNLoader.h"
#include <iostream>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <climits>
#include <map>
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif


ControllerGUI::ControllerGUI(bool isServer, const char* host, int port, const char* enginePath, const char* pgnPath)
    : Window("ChessBox", 0, 0, 480, 800) { 
    
    m_host = (host != nullptr) ? host : "127.0.0.1";
    m_port = port;
    m_running = true;

    // 1. Initialize Stockfish (UCIClient constructor already calls sendUCINewGame - do NOT call it again)
    std::string pathStr = (enginePath != nullptr) ? enginePath : "C:\\chess\\stockfish.exe";
    m_uciClient = new UCIClient(pathStr);

    // 2. Components
    m_board = new Board(0, 0, 480, 480);
    m_movesPanel = new MovesPanel("moves", 0, 514, 480, 222, nullptr);
    m_levelPopup = new LevelPopup(480, 800, nullptr);
    m_depthPopup = new DepthPopup(480, 800, nullptr);
    m_timePopup  = new TimePopup(480, 800, nullptr);
    m_menuPopup  = new MenuPopup(480, 800, nullptr);
    
    addComponent(m_board);
    addComponent(m_movesPanel);
    
    m_connector = new Connector();
    m_pendingMoveStart = "";
    m_filePickerPopup = new FilePickerPopup(480, 800, nullptr);
    m_clockPopup = new ClockPopup(480, 800, nullptr);
    m_markPopup  = new MarkPopup(480, 800, nullptr); 
}

ControllerGUI::~ControllerGUI() {
    delete m_board;
    delete m_movesPanel;
    delete m_levelPopup;
    delete m_depthPopup;
    delete m_timePopup;
    delete m_menuPopup;
    delete m_uciClient;
    delete m_connector;
    delete m_filePickerPopup;
    delete m_clockPopup;
    delete m_markPopup;
}

void ControllerGUI::initComponents() {
    m_buttons.clear();

    if (m_movesPanel && m_font) m_movesPanel->setFont(m_font);
    if (m_levelPopup && m_font) m_levelPopup->setFont(m_font);
    if (m_depthPopup && m_font) m_depthPopup->setFont(m_font);
    if (m_timePopup  && m_font) m_timePopup->setFont(m_font);
    if (m_menuPopup  && m_font) m_menuPopup->setFont(m_font);
    if (m_filePickerPopup && m_font) m_filePickerPopup->setFont(m_font);
    if (m_clockPopup      && m_font) m_clockPopup->setFont(m_font);
    if (m_markPopup       && m_font) m_markPopup->setFont(m_font);

    // Asset path - works on both Windows and Pi
#ifdef _WIN32
    const char* assetBase  = "C:/Users/omart/CLionProjects/chessbox/controller-gui/assets/";
    const char* piecesPath = "C:/Users/omart/CLionProjects/chessbox/controller-gui/assets/pieces/merida_new";
#else
    const char* assetBase  = "/home/pi/chessbox/assets/";
    const char* piecesPath = "/home/pi/chessbox/assets/pieces/merida_new";
#endif

    if (m_board && m_renderer) {
        m_board->loadPieces(m_renderer, piecesPath);
        m_board->Forsyth("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
    auto imgPath = [&](const char* file) -> std::string { return std::string(assetBase) + file; };
    std::map<std::string, std::string> imageButtons = {
        {"B_P",  imgPath("button-player-black.png")},
        {"W_P",  imgPath("button-player-white.png")},
        {"WB_P", imgPath("button-twoplayer-whiteblack.png")},
        {"BB_P", imgPath("button-twoplayer-blackwhite.png")},
        {"<<",   imgPath("button-fastback.png")},
        {"<",    imgPath("button-back.png")},
        {">",    imgPath("button-fwd.png")},
    };

    // Build menu popup with all settings/utility buttons
    if (m_menuPopup && m_renderer) {
        char levelLabel[16], depthLabel[16], timeLabel[16];
        snprintf(levelLabel, sizeof(levelLabel), "Lvl:%d", m_uciClient ? m_uciClient->getSkillLevel() : 5);
        int curDepth = m_uciClient ? m_uciClient->getDepth() : 0;
        if (curDepth > 0) snprintf(depthLabel, sizeof(depthLabel), "Dpt:%d", curDepth);
        else              snprintf(depthLabel, sizeof(depthLabel), "Dpt:T");
        int curTime = m_uciClient ? m_uciClient->getMoveTime() : 3000;
        if (curTime == 0)        snprintf(timeLabel, sizeof(timeLabel), "T:None");
        else if (curTime < 1000) snprintf(timeLabel, sizeof(timeLabel), "T:0.5s");
        else                     snprintf(timeLabel, sizeof(timeLabel), "T:%ds", curTime / 1000);
        m_menuPopup->buildButtons(m_renderer, imageButtons, assetBase,
                                  levelLabel, "Back", depthLabel, timeLabel);
    }

    // Bottom row: Connect, Flip, Hint, Inspect, Menu, <<, <, >
    SDL_Color limeGreen = {22, 35, 52, 255};  // dark blue-gray matching reference
    const char* bottomLabels[] = { "Menu", "Connect", "New", "Hint", "<<", "<", ">" };
    int bW = 55, bH = 59, bPad = 4;
    int totalRowW = 7 * bW + 8 * bPad;
    int startX = (480 - totalRowW) / 2;
    int bRowY = 740;
    for (int i = 0; i < 7; i++) {
        int bx = startX + bPad + i * (bW + bPad);
        std::string lid = bottomLabels[i];
        Button* btn;
        if (imageButtons.count(lid) && m_renderer) {
            ImageButton* ib = new ImageButton(lid.c_str(), imageButtons[lid].c_str(), bx, bRowY, bW, bH);
            ib->loadImage(m_renderer);
            btn = ib;
        } else {
            TextButton* tb = new TextButton(lid.c_str(), lid.c_str(), bx, bRowY, bW, bH, m_font);
            tb->setBackgroundColor(limeGreen);
            btn = tb;
        }
        addButton(btn);
    }

    // --- LABELS ---
    SDL_Color white = {255, 255, 255, 255};
    auto createLabel = [&](std::string id, std::string text, int x, int y) {
        Label* l = new Label(id, x, y, 220, 30);
        l->setText(text.c_str()); 
        l->setFont(m_font);
        l->setColor(white);
        addComponent(l);
    };
}

// Mirror a FEN string for physical board when playing as black
// Reverses both rank order and file order, swaps piece colors
// This maps chess coordinates to physical coordinates when board is flipped
static std::string mirrorFen(const std::string& fen) {
    std::string pos, rest;
    size_t sp = fen.find(' ');
    if (sp == std::string::npos) { pos = fen; rest = " b - - 0 1"; }
    else { pos = fen.substr(0, sp); rest = fen.substr(sp); }
    // Split into ranks
    std::vector<std::string> ranks;
    std::string rank;
    for (char c : pos) {
        if (c == '/') { ranks.push_back(rank); rank = ""; }
        else rank += c;
    }
    ranks.push_back(rank);
    // Reverse rank order AND file order within each rank, then swap colors
    std::reverse(ranks.begin(), ranks.end());
    // Expand each rank to 8 squares, reverse, then compress back
    std::string mirrored;
    for (size_t i = 0; i < ranks.size(); i++) {
        if (i > 0) mirrored += '/';
        // Expand rank
        std::string expanded;
        for (char c : ranks[i]) {
            if (c >= '1' && c <= '8') { for(int j=0;j<c-'0';j++) expanded += '.'; }
            else expanded += c;
        }
        // Reverse files
        std::reverse(expanded.begin(), expanded.end());
        // Compress back and swap colors
        int empty = 0;
        for (char c : expanded) {
            if (c == '.') { empty++; }
            else {
                if (empty > 0) { mirrored += (char)('0'+empty); empty = 0; }
                if (c >= 'a' && c <= 'z') mirrored += (char)(c-32);
                else if (c >= 'A' && c <= 'Z') mirrored += (char)(c+32);
                else mirrored += c;
            }
        }
        if (empty > 0) mirrored += (char)('0'+empty);
    }
    // Flip side to move
    char side = (rest.size()>1 && rest[1]=='w') ? 'b' : 'w';
    return mirrored + " " + side + " - - 0 1";
}

void ControllerGUI::update(long ticks) {
    Window::update(ticks);
    if (m_menuPopup) m_menuPopup->update(ticks);
    if (m_filePickerPopup) m_filePickerPopup->update(ticks);
    // Chess clock tick
    if (m_clockEnabled && m_clockRunning) {
        if (m_whiteTicking) {
            m_whiteTimeMs -= ticks;
            if (m_whiteTimeMs < 0) m_whiteTimeMs = 0;
            if (m_whiteTimeMs == 0 && !m_gameOver) {
                m_gameOver = true;
                m_clockRunning = false;
                std::string kSq;
                { const char* fen=m_board->getFen(); thc::ChessRules cr; cr.Forsyth(fen); for(int i=0;i<64;i++){if(cr.squares[i]=='K'){char f='a'+(i%8);char r='8'-(i/8);kSq=std::string({f,r});break;}} }
                if (!kSq.empty()) {
                    m_board->setHighlight(kSq.c_str(), true);
                    simSend(nlohmann::json{{"action","flash"},{"on",true},{"timeout",true},{"squares",nlohmann::json::array({kSq})}});
                }
                fprintf(stderr, "WHITE FLAG!\n");
            }
        } else {
            m_blackTimeMs -= ticks;
            if (m_blackTimeMs < 0) m_blackTimeMs = 0;
            if (m_blackTimeMs == 0 && !m_gameOver) {
                m_gameOver = true;
                m_clockRunning = false;
                std::string kSq;
                { const char* fen=m_board->getFen(); thc::ChessRules cr; cr.Forsyth(fen); for(int i=0;i<64;i++){if(cr.squares[i]=='k'){char f='a'+(i%8);char r='8'-(i/8);kSq=std::string({f,r});break;}} }
                if (!kSq.empty()) {
                    m_board->setHighlight(kSq.c_str(), true);
                    simSend(nlohmann::json{{"action","flash"},{"on",true},{"timeout",true},{"squares",nlohmann::json::array({kSq})}});
                }
                fprintf(stderr, "BLACK FLAG!\n");
            }
        }
    }

    static long lockoutTimer = 0;
    if (lockoutTimer > 0) { lockoutTimer -= ticks; if (lockoutTimer < 0) lockoutTimer = 0; }
    m_syncTimer += ticks;

    // Delayed engine start (after new game as black, wait for Stockfish readyok)
    if (m_engineStartDelay > 0) {
        m_engineStartDelay -= ticks;
        if (m_engineStartDelay <= 0) {
            m_engineStartDelay = 0;
            m_uciClient->setPosition(m_board->getFen());
            m_uciClient->sendGo(m_uciClient->getMoveTime());
            fprintf(stderr, "ENGINE START: firing go after delay\n");
        }
    }

    // --- HEARTBEAT SYNC ---
    // Skip while piece is in hand so sim LEDs showing legal moves aren't overwritten
    if (m_syncTimer > 400 && m_pendingMoveStart.empty()) {
        m_syncTimer = 0;
        if (m_connector && m_connector->isConnected() && m_board) {
            try {
            if (m_gameOver) {
                // Game over - no sync needed, flash already sent once
            } else {
                nlohmann::json jSync = {{"action","highlight"},{"color","green"},{"squares",nlohmann::json::array()}};
                for (int i = 0; i < 64; i++) {
                    if (m_board->isHighlighted(i)) {
                        char f = 'a' + (i % 8);
                        char r = '8' - (i / 8);
                        jSync["squares"].push_back(std::string({f, r}));
                    }
                }
                // disabled for hardware: try { m_connector->send((jSync.dump() + "\n").c_str()); } catch (...) {}
            }
            } catch (...) { fprintf(stderr, "HEARTBEAT exception\n"); }
        }
    }

    // --- MESSAGE PROCESSING ---
    if (m_connector && m_connector->isConnected()) {
        char buffer[1024];
        int _rln = 0;
        try { _rln = m_connector->readline(buffer, sizeof(buffer)); } catch (...) { m_connector->close(); _rln = 0; }
        if (_rln > 0) fprintf(stderr, "READLINE: [%s]\n", buffer);
        while (_rln > 0) {
            try {
                nlohmann::json msg = nlohmann::json::parse(buffer);
                std::string action = msg.value("action", "");
                fprintf(stderr, "JSON ACTION: [%s]\n", action.c_str());
                // Handle takeback completion format before early break
                if (action.empty() && msg.contains("from") && msg.contains("to") && msg.value("type","") == "move") {
                    // Check if this is engine move confirmation when playing as black
                    if (!m_undoConfirm.active && m_humanIsBlack && !m_pendingEngineMove.empty()) {
                        std::string from = msg.value("from", "");
                        std::string to   = msg.value("to", "");
                        for(auto& ch : from) ch = tolower(ch);
                        for(auto& ch : to)   ch = tolower(ch);
                        std::string lan = from + to;
                        std::string flipped = simLan(m_pendingEngineMove);
                        fprintf(stderr, "BLACK CONFIRM: lan=%s flipped=%s pending=%s\n", lan.c_str(), flipped.c_str(), m_pendingEngineMove.c_str());
                        if (lan == flipped || lan == m_pendingEngineMove) {
                            fprintf(stderr, "ENGINE MOVE CONFIRMED (black mode): %s\n", m_pendingEngineMove.c_str());
                            if (m_clockEnabled && m_clockRunning) {
                                if (m_humanIsBlack) { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = false; }
                                else { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                            }
                            m_pendingEngineMove = "";
                            m_board->clearHighlights();
                            lockoutTimer = 0;
                            break;
                        }
                    }
                }
                if (action.empty() && msg.contains("from") && msg.contains("to") && msg.value("type","") == "move" && m_undoConfirm.active) {
                    std::string from = msg.value("from", "");
                    std::string to   = msg.value("to", "");
                    for(auto& ch : from) ch = tolower(ch);
                    for(auto& ch : to)   ch = tolower(ch);
                    std::string lan = from + to;
                    fprintf(stderr, "TAKEBACK RESPONSE: lan=%s\n", lan.c_str());
                    if (!m_undoConfirm.move1Done && lan == m_undoConfirm.move1From + m_undoConfirm.move1To) {
                        m_undoConfirm.move1Done = true;
                        m_board->clearHighlights();
                        if (!m_undoConfirm.move2Done) {
                            m_board->setHighlight(m_undoConfirm.move2From.c_str(), true);
                            m_board->setHighlight(m_undoConfirm.move2To.c_str(), true);
                            nlohmann::json hint; hint["action"]="takeback"; hint["from"]=m_undoConfirm.move2From; hint["to"]=m_undoConfirm.move2To; simSend(hint);
                            fprintf(stderr, "UNDO: now takeback black %s->%s\n", m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str());
                        } else {
                            m_undoConfirm.active = false;
                            if (m_connector && m_connector->isConnected()) { try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){} }
                            fprintf(stderr, "UNDO: complete\n");
                        }
                    } else if (!m_undoConfirm.move2Done && lan == m_undoConfirm.move2From + m_undoConfirm.move2To) {
                        m_undoConfirm.move2Done = true;
                        m_board->clearHighlights();
                        if (!m_undoConfirm.move1Done) {
                            m_board->setHighlight(m_undoConfirm.move1From.c_str(), true);
                            m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
                            nlohmann::json hint; hint["action"]="takeback"; hint["from"]=m_undoConfirm.move1From; hint["to"]=m_undoConfirm.move1To; simSend(hint);
                            fprintf(stderr, "UNDO: now takeback white %s->%s\n", m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str());
                        } else {
                            m_undoConfirm.active = false;
                            if (m_connector && m_connector->isConnected()) { try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){} }
                            fprintf(stderr, "UNDO: complete\n");
                        }
                    }
                    break;
                }
                if (action.empty() || action == "pieceUp" || action == "pieceDown") break;

                // Handle takeback completion format: {"from":"E5","to":"E4","type":"move"} (no action field)
                if (action.empty() && msg.contains("from") && msg.contains("to") && msg.value("type","") == "move") {
                    std::string from = msg.value("from", "");
                    std::string to   = msg.value("to", "");
                    // Convert to lowercase
                    for(auto& ch : from) ch = tolower(ch);
                    for(auto& ch : to)   ch = tolower(ch);
                    std::string lan = from + to;
                    fprintf(stderr, "TAKEBACK RESPONSE: lan=%s\n", lan.c_str());
                    if (m_undoConfirm.active) {
                        fprintf(stderr, "UNDO CHECK takeback: lan=%s move1=%s%s move2=%s%s\n", lan.c_str(), m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str(), m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str());
                        if (!m_undoConfirm.move1Done && lan == m_undoConfirm.move1From + m_undoConfirm.move1To) {
                            m_undoConfirm.move1Done = true;
                            fprintf(stderr, "UNDO: white piece moved back %s\n", lan.c_str());
                            m_board->clearHighlights();
                            if (!m_undoConfirm.move2Done) {
                                m_board->setHighlight(m_undoConfirm.move2From.c_str(), true);
                                m_board->setHighlight(m_undoConfirm.move2To.c_str(), true);
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = m_undoConfirm.move2From;
                                hint["to"] = m_undoConfirm.move2To;
                                simSend(hint);
                                fprintf(stderr, "UNDO: now takeback black %s->%s\n", m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str());
                            } else {
                                m_undoConfirm.active = false;
                                if (m_connector && m_connector->isConnected()) {
                                    try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){}
                                }
                                fprintf(stderr, "UNDO: complete\n");
                            }
                        } else if (!m_undoConfirm.move2Done && lan == m_undoConfirm.move2From + m_undoConfirm.move2To) {
                            m_undoConfirm.move2Done = true;
                            fprintf(stderr, "UNDO: black piece moved back %s\n", lan.c_str());
                            m_board->clearHighlights();
                            if (!m_undoConfirm.move1Done) {
                                m_board->setHighlight(m_undoConfirm.move1From.c_str(), true);
                                m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = m_undoConfirm.move1From;
                                hint["to"] = m_undoConfirm.move1To;
                                simSend(hint);
                                fprintf(stderr, "UNDO: now takeback white %s->%s\n", m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str());
                            } else {
                                m_undoConfirm.active = false;
                                if (m_connector && m_connector->isConnected()) {
                                    try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){}
                                }
                                fprintf(stderr, "UNDO: complete\n");
                            }
                        }
                    }
                    break;
                }
                // Handle cbcontroller hardware format: {"action":"move","moves":[{"lan":"e2e4",...}]}
                if (action == "move" && msg.contains("moves") && msg["moves"].is_array() && !msg["moves"].empty()) {
                    std::string lan  = msg["moves"][0].value("lan", "");
                    std::string from = msg["moves"][0].value("from", "");
                    // Check if this is castle rook completion
                    // Accept any castle rook move (coordinates vary by orientation)
                    bool isCastleRookMove = m_waitingForRook && (
                        lan=="h1f1"||lan=="a1d1"||  // white kingside/queenside
                        lan=="h8f8"||lan=="a8d8"||  // black kingside/queenside (normal)
                        lan=="a8c8"||lan=="h8d8"    // black kingside/queenside (flipped coords)
                    );
                    if (isCastleRookMove) {
                        fprintf(stderr, "CASTLE ROOK MOVE confirmed: %s\n", lan.c_str());
                        m_waitingForRook = false;
                        if (m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jpos;
                                jpos["action"] = "setposition";
                                jpos["fen"] = std::string(m_board->getFen());
                                m_connector->send((jpos.dump() + "\r\n").c_str());
                            } catch (...) {}
                        }
                        if (!m_pendingEngineMove.empty()) {
                            nlohmann::json hint;
                            hint["action"] = "hint";
                            hint["from"] = m_pendingEngineMove.substr(0,2);
                            hint["to"]   = m_pendingEngineMove.substr(2,2);
                            simSend(hint);
                        }
                        if (m_engineMoveRequested && !m_gameOver) {
                            m_uciClient->setPosition(m_board->getFen());
                            m_uciClient->sendGo(m_uciClient->getMoveTime());
                        }
                        continue;
                    }
                    fprintf(stderr, "HW MOVE CHECK: lan=%s engineToMove=%d lockout=%ld\n", lan.c_str(), isEngineToMove()?1:0, lockoutTimer);
                    // Check if this is physical confirmation of engine move
                    fprintf(stderr, "HW MOVE pending=[%s] lan=[%s] match=%d\n", m_pendingEngineMove.c_str(), lan.c_str(), (lan==m_pendingEngineMove)?1:0);
                    // Also check flipped lan for when playing as black
                    std::string flippedPending = m_humanIsBlack ? simLan(m_pendingEngineMove) : m_pendingEngineMove;
                    if (!lan.empty() && !m_pendingEngineMove.empty() && (lan == m_pendingEngineMove || lan == flippedPending)) {
                        fprintf(stderr, "ENGINE MOVE CONFIRMED physically via HW move: %s\n", lan.c_str());
                        // Switch clock after engine move confirmed
                        if (m_clockEnabled && m_clockRunning) {
                            if (m_humanIsBlack) { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = false; }
                            else { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                        }
                        m_pendingEngineMove = "";
                        m_board->clearHighlights();
                        lockoutTimer = 0;
                        // Send setposition to resync cbcontroller after engine move confirmed
                        if (m_humanIsBlack && m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jp;
                                jp["action"] = "setposition";
                                jp["fen"] = std::string(m_board->getFen());
                                m_connector->send((jp.dump() + "\r\n").c_str());
                                fprintf(stderr, "RESYNC after engine confirm: %s\n", m_board->getFen());
                            } catch (...) {}
                        }
                        break;
                    }
                    // Handle undo confirmation via HW move
                    if (m_undoConfirm.active && !lan.empty()) {
                        fprintf(stderr, "UNDO CHECK: lan=%s move1=%s%s move2=%s%s done1=%d done2=%d\n", lan.c_str(), m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str(), m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str(), m_undoConfirm.move1Done?1:0, m_undoConfirm.move2Done?1:0);
                        if (!m_undoConfirm.move1Done && lan == m_undoConfirm.move1From + m_undoConfirm.move1To) {
                            m_undoConfirm.move1Done = true;
                            fprintf(stderr, "UNDO: white piece moved back %s\n", lan.c_str());
                            m_board->clearHighlights();
                            if (!m_undoConfirm.move2Done) {
                                m_board->setHighlight(m_undoConfirm.move2From.c_str(), true);
                                m_board->setHighlight(m_undoConfirm.move2To.c_str(), true);
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = m_undoConfirm.move2From;
                                hint["to"] = m_undoConfirm.move2To;
                                simSend(hint);
                                fprintf(stderr, "UNDO: now hint black %s->%s\n", m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str());
                            } else {
                                m_undoConfirm.active = false;
                                if (m_connector && m_connector->isConnected()) {
                                    try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){}
                                }
                                fprintf(stderr, "UNDO: complete\n");
                            }
                        } else if (!m_undoConfirm.move2Done && lan == m_undoConfirm.move2From + m_undoConfirm.move2To) {
                            m_undoConfirm.move2Done = true;
                            fprintf(stderr, "UNDO: black piece moved back %s\n", lan.c_str());
                            m_board->clearHighlights();
                            if (!m_undoConfirm.move1Done) {
                                m_board->setHighlight(m_undoConfirm.move1From.c_str(), true);
                                m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = m_undoConfirm.move1From;
                                hint["to"] = m_undoConfirm.move1To;
                                simSend(hint);
                                fprintf(stderr, "UNDO: now hint white %s->%s\n", m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str());
                            } else {
                                m_undoConfirm.active = false;
                                if (m_connector && m_connector->isConnected()) {
                                    try { nlohmann::json jp; jp["action"]="setposition"; jp["fen"]=m_undoTargetFen; m_connector->send((jp.dump()+"\r\n").c_str()); } catch(...){}
                                }
                                fprintf(stderr, "UNDO: complete\n");
                            }
                        }
                        break;
                    }
                    if (!lan.empty() && !isEngineToMove() && lockoutTimer == 0 && m_board) {
                        fprintf(stderr, "HW MOVE: lan=%s\n", lan.c_str());
                        m_pendingHintMove = "";
                        m_fenHistory.push_back(std::string(m_board->getFen())); // save pre-move FEN
                        m_board->playMove(lan.c_str());
                        if (m_movesPanel) m_movesPanel->addMove(lan.c_str());
                        m_pendingMoveStart = "";
                        lockoutTimer = 800;
                        m_lastHumanMove = lan;
                        // Switch clock after human move
                        if (m_clockEnabled && m_clockRunning) {
                            fprintf(stderr, "CLOCK HUMAN MOVE: whiteTicking=%d humanIsBlack=%d\n", m_whiteTicking?1:0, m_humanIsBlack?1:0);
                            if (m_whiteTicking) { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = false; }
                            else                { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                        }
                        // Detect castle by king LAN (normal and flipped board coordinates)
                        if (lan=="e1g1"||lan=="e1c1"||lan=="e8g8"||lan=="e8c8"||
                            lan=="d1b1"||lan=="d1f1"||lan=="d8b8"||lan=="d8f8") {
                            m_waitingForRook = true;
                            // Send hint showing where rook needs to go
                            std::string rookFrom, rookTo;
                            if (lan=="e1g1") { rookFrom="h1"; rookTo="f1"; }
                            else if (lan=="e1c1") { rookFrom="a1"; rookTo="d1"; }
                            else if (lan=="e8g8") { rookFrom="h8"; rookTo="f8"; }
                            else if (lan=="e8c8") { rookFrom="a8"; rookTo="d8"; }
                            else if (lan=="d1b1") { rookFrom="h1"; rookTo="f1"; }
                            else if (lan=="d1f1") { rookFrom="a1"; rookTo="d1"; }
                            else if (lan=="d8b8") { rookFrom="h8"; rookTo="f8"; }
                            else if (lan=="d8f8") { rookFrom="a8"; rookTo="d8"; }
                            if (!rookFrom.empty() && m_connector && m_connector->isConnected()) {
                                try {
                                    nlohmann::json jr;
                                    jr["action"] = "hint";
                                    jr["from"] = rookFrom;
                                    jr["to"] = rookTo;
                                    jr["capture"] = false;
                                    m_connector->send((jr.dump()+"\r\n").c_str());
                                } catch(...) {}
                            }
                        }
                        fprintf(stderr, "HUMAN MOVE: %s sent, requesting engine move waitingForRook=%d\n", lan.c_str(), m_waitingForRook?1:0);
                        // Send updated position to controller after HW move
                        if (!m_waitingForRook && m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jp;
                                jp["action"] = "setposition";
                                jp["fen"] = std::string(m_board->getFen());
                                m_connector->send((jp.dump() + "\r\n").c_str());
                                fprintf(stderr, "HW POST-MOVE SETPOSITION: %s\n", m_board->getFen());
                            } catch (...) {}
                        }
                        if (!m_twoPlayer && isEngineToMove()) {
                            m_engineMoveRequested = true;
                            if (m_hintJustFired) { m_hintJustFired = false; m_uciClient->sendUCINewGame(); }
                            m_uciClient->setPosition(m_board->getFen());
                            if (!m_waitingForRook) m_uciClient->sendGo(m_uciClient->getMoveTime());
                        }
                    }
                    break; // processed HW move, exit loop for this tick
                }

                // piece_up always processed immediately (lockout does not apply)
                if (action == "piece_up") {
                    std::string lan = msg.value("lan", "");
                    fprintf(stderr, "PIECE_UP: lan=%s lockout=%ld hintMode=%d\n", lan.c_str(), lockoutTimer, m_hintMode ? 1 : 0);

                    // Ignore all piece events during study mode
                    if (m_studyIndex >= 0) continue;

                    // During undo mode
                    if (m_undoConfirm.active) {
                        if (!m_undoConfirm.move1Done && lan == m_undoConfirm.move1From) {
                            // White piece lifted — show destination
                            m_board->clearHighlights();
                            m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
                            try {
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = lan;
                                hint["to"] = m_undoConfirm.move1To;
                                simSend(hint);
                            } catch (...) {}
                            m_pendingMoveStart = lan;
                            fprintf(stderr, "UNDO: white piece lifted from %s, flash dest %s\n",
                                lan.c_str(), m_undoConfirm.move1To.c_str());
                        } else if (!m_undoConfirm.move2Done && lan == m_undoConfirm.move2From) {
                            // Black piece lifted — show destination
                            m_board->clearHighlights();
                            m_board->setHighlight(m_undoConfirm.move2To.c_str(), true);
                            try {
                                nlohmann::json hint;
                                hint["action"] = "takeback";
                                hint["from"] = lan;
                                hint["to"] = m_undoConfirm.move2To;
                                simSend(hint);
                            } catch (...) {}
                            m_pendingMoveStart = lan;
                            fprintf(stderr, "UNDO: black piece lifted from %s, flash dest %s\n",
                                lan.c_str(), m_undoConfirm.move2To.c_str());
                        }
                        // Any other lift during undo — ignore
                        continue;
                    }

                    try { simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}}); } catch (...) {}

                    // Check for engine move confirmation
                    if (!m_pendingEngineMove.empty()) {
                        std::string engineTo = m_pendingEngineMove.substr(2, 2);
                        if (lan == engineTo) {
                            fprintf(stderr, "ENGINE MOVE CONFIRMED (piece_up at dst): %s\n", m_pendingEngineMove.c_str());
                            if (m_clockEnabled && m_clockRunning) {
                                if (m_humanIsBlack) { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = false; }
                                else { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                            }
                            m_pendingEngineMove = "";
                            m_board->clearHighlights();
                            m_pendingMoveStart = "";
                            lockoutTimer = 0;
                            continue;
                        }
                        if (!m_humanIsBlack) {
                            // Playing white — must confirm engine's black move first
                            m_pendingMoveStart = "";
                            continue;
                        }
                        // Playing black — engine move pending but human picking up their piece
                        // Clear engine move and proceed as normal black move
                        m_pendingEngineMove = "";
                        m_board->clearHighlights();
                    }
                    m_pendingMoveStart = lan;

                    // Highlight source square on GUI
                    m_board->clearHighlights();
                    m_board->setHighlight(lan.c_str(), true);

                    // Send legal move destinations to sim LEDs only
                    {
                        nlohmann::json jHL = {{"action","highlight"},{"color","green"},{"squares",nlohmann::json::array()}};
                        thc::ChessRules& rules = m_board->getRules();
                        std::vector<thc::Move> legalMoves;
                        rules.GenLegalMoveList(legalMoves);
                        for (auto& mv : legalMoves) {
                            if (mv.TerseOut().substr(0, 2) == lan)
                                jHL["squares"].push_back(mv.TerseOut().substr(2, 2));
                        }
                        fprintf(stderr, "HIGHLIGHT SEND: %s\n", jHL.dump().c_str());
                        // disabled for hardware: try { m_connector->send((jHL.dump() + "\n").c_str()); } catch (...) {}
                    }
                    continue;
                }

                // All other actions are guarded by lockout
                if (lockoutTimer > 0) continue;

                if (action == "piece_down") {
                    std::string lan = msg.value("lan", "");
                    fprintf(stderr, "PIECE_DOWN: lan=%s pendingStart=%s pendingEngine=%s\n",
                        lan.c_str(), m_pendingMoveStart.c_str(), m_pendingEngineMove.c_str());

                    // Ignore piece events during study mode
                    if (m_studyIndex >= 0) continue;

                    if (m_gameOver) { m_pendingMoveStart = ""; continue; }

                    if (!m_pendingMoveStart.empty() && m_pendingMoveStart != lan) {
                        std::string fullMove = m_pendingMoveStart + lan;

                        if (!m_pendingEngineMove.empty() && !m_humanIsBlack) {
                            std::string engineFrom = m_pendingEngineMove.substr(0, 2);
                            std::string engineTo   = m_pendingEngineMove.substr(2, 2);
                            if (m_pendingMoveStart == engineFrom && lan == engineTo) {
                                fprintf(stderr, "ENGINE MOVE CONFIRMED physically: %s\n", m_pendingEngineMove.c_str());
                                if (m_clockEnabled && m_clockRunning) {
                                    if (m_humanIsBlack) { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                                    else { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                                }
                                m_pendingEngineMove = "";
                                m_board->clearHighlights();
                            } else {
                                // Human move while engine move pending
                                m_board->playMove(fullMove.c_str());
                                if (m_movesPanel) m_movesPanel->addMove(fullMove.c_str());
                                m_uciClient->setPosition(m_board->getFen());
                                try {
                                    simSend(nlohmann::json{{"action","move"},{"lan",fullMove}});

                                } catch (...) {}
                                lockoutTimer = 800;
                                fprintf(stderr, "HUMAN MOVE: %s sent\n", fullMove.c_str());
                                if (isEngineToMove()) {
                                    m_engineMoveRequested = true;
                                    if (m_hintJustFired) { m_hintJustFired = false; m_uciClient->sendUCINewGame(); }
                                    m_uciClient->setPosition(m_board->getFen());
                                    m_uciClient->sendGo(1000);
                                }
                            }
                        } else {
                            // Normal human move
                            // When playing black, clear any pending engine move (already confirmed via piece_up)
                            if (m_humanIsBlack && !m_pendingEngineMove.empty()) {
                                m_pendingEngineMove = "";
                                m_board->clearHighlights();
                            }
                            // Check if this is an undo confirmation
                            if (m_undoConfirm.active) {
                                if (!m_undoConfirm.move1Done && lan == m_undoConfirm.move1To) {
                                    m_undoConfirm.move1Done = true;
                                    fprintf(stderr, "UNDO: white piece placed at %s\n", lan.c_str());
                                    // Tell sim to move the piece
                                    try { simSend(nlohmann::json{{"action","move"},
                                        {"lan", m_undoConfirm.move1From + m_undoConfirm.move1To}
                                    }); } catch (...) {}
                                    m_board->clearHighlights();
                                    if (!m_undoConfirm.move2Done) {
                                        // Show black piece hint
                                        m_board->setHighlight(m_undoConfirm.move2From.c_str(), true);
                                        m_board->setHighlight(m_undoConfirm.move2To.c_str(), true);
                                        try {
                                            nlohmann::json hint;
                                            hint["action"] = "takeback";
                                            hint["from"] = m_undoConfirm.move2From;
                                            hint["to"] = m_undoConfirm.move2To;
                                            simSend(hint);
                                        } catch (...) {}
                                        fprintf(stderr, "UNDO: now hint black piece at %s->%s\n", m_undoConfirm.move2From.c_str(), m_undoConfirm.move2To.c_str());
                                    } else {
                                        m_undoConfirm.active = false;
                                        // Reset cbcontroller board
                                        if (m_connector && m_connector->isConnected()) {
                                            try {
                                                nlohmann::json jpos;
                                                jpos["action"] = "setposition";
                                                jpos["fen"] = m_undoTargetFen;
                                                m_connector->send((jpos.dump() + "\r\n").c_str());
                                            } catch (...) {}
                                        }
                                        fprintf(stderr, "UNDO: complete\n");
                                    }
                                } else if (!m_undoConfirm.move2Done && lan == m_undoConfirm.move2To) {
                                    m_undoConfirm.move2Done = true;
                                    fprintf(stderr, "UNDO: black piece placed at %s\n", lan.c_str());
                                    try { simSend(nlohmann::json{{"action","move"},
                                        {"lan", m_undoConfirm.move2From + m_undoConfirm.move2To}
                                    }); } catch (...) {}
                                    m_board->clearHighlights();
                                    if (!m_undoConfirm.move1Done) {
                                        // Show white piece hint
                                        m_board->setHighlight(m_undoConfirm.move1From.c_str(), true);
                                        m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
                                        try {
                                            nlohmann::json hint;
                                            hint["action"] = "takeback";
                                            hint["from"] = m_undoConfirm.move1From;
                                            hint["to"] = m_undoConfirm.move1To;
                                            simSend(hint);
                                        } catch (...) {}
                                        fprintf(stderr, "UNDO: now hint white piece at %s->%s\n", m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str());
                                    } else {
                                        m_undoConfirm.active = false;
                                        // Reset cbcontroller board
                                        if (m_connector && m_connector->isConnected()) {
                                            try {
                                                nlohmann::json jpos;
                                                jpos["action"] = "setposition";
                                                jpos["fen"] = m_undoTargetFen;
                                                m_connector->send((jpos.dump() + "\r\n").c_str());
                                            } catch (...) {}
                                        }
                                        fprintf(stderr, "UNDO: complete\n");
                                    }
                                }
                                // Wrong square or already done — ignore
                                m_pendingMoveStart = "";
                                continue;
                            }

                            // Save FEN before move for undo
                            m_fenHistory.push_back(std::string(m_board->getFen()));

                            m_board->playMove(fullMove.c_str());
                            if (m_movesPanel) m_movesPanel->addMove(fullMove.c_str());
                            m_uciClient->setPosition(m_board->getFen());
                            try {
                                simSend(nlohmann::json{{"action","move"},{"lan",fullMove}});

                                if (m_board->isCheckmate()) {
                                    m_gameOver = true;
                                    std::string kSq = m_board->getKingSquare();
                                    m_board->clearHighlights();
                                    if (!kSq.empty()) {
                                        m_board->setHighlight(kSq.c_str(), true);
                                        simSend(nlohmann::json{{"action","flash"},{"on",true},{"mate",true},{"squares",nlohmann::json::array({kSq})}});
                                    }
                                    fprintf(stderr, "CHECKMATE!\n");
                                } else if (m_board->isInCheck()) {
                                    std::string kSq = m_board->getKingSquare();
                                    if (!kSq.empty())
                                        simSend(nlohmann::json{{"action","flash"},{"on",true},{"squares",nlohmann::json::array({kSq})}});
                                } else {
                                    simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}});
                                }
                            } catch (...) {}
                            lockoutTimer = 800;
                            bool isCastle = (fullMove=="e1g1"||fullMove=="e1c1"||fullMove=="e8g8"||fullMove=="e8c8"||
                                              fullMove=="d1b1"||fullMove=="d1f1"||fullMove=="d8b8"||fullMove=="d8f8");
                            if (isCastle) {
                                m_waitingForRook = true;
                                // Send hint showing where rook needs to go
                                std::string rookFrom, rookTo;
                                if (fullMove=="e1g1") { rookFrom="h1"; rookTo="f1"; }
                                else if (fullMove=="e1c1") { rookFrom="a1"; rookTo="d1"; }
                                else if (fullMove=="e8g8") { rookFrom="h8"; rookTo="f8"; }
                                else if (fullMove=="e8c8") { rookFrom="a8"; rookTo="d8"; }
                                else if (fullMove=="d1b1") { rookFrom="h1"; rookTo="f1"; }
                                else if (fullMove=="d1f1") { rookFrom="a1"; rookTo="d1"; }
                                else if (fullMove=="d8b8") { rookFrom="h8"; rookTo="f8"; }
                                else if (fullMove=="d8f8") { rookFrom="a8"; rookTo="d8"; }
                                if (!rookFrom.empty() && m_connector && m_connector->isConnected()) {
                                    try {
                                        nlohmann::json jr;
                                        jr["action"] = "hint";
                                        jr["from"] = rookFrom;
                                        jr["to"] = rookTo;
                                        jr["capture"] = false;
                                        m_connector->send((jr.dump()+"\r\n").c_str());
                                    } catch(...) {}
                                }
                            }
                            // Send updated position to controller after human move (skip for castle)
                            if (!m_waitingForRook && m_connector && m_connector->isConnected()) {
                                try {
                                    nlohmann::json jp;
                                    jp["action"] = "setposition";
                                    jp["fen"] = std::string(m_board->getFen());
                                    m_connector->send((jp.dump() + "\r\n").c_str());
                                    fprintf(stderr, "POST-HUMAN-MOVE SETPOSITION: %s\n", m_board->getFen());
                                } catch (...) {}
                            }
                            fprintf(stderr, "HUMAN MOVE: %s sent, requesting engine move\n", fullMove.c_str());
                            if (isEngineToMove()) {
                                m_engineMoveRequested = true;
                                if (m_hintJustFired) { m_hintJustFired = false; m_uciClient->sendUCINewGame(); }
                                m_uciClient->setPosition(m_board->getFen());
                                if (!m_waitingForRook) m_uciClient->sendGo(m_uciClient->getMoveTime());
                            }
                        }
                    }
                    m_pendingMoveStart = "";

                } else if (action == "ready") {
                    // Switch clock back to human after engine move confirmed
                    if (m_clockEnabled && m_clockRunning && !m_pendingEngineMove.empty()) {
                        if (m_humanIsBlack) { m_whiteTimeMs += m_clockIncrement*1000; m_whiteTicking = false; }
                        else { m_blackTimeMs += m_clockIncrement*1000; m_whiteTicking = true; }
                    }
                    if (m_waitingForRook && m_engineMoveRequested && !m_gameOver) {
                        // Castle rook placed - fire engine
                        m_waitingForRook = false;
                        fprintf(stderr, "READY: castle rook placed, firing engine\n");
                        if (m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jpos;
                                jpos["action"] = "setposition";
                                jpos["fen"] = std::string(m_board->getFen());
                                m_connector->send((jpos.dump() + "\r\n").c_str());
                            } catch (...) {}
                        }
                        if (!m_pendingEngineMove.empty()) {
                            nlohmann::json hint;
                            hint["action"] = "hint";
                            hint["from"] = m_pendingEngineMove.substr(0,2);
                            hint["to"]   = m_pendingEngineMove.substr(2,2);
                            simSend(hint);
                        }
                        m_uciClient->setPosition(m_board->getFen());
                        m_uciClient->sendGo(m_uciClient->getMoveTime());
                    } else if (m_humanIsBlack && m_engineMoveRequested && !m_gameOver) {
                        // Board setup complete for new game as black
                        fprintf(stderr, "READY: board setup complete, firing engine\n");
                        m_engineStartDelay = 0;
                        m_uciClient->setPosition(m_board->getFen());
                        m_uciClient->sendGo(m_uciClient->getMoveTime());
                    }
                } else if (action == "move" && m_waitingForRook &&
                               msg.contains("moves") && msg["moves"].is_array() && !msg["moves"].empty()) {
                    // Castle rook move confirmation - treat same as "ready"
                    std::string rookLan = msg["moves"][0].value("lan","");
                    if (rookLan=="h1f1"||rookLan=="a1d1"||rookLan=="h8f8"||rookLan=="a8d8") {
                        fprintf(stderr, "CASTLE ROOK MOVE: %s - treating as ready\n", rookLan.c_str());
                        m_waitingForRook = false;
                        if (m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jpos;
                                jpos["action"] = "setposition";
                                jpos["fen"] = std::string(m_board->getFen());
                                m_connector->send((jpos.dump() + "\r\n").c_str());
                            } catch (...) {}
                        }
                        if (!m_pendingEngineMove.empty()) {
                            nlohmann::json hint;
                            hint["action"] = "hint";
                            hint["from"] = m_pendingEngineMove.substr(0,2);
                            hint["to"]   = m_pendingEngineMove.substr(2,2);
                            simSend(hint);
                        }
                        if (m_engineMoveRequested && !m_gameOver) {
                            m_uciClient->setPosition(m_board->getFen());
                            m_uciClient->sendGo(m_uciClient->getMoveTime());
                        }
                    }
                } else if (action == "ready") {
                    if (m_waitingForRook) {
                        m_waitingForRook = false;
                        fprintf(stderr, "READY: rook placed, sending deferred msgs\n");
                        if (m_connector && m_connector->isConnected()) {
                            try {
                                nlohmann::json jpos;
                                jpos["action"] = "setposition";
                                jpos["fen"] = std::string(m_board->getFen());
                                m_connector->send((jpos.dump() + "\r\n").c_str());
                            } catch (...) {}
                        }
                        if (!m_pendingEngineMove.empty()) {
                            nlohmann::json hint;
                            hint["action"] = "hint";
                            hint["from"] = m_pendingEngineMove.substr(0,2);
                            hint["to"]   = m_pendingEngineMove.substr(2,2);
                            simSend(hint);
                        }
                        if (m_engineMoveRequested && !m_gameOver) {
                            m_uciClient->setPosition(m_board->getFen());
                            m_uciClient->sendGo(m_uciClient->getMoveTime());
                        }
                    } else if (m_humanIsBlack && m_engineMoveRequested && !m_gameOver) {
                        fprintf(stderr, "READY: board setup complete, firing engine\n");
                        m_engineStartDelay = 0;
                        m_uciClient->setPosition(m_board->getFen());
                        m_uciClient->sendGo(m_uciClient->getMoveTime());
                    }
                } else if (action == "reset") {
                    fprintf(stderr, "RESET received from sim\n");
                    m_board->Forsyth("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
                    m_board->clearHighlights();
                    m_board->flip(m_humanIsBlack);
                    m_pendingMoveStart = "";
                    m_pendingEngineMove = "";
                    m_engineMoveRequested = false;
                    m_gameOver = false;
                    m_fenHistory.clear();
                    m_undoConfirm.active = false;
                    m_uciClient->sendUCINewGame();
                    m_uciClient->setPosition(m_board->getFen());
                    lockoutTimer = 0;
                    if (m_movesPanel) m_movesPanel->clear();
                    try { simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}}); } catch (...) {}
                    // disabled for hardware: flip_board
                    if (m_humanIsBlack) {
                        m_engineMoveRequested = true;
                        m_engineStartDelay = 500;
                        fprintf(stderr, "RESET: human=black, engine will play white after delay\n");
                    }
                }

            } catch (...) {}
            _rln = 0; // process one message per update tick
        }
    }

    // --- HINT MOVE LOGIC ---
    if (m_hintMode && m_uciClient->hasNewMove()) {
        std::string hintStr = m_uciClient->getAndClearMove();
        m_hintMode = false;
        m_hintJustFired = true;
        m_pendingHintMove = hintStr;
        fprintf(stderr, "HINT move: %s\n", hintStr.c_str());
        if (hintStr.size() >= 4) {
            std::string fromSq = hintStr.substr(0, 2);
            std::string toSq   = hintStr.substr(2, 2);
            // Highlight hint squares on GUI board
            if (m_board) {
                m_board->clearHighlights();
                m_board->setHighlight(fromSq.c_str(), true);
                m_board->setHighlight(toSq.c_str(), true);
            }
            // Send move to cbcontroller to light LEDs
            if (m_connector && m_connector->isConnected()) {
                try {
                    nlohmann::json jpos;
                    jpos["action"] = "setposition";
                    jpos["fen"] = std::string(m_board->getFen());
                    std::string sp = jpos.dump() + "\r\n";
                    m_connector->send(sp.c_str());
                    // Send hint action to cbcontroller to light LEDs without advancing board
                    nlohmann::json hint;
                    hint["action"] = "hint";
                    hint["from"] = fromSq;
                    hint["to"] = toSq;
                    simSend(hint);
                } catch (...) {}
            }
        }
    }

    // --- ENGINE MOVE LOGIC ---
    if (m_board && isEngineToMove() && m_uciClient->hasNewMove() && m_engineMoveRequested && !m_waitingForRook) {
        std::string moveStr = m_uciClient->getAndClearMove();
        thc::Move mv;
        if (mv.TerseIn(&m_board->getRules(), moveStr.c_str())) {
            m_engineMoveRequested = false;
            m_pendingEngineMove = moveStr;
            // Save FEN before engine move for undo
            std::string preMovefen = std::string(m_board->getFen());
            m_fenHistory.push_back(preMovefen);
            m_board->playMove(moveStr.c_str());
            if (m_movesPanel) m_movesPanel->addMove(moveStr.c_str());
            m_uciClient->setPosition(m_board->getFen());
            // Highlight engine from+to squares on GUI
            m_board->clearHighlights();
            m_board->setHighlight(moveStr.substr(0, 2).c_str(), true);
            m_board->setHighlight(moveStr.substr(2, 2).c_str(), true);
            fprintf(stderr, "ENGINE MOVE: %s applied, waiting for physical confirmation\n", moveStr.c_str());
            bool lastWasCastle = (m_lastHumanMove=="e1g1"||m_lastHumanMove=="e1c1"||m_lastHumanMove=="e8g8"||m_lastHumanMove=="e8c8");
            m_lastHumanMove = "";
            // Send setposition so controller knows current position (skip during castle rook pending)
            if (!m_waitingForRook) {
                try {
                    nlohmann::json jpos;
                    jpos["action"] = "setposition";
                    jpos["fen"] = std::string(m_board->getFen());
                    std::string sp = jpos.dump() + "\r\n";
                    m_connector->send(sp.c_str());
                    fprintf(stderr, "SETPOSITION sent: %s\n", m_board->getFen());
                } catch (...) { fprintf(stderr, "SETPOSITION send failed\n"); }
            } else {
                fprintf(stderr, "SETPOSITION skipped: waiting for castle rook\n");
            }
            if (m_connector && m_connector->isConnected()) {
                try {
                    if (m_board->isCheckmate()) {
                        m_gameOver = true;
                        std::string kSq = m_board->getKingSquare();
                        m_board->clearHighlights();
                        if (!kSq.empty()) {
                            m_board->setHighlight(kSq.c_str(), true);
                            simSend(nlohmann::json{{"action","flash"},{"on",true},{"mate",true},{"squares",nlohmann::json::array({kSq})}});
                        }
                        fprintf(stderr, "CHECKMATE!\n");
                    } else if (m_board->isInCheck()) {
                        std::string kSq = m_board->getKingSquare();
                        if (!kSq.empty())
                            simSend(nlohmann::json{{"action","flash"},{"on",true},{"squares",nlohmann::json::array({kSq})}});
                    } else {
                        simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}});
                    }
                    {
                        fprintf(stderr, "ENGINE MOVE SEND: humanIsBlack=%d waitingForRook=%d\n", m_humanIsBlack?1:0, m_waitingForRook?1:0);
                        if (true) {  // always use hint for engine moves
                            nlohmann::json hint;
                            hint["action"] = "hint";
                            hint["from"] = moveStr.substr(0,2);
                            hint["to"]   = moveStr.substr(2,2);
                            // Check if capture using pre-move FEN
                            {
                                thc::ChessRules cr;
                                cr.Forsyth(preMovefen.c_str());
                                std::string toSq = moveStr.substr(2,2);
                                int col = tolower(toSq[0]) - 'a';
                                int row = 8 - (toSq[1] - '0');
                                int idx = row * 8 + col;
                                char piece = cr.squares[idx];
                                bool isCapture = (piece != ' ' && piece != '.');
                                hint["capture"] = isCapture;
                                fprintf(stderr, "ENGINE HINT: %s->%s capture=%d piece=%c\n",
                                    moveStr.substr(0,2).c_str(), moveStr.substr(2,2).c_str(), isCapture?1:0, piece);
                            }
                            if (!m_waitingForRook) simSend(hint);
                        } else {
                            nlohmann::json mv;
                            mv["action"] = "move";
                            mv["description"] = nullptr;
                            nlohmann::json m;
                            m["from"] = moveStr.substr(0,2);
                            m["to"]   = moveStr.substr(2,2);
                            m["lan"]  = moveStr;
                            m["type"] = "move";
                            mv["moves"] = nlohmann::json::array({m});
                            simSend(mv);
                        }
                    }
                } catch (...) {}
                lockoutTimer = m_humanIsBlack ? 0 : 800;
            }
        }
    }
}

static std::string flipSquare(const std::string& sq) {
    if (sq.size() < 2) return sq;
    char file = sq[0]; // keep file same
    char rank = '1' + ('8' - sq[1]); // mirror rank only: 1<->8, 2<->7, etc.
    return std::string({file, rank});
}

std::string ControllerGUI::simSq(const std::string& sq) const {
    return sq;  // physical board never rotates; always use real chess coordinates
}

std::string ControllerGUI::simLan(const std::string& lan) const {
    if (lan.size() < 4) return lan;
    return simSq(lan.substr(0,2)) + simSq(lan.substr(2,2));
}

void ControllerGUI::simSend(const nlohmann::json& j) const {
    if (!m_connector || !m_connector->isConnected()) return;
    try { std::string s = j.dump() + "\n"; m_connector->send(s.c_str()); fprintf(stderr, "SIMSEND: %s", s.c_str()); } catch (...) {}
}


void ControllerGUI::newGame() {
    m_board->Forsyth("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    m_board->clearHighlights();
    m_board->flip(m_humanIsBlack);
    m_pendingMoveStart = "";
    m_pendingEngineMove = "";
    m_engineMoveRequested = false;
    m_gameOver = false;
    m_fenHistory.clear();
    m_undoConfirm.active = false;
    m_studyIndex = -1;
    m_studyWaitingConfirm = false;
    m_hintMode = false;
    m_hintJustFired = false;
    m_waitingForRook = false;
    m_syncTimer = 0;
    // Clear mark state
    if (!m_markFen.empty()) {
        m_markFen = "";
        if (m_menuPopup) m_menuPopup->setButtonLabel("Return", "Mark");
    }
    if (m_clockEnabled) {
        int presetMs = ClockPopup::presetTimeMs(m_clockPresetIndex);
        m_whiteTimeMs = presetMs;
        m_blackTimeMs = presetMs;
        m_clockIncrement = ClockPopup::presetIncrementSec(m_clockPresetIndex);
        m_clockRunning = true;
        m_whiteTicking = true;
        fprintf(stderr, "CLOCK START: presetIndex=%d timeMs=%d running=%d\n", m_clockPresetIndex, presetMs, m_clockRunning?1:0);
    } else {
        m_clockRunning = false;
        fprintf(stderr, "CLOCK DISABLED\n");
    }
    // m_twoPlayer and m_humanIsBlack are set by the caller before newGame()
    if (m_movesPanel) m_movesPanel->clear();
    m_uciClient->sendUCINewGame();
    m_uciClient->setPosition(m_board->getFen());
    if (m_connector && m_connector->isConnected()) {
        try {
            // Send setmode FIRST so boardFlipped is set before setPosition runs
            nlohmann::json jmode;
            jmode["action"] = "setmode";
            jmode["mode"] = m_humanIsBlack ? "playblack" : "play";
            m_connector->send((jmode.dump() + "\r\n").c_str());
            // THEN setposition
            nlohmann::json jpos;
            jpos["action"] = "setposition";
            jpos["fen"] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            std::string sp = jpos.dump() + "\r\n";
            m_connector->send(sp.c_str());
        } catch (...) {}
    }
    if (m_humanIsBlack) {
        m_engineMoveRequested = true;
        m_engineStartDelay = 0; // engine fires only when controller sends "ready"
        fprintf(stderr, "NEW GAME: human=black, waiting for board ready signal\n");
    } else {
        fprintf(stderr, "NEW GAME: human=white\n");
    }
}

void ControllerGUI::undoLastTwoMoves() {
    // Need at least 2 moves to undo (black + white)
    const auto& moves = m_movesPanel ? m_movesPanel->getMoves() : std::vector<std::string>{};
    if (moves.size() < 2 || m_fenHistory.size() < 2) {
        fprintf(stderr, "UNDO: not enough moves to undo\n");
        return;
    }
    if (m_undoConfirm.active) {
        fprintf(stderr, "UNDO: already in undo mode\n");
        return;
    }
    // Clear any pending engine move - undo overrides it
    m_pendingEngineMove = "";
    m_engineMoveRequested = false;

    // Get the last two moves
    std::string blackMove = moves[moves.size() - 1];
    std::string whiteMove = moves[moves.size() - 2];

    fprintf(stderr, "UNDO: undoing white=%s black=%s\n", whiteMove.c_str(), blackMove.c_str());

    // Restore board state
    std::string currentPhysicalFen = std::string(m_board->getFen()); // save BEFORE restoring
    std::string targetFen = m_fenHistory[m_fenHistory.size() - 2];
    m_board->Forsyth(targetFen.c_str());
    m_uciClient->setPosition(m_board->getFen());
    m_fenHistory.resize(m_fenHistory.size() - 2);
    // Store target FEN for final setposition to cbcontroller
    m_undoTargetFen = targetFen;
    m_undoCurrentFen = currentPhysicalFen;
    m_movesPanel->removeLastTwo();
    m_board->clearHighlights();

    // Set up undo confirmation
    // move1From = where white piece currently sits (needs to move back to move1To)
    // move2From = where black piece currently sits (needs to move back to move2To)
    m_undoConfirm.active    = true;
    m_undoConfirm.move1From = whiteMove.substr(2, 2); // white dst (f3)
    m_undoConfirm.move1To   = whiteMove.substr(0, 2); // white src (g1)
    m_undoConfirm.move2From = blackMove.substr(2, 2); // black dst (e7)
    m_undoConfirm.move2To   = blackMove.substr(0, 2); // black src (g8)
    m_undoConfirm.move1Done = false;
    m_undoConfirm.move2Done = false;

    m_gameOver = false;
    m_engineMoveRequested = false;
    m_pendingMoveStart = "";
    m_hintMode = false;

    // Send CURRENT FEN to cbcontroller so it knows where pieces are before undo
    if (m_connector && m_connector->isConnected()) {
        try {
            // Use the FEN that was current before undo started (2 moves ahead of target)
            nlohmann::json jp;
            jp["action"] = "setposition";
            jp["fen"] = m_undoCurrentFen;
            m_connector->send((jp.dump() + "\r\n").c_str());
        } catch (...) {}
    }
    // Light white piece square and destination so player knows what to move
    m_board->setHighlight(m_undoConfirm.move1From.c_str(), true);
    m_board->setHighlight(m_undoConfirm.move1To.c_str(), true);
    try {
        nlohmann::json hint;
        hint["action"] = "takeback";
        hint["from"] = m_undoConfirm.move1From;
        hint["to"] = m_undoConfirm.move1To;
        simSend(hint);
    } catch (...) {}

    fprintf(stderr, "UNDO: flash %s (white piece), then %s (destination)\n",
        m_undoConfirm.move1From.c_str(), m_undoConfirm.move1To.c_str());
}

void ControllerGUI::loadPGN() {
    if (m_filePickerPopup) m_filePickerPopup->show("/home/pi/chessbox/games");
}

void ControllerGUI::loadPGNFile(const std::string& path) {

    if (!m_pgnLoader.loadFile(path)) {
        fprintf(stderr, "STUDY: failed to load %s\n", path.c_str());
        return;
    }

    // Build FEN list by replaying all moves from start
    m_studyFens.clear();
    m_studyLanMoves.clear();

    thc::ChessRules cr;
    {
        std::string startFen = m_pgnLoader.header("FEN");
        if (startFen.empty()) startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        cr.Forsyth(startFen.c_str());
    }

    // Store starting position
    static std::string fen0; fen0 = cr.ForsythPublish();
    m_studyFens.push_back(fen0);

    for (const std::string& san : m_pgnLoader.moves()) {
        thc::Move mv;
        // Try NaturalIn (SAN)
        bool ok = mv.NaturalIn(&cr, san.c_str());
        if (!ok) {
            // Try TerseIn (LAN)
            ok = mv.TerseIn(&cr, san.c_str());
        }
        if (!ok) {
            fprintf(stderr, "STUDY: cannot parse move '%s', stopping\n", san.c_str());
            break;
        }
        // Store LAN for display
        m_studyLanMoves.push_back(mv.TerseOut());
        cr.PlayMove(mv);
        static std::string fenN; fenN = cr.ForsythPublish();
        m_studyFens.push_back(fenN);
    }

    fprintf(stderr, "STUDY: loaded %zu positions from %s\n",
        m_studyFens.size(), path.c_str());

    // Go to start position
    m_studyIndex = 0;
    studyStep(0);
}

void ControllerGUI::studyStep(int delta) {
    if (m_studyFens.empty()) return;

    int newIndex;
    if (delta == INT_MIN) newIndex = 0;
    else if (delta == INT_MAX) newIndex = (int)m_studyFens.size() - 1;
    else newIndex = m_studyIndex + delta;

    // Clamp
    if (newIndex < 0) newIndex = 0;
    if (newIndex >= (int)m_studyFens.size()) newIndex = (int)m_studyFens.size() - 1;
    if (newIndex == m_studyIndex && delta != 0) return;

    m_studyIndex = newIndex;

    // Update board
    m_board->Forsyth(m_studyFens[m_studyIndex].c_str());
    m_board->clearHighlights();

    // Highlight the last move on GUI
    if (m_studyIndex > 0 && m_studyIndex - 1 < (int)m_studyLanMoves.size()) {
        const std::string& lastMove = m_studyLanMoves[m_studyIndex - 1];
        m_board->setHighlight(lastMove.substr(0, 2).c_str(), true);
        m_board->setHighlight(lastMove.substr(2, 2).c_str(), true);
    }

    // Send full board state to sim by replaying from start
    if (m_connector && m_connector->isConnected()) {
        try {
            // Always do a full resync — send reset then replay all moves to current pos
            // First clear flash
            simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}});

            // Rebuild sim board from scratch by sending all moves up to current index
            // We do this by building the position incrementally
            thc::ChessRules cr;
            cr.Forsyth("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

            // Send a setup_pieces for the starting position first
            // Actually sim doesn't support full position reset easily
            // Best approach: send moves from start
            // But sim board may be out of sync — for now just send the last move
            // Send current board position to controller
            nlohmann::json jpos;
            jpos["action"] = "setposition";
            jpos["fen"] = m_studyFens[m_studyIndex];
            m_connector->send((jpos.dump() + "\r\n").c_str());
            if (m_studyIndex > 0 && m_studyIndex - 1 < (int)m_studyLanMoves.size()) {
                const std::string& lan = m_studyLanMoves[m_studyIndex - 1];
                // Light from/to LEDs for the move just played
                nlohmann::json hint;
                hint["action"] = "hint";
                hint["from"] = lan.substr(0, 2);
                hint["to"]   = lan.substr(2, 2);
                simSend(hint);
            } else {
                // At start position — clear all LEDs
                simSend(nlohmann::json{{"action","flash"},{"on",false},{"squares",nlohmann::json::array()}});
            }
        } catch (...) {}
    }

    // Update moves panel — show moves up to current position
    if (m_movesPanel) {
        m_movesPanel->clear();
        for (int i = 0; i < m_studyIndex && i < (int)m_studyLanMoves.size(); i++)
            m_movesPanel->addMove(m_studyLanMoves[i].c_str());
    }

    fprintf(stderr, "STUDY: position %d/%zu\n", m_studyIndex, m_studyFens.size() - 1);
}

void ControllerGUI::exportPGN() {
    if (!m_movesPanel) return;
    const std::vector<std::string>& moves = m_movesPanel->getMoves();
    if (moves.empty()) {
        fprintf(stderr, "EXPORT: no moves to export\n");
        return;
    }

    // Generate timestamped filename
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char filename[64];
    strftime(filename, sizeof(filename), "/home/pi/chessbox/games/game_%Y%m%d_%H%M%S.pgn", t);

    char dateStr[16];
    strftime(dateStr, sizeof(dateStr), "%Y.%m.%d", t);

    std::ofstream f(filename);
    if (!f.is_open()) {
        fprintf(stderr, "EXPORT: failed to open %s\n", filename);
        return;
    }

    // PGN headers
    f << "[Event \"ChessBox Game\"]\n";
    f << "[Site \"ChessBox\"]\n";
    f << "[Date \"" << dateStr << "\"]\n";
    f << "[Round \"1\"]\n";
    f << "[White \"Human\"]\n";
    f << "[Black \"Stockfish Lvl " << (m_uciClient ? m_uciClient->getSkillLevel() : 0) << "\"]\n";
    f << "[Result \"*\"]\n";
    f << "\n";

    // Moves — pair up as numbered move list
    for (size_t i = 0; i < moves.size(); i++) {
        if (i % 2 == 0)
            f << (i / 2 + 1) << ". ";
        f << moves[i] << " ";
        if (i % 2 == 1)
            f << "\n";
    }
    f << "*\n";
    f.close();

    fprintf(stderr, "EXPORT: saved to %s\n", filename);
}

void ControllerGUI::processButtonClicked(Button* b) {
    if (!b) return;
    if (b->id().substr(0, 4) == "Lvl:" && m_levelPopup && m_uciClient) {
        m_levelPopup->show(m_uciClient->getSkillLevel());
        return;
    }
    if (b->id().substr(0, 4) == "Dpt:" && m_depthPopup && m_uciClient) {
        m_depthPopup->show(m_uciClient->getDepth());
        return;
    }
    if (b->id().substr(0, 2) == "T:" && m_timePopup && m_uciClient) {
        m_timePopup->show(m_uciClient->getMoveTime());
        return;
    }
    if (b->id() == "Back") { undoLastTwoMoves(); return; }
    if (b->id() == "Menu") { if (m_menuPopup) m_menuPopup->show(); return; }
    if (b->id() == "W_P")  { m_humanIsBlack = false; m_twoPlayer = false; newGame(); return; }
    if (b->id() == "B_P")  { m_humanIsBlack = true;  m_twoPlayer = false; newGame(); return; }
    if (b->id() == "WB_P") { m_humanIsBlack = false; m_twoPlayer = true;  newGame(); return; }
    if (b->id() == "BB_P") { m_humanIsBlack = true;  m_twoPlayer = true;  newGame(); return; }
    if (b->id() == "LEDTest") {
        if (m_connector && m_connector->isConnected()) {
            try {
                nlohmann::json j;
                j["action"] = "setmode";
                j["mode"] = "ledtest";
                m_connector->send((j.dump() + "\r\n").c_str());
            } catch (...) {}
        }
        return;
    }
    if (b->id() == "ReedTest") {
        if (m_connector && m_connector->isConnected()) {
            try {
                nlohmann::json j;
                j["action"] = "setmode";
                j["mode"] = "reedtest";
                m_connector->send((j.dump() + "\r\n").c_str());
            } catch (...) {}
        }
        return;
    }
    if (b->id() == "Connect") {
        m_connector->connect(m_host.c_str(), m_port);
        // Tell cbcontroller to enter play mode so LEDs work
        try { m_connector->send("{\"action\":\"setmode\",\"mode\":\"play\"}\n"); } catch (...) {}
    }
    if (b->id() == "Load")   { if (m_filePickerPopup) m_filePickerPopup->show("/home/pi/chessbox/games"); return; }
    if (b->id() == "Export") exportPGN();
    if (b->id() == "Clock")  { if (m_clockPopup) m_clockPopup->show(m_clockPresetIndex); return; }
    if (b->id() == "Mark") {
        if (m_board) {
            m_markFen = std::string(m_board->getFen());
            m_markClockRunning = m_clockRunning;
            m_markWhiteTicking = m_whiteTicking;
            m_clockRunning = false;
            if (m_menuPopup) m_menuPopup->setButtonLabel("Mark", "Return");
        }
        return;
    }
    if (b->id() == "Return") {
        if (m_markPopup) {
            m_menuPopup->hide();
            m_markPopup->show();
        }
        return;
    }
    if (b->id() == "<")  { studyStep(-1); return; }
    if (b->id() == ">")  { studyStep(+1); return; }
    if (b->id() == "<<") { studyStep(INT_MIN); return; }
    if (b->id() == "New") { newGame(); return; }
    if (b->id() == "Flip") m_board->flip(!m_board->isFlipped());
    if (b->id() == "Hint" && m_uciClient && m_board) {
        if (m_engineMoveRequested) {
            fprintf(stderr, "HINT blocked: engine move pending\n");
        } else if (m_hintMode) {
            fprintf(stderr, "HINT blocked: already in hint mode\n");
        } else {
            fprintf(stderr, "HINT: requesting move for FEN: %s\n", m_board->getFen());
            m_hintMode = true;
            if (m_uciClient->hasNewMove()) m_uciClient->getAndClearMove();
            m_uciClient->setPosition(m_board->getFen());
            m_uciClient->sendGo(1000);
        }
    }
}

Component* ControllerGUI::mouseEvent(SDL_Event* event) {
    // Popups intercept all clicks when visible
    if (m_clockPopup && m_clockPopup->isVisible()) {
        Component* caught = m_clockPopup->mouseEvent(event);
        if (caught) {
            int sel = m_clockPopup->getSelectedIndex();
            if (sel >= 0) {
                m_clockPopup->clearSelection();
                m_clockPresetIndex = sel;
                int newTimeMs = ClockPopup::presetTimeMs(sel);
                m_clockIncrement  = ClockPopup::presetIncrementSec(sel);
                if (newTimeMs == 0) {
                    m_clockEnabled = false;
                } else {
                    m_clockEnabled = true;
                    m_whiteTimeMs  = newTimeMs;
                    m_blackTimeMs  = newTimeMs;
                    m_clockRunning = true;
                    m_whiteTicking = true;
                }
            }
        }
        return caught;
    }
    if (m_markPopup && m_markPopup->isVisible()) {
        Component* caught = m_markPopup->mouseEvent(event);
        if (caught) {
            MarkPopup::Result res = m_markPopup->getResult();
            m_markPopup->clearResult();
            if (res == MarkPopup::RETURN_TO_MARK && !m_markFen.empty() && m_board) {
                m_board->Forsyth(m_markFen.c_str());
                m_clockRunning = m_markClockRunning;
                m_whiteTicking = m_markWhiteTicking;
                try {
                    nlohmann::json jp;
                    jp["action"] = "setposition";
                    jp["fen"] = m_markFen;
                    if (m_connector && m_connector->isConnected())
                        m_connector->send((jp.dump()+"\r\n").c_str());
                } catch(...) {}
                if (m_uciClient) m_uciClient->setPosition(m_markFen.c_str());
                if (m_menuPopup) m_menuPopup->setButtonLabel("Return", "Mark");
                m_markFen = "";
            } else if (res == MarkPopup::NEW_MARK && m_board) {
                m_markFen = std::string(m_board->getFen());
                m_markClockRunning = m_clockRunning;
                m_markWhiteTicking = m_whiteTicking;
                // clock stays paused, button stays "Return"
            } else if (res == MarkPopup::CLEAR) {
                m_markFen = "";
                m_clockRunning = m_markClockRunning;
                m_whiteTicking = m_markWhiteTicking;
                if (m_menuPopup) m_menuPopup->setButtonLabel("Return", "Mark");
            }
        }
        return caught;
    }
    if (m_filePickerPopup && m_filePickerPopup->isVisible() && !m_filePickerPopup->isClosing()) {
        Component* caught = m_filePickerPopup->mouseEvent(event);
        if (caught) {
            std::string sel = m_filePickerPopup->getSelectedFile();
            if (!sel.empty()) {
                m_filePickerPopup->clearSelection();
                loadPGNFile(sel);
            }
        }
        return caught;
    }
    if (m_menuPopup && m_menuPopup->isVisible()) {
        Component* caught = m_menuPopup->mouseEvent(event);
        if (caught) {
            std::string sel = m_menuPopup->getSelectedId();
            if (!sel.empty()) {
                // Find the matching button from menu and process it
                for (auto* b : m_menuPopup->getButtons()) {
                    if (b->id() == sel) {
                        processButtonClicked(b);
                        break;
                    }
                }
            }
            m_menuPopup->clearSelection();
        }
        return caught;
    }
    if (m_levelPopup && m_levelPopup->isVisible()) {
        Component* caught = m_levelPopup->mouseEvent(event);
        if (caught) {
            int level = m_levelPopup->getSelectedLevel();
            if (level >= 0 && m_uciClient) {
                m_uciClient->setSkillLevel(level);
                initComponents();
            }
            m_levelPopup->clearSelection();
        }
        return caught;
    }
    if (m_timePopup && m_timePopup->isVisible()) {
        Component* caught = m_timePopup->mouseEvent(event);
        if (caught) {
            int ms = m_timePopup->getSelectedMs();
            if (ms >= 0 && m_uciClient) {
                m_uciClient->setMoveTime(ms);
                initComponents();
            }
            m_timePopup->clearSelection();
        }
        return caught;
    }
    if (m_depthPopup && m_depthPopup->isVisible()) {
        Component* caught = m_depthPopup->mouseEvent(event);
        if (caught) {
            int depth = m_depthPopup->getSelectedDepth();
            if (depth >= 1 && m_uciClient) {
                m_uciClient->setDepth(depth);
                initComponents();
            }
            m_depthPopup->clearSelection();
        }
        return caught;
    }
    for (auto b : m_buttons) {
        Component* caught = b->mouseEvent(event);
        if (caught) {
            this->processButtonClicked(static_cast<Button*>(caught));
            return caught;
        }
    }
    return Window::mouseEvent(event);
}

void ControllerGUI::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255);  // very dark bg
    SDL_RenderClear(renderer);

    SDL_Rect buttonRow = { 0, 736, 480, 68 };
    SDL_SetRenderDrawColor(renderer, 10, 14, 20, 255);  // same dark bg
    SDL_RenderFillRect(renderer, &buttonRow);

    Window::draw(renderer);

    // FPS counter — top right corner
    if (m_font) {
        static uint32_t lastTime = SDL_GetTicks();
        static int frameCount = 0;
        static int fps = 0;
        frameCount++;
        uint32_t now = SDL_GetTicks();
        if (now - lastTime >= 1000) {
            fps = frameCount;
            frameCount = 0;
            lastTime = now;
        }
        char fpsBuf[16];
        snprintf(fpsBuf, sizeof(fpsBuf), "%d fps", fps);
        SDL_Color fpsColor = {100, 140, 180, 255};
        SDL_Surface* s = TTF_RenderText_Blended(m_font, fpsBuf, fpsColor);
        if (s) {
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect dst = {480 - s->w - 4, 482, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &dst);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }

    // Draw popups on top of everything
    if (m_menuPopup  && m_menuPopup->isVisible())
        m_menuPopup->draw(renderer);
    if (m_filePickerPopup && m_filePickerPopup->isVisible())
        m_filePickerPopup->draw(renderer);
    if (m_clockEnabled && !(m_menuPopup && m_menuPopup->isVisible()) && !(m_filePickerPopup && m_filePickerPopup->isVisible())) drawClock(renderer);
    if (m_clockPopup && m_clockPopup->isVisible()) m_clockPopup->draw(renderer);
    if (m_markPopup  && m_markPopup->isVisible())  m_markPopup->draw(renderer);
    if (m_levelPopup && m_levelPopup->isVisible())
        m_levelPopup->draw(renderer);
    if (m_depthPopup && m_depthPopup->isVisible())
        m_depthPopup->draw(renderer);
    if (m_timePopup  && m_timePopup->isVisible())
        m_timePopup->draw(renderer);
}

void ControllerGUI::startGame() {
    SDL_Event event;
    uint32_t lastTicks = SDL_GetTicks();
    while (m_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) m_running = false;
            // Convert finger touch to mouse click for touchscreen support
            if (event.type == SDL_FINGERDOWN) {
                SDL_Event mouseEvent;
                mouseEvent.type = SDL_MOUSEBUTTONDOWN;
                mouseEvent.button.button = SDL_BUTTON_LEFT;
                mouseEvent.button.state = SDL_PRESSED;
                mouseEvent.button.x = (int)(event.tfinger.x * 480);
                mouseEvent.button.y = (int)(event.tfinger.y * 800);
                fprintf(stderr, "TOUCH: x=%d y=%d\n", mouseEvent.button.x, mouseEvent.button.y);
                this->mouseEvent(&mouseEvent);
            } else {
                this->mouseEvent(&event);
            }
        }
        uint32_t currentTicks = SDL_GetTicks();
        this->update(currentTicks - lastTicks);
        lastTicks = currentTicks;
        if (m_renderer) {
            this->draw(m_renderer);
            SDL_RenderPresent(m_renderer);
        }
        SDL_Delay(16);
    }
}
void ControllerGUI::clockReset() {
    m_whiteTimeMs = 10 * 60 * 1000;
    m_blackTimeMs = 10 * 60 * 1000;
    m_whiteTicking = true;
    m_clockRunning = false;
}

void ControllerGUI::clockStop()       { m_clockRunning = false; }
void ControllerGUI::clockStartWhite() { m_whiteTicking = true;  m_clockRunning = true; }
void ControllerGUI::clockStartBlack() { m_whiteTicking = false; m_clockRunning = true; }

void ControllerGUI::drawClock(SDL_Renderer* renderer) {
    if (!m_font) return;

    auto formatTime = [](long ms) -> std::string {
        if (ms < 0) ms = 0;
        int total = (int)(ms / 1000);
        int mins  = total / 60;
        int secs  = total % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
        return std::string(buf);
    };

    // Clock bar between board (y=480) and moves panel
    // Two side-by-side boxes spanning full width
    int barY = 481;
    int barH = 30;
    int half = 240;

    SDL_Rect blackBox = {0,    barY, half,      barH};
    SDL_Rect whiteBox = {half, barY, 480-half,  barH};

    bool blackActive = m_clockRunning && !m_whiteTicking;
    bool whiteActive = m_clockRunning && m_whiteTicking;

    bool blackWarn = m_blackTimeMs < 30000;
    bool whiteWarn = m_whiteTimeMs < 30000;
    // Black clock (left)
    SDL_SetRenderDrawColor(renderer, blackWarn ? 180 : (blackActive ? 0 : 15), blackWarn ? 20 : (blackActive ? 110 : 25), blackWarn ? 20 : (blackActive ? 175 : 40), 255);
    SDL_RenderFillRect(renderer, &blackBox);
    SDL_SetRenderDrawColor(renderer, blackActive ? 180 : 60, blackActive ? 230 : 90, blackActive ? 0 : 90, 255);
    SDL_RenderDrawRect(renderer, &blackBox);

    // White clock (right)
    SDL_SetRenderDrawColor(renderer, whiteWarn ? 180 : (whiteActive ? 0 : 15), whiteWarn ? 20 : (whiteActive ? 110 : 25), whiteWarn ? 20 : (whiteActive ? 175 : 40), 255);
    SDL_RenderFillRect(renderer, &whiteBox);
    SDL_SetRenderDrawColor(renderer, whiteActive ? 180 : 60, whiteActive ? 230 : 90, whiteActive ? 0 : 90, 255);
    SDL_RenderDrawRect(renderer, &whiteBox);

    // Labels
    auto drawText = [&](SDL_Rect& r, const std::string& txt, SDL_Color col) {
        SDL_Surface* s = TTF_RenderText_Blended(m_font, txt.c_str(), col);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        if (t) {
            SDL_Rect d = {r.x + (r.w - s->w)/2, r.y + (r.h - s->h)/2, s->w, s->h};
            SDL_RenderCopy(renderer, t, nullptr, &d);
            SDL_DestroyTexture(t);
        }
        SDL_FreeSurface(s);
    };

    SDL_Color white = {220, 235, 255, 255};
    SDL_Color warn  = {220, 235, 255, 255};

    drawText(blackBox, "B " + formatTime(m_blackTimeMs), m_blackTimeMs < 30000 ? warn : white);
    drawText(whiteBox, "W " + formatTime(m_whiteTimeMs), m_whiteTimeMs < 30000 ? warn : white);
}

