#include <SDL2/SDL.h>
#include <cstdio>
#include <cctype>
#include <vector>
#include <fstream>
#include <ctime>
#include "telnetserver.h"
#include "json.hpp"
#include "Window.h"
#include "Button.h"
#include "Sprite.h"
#include "FontManager.h"
#include "Label.h"

// --- File logger - opens and closes on every write to guarantee disk flush ---
static void simlog(const char* fmt, ...) {
    FILE* logFile = fopen("C:\\chess\\sim_debug.log", "a");
    if (!logFile) {
        printf("SIMLOG ERROR: could not open log file\n");
        fflush(stdout);
        return;
    }

    time_t t = time(nullptr);
    struct tm* tm_info = localtime(&t);
    char timebuf[16];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);
    fprintf(logFile, "[%s] ", timebuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(logFile, fmt, args);
    va_end(args);
    fprintf(logFile, "\n");
    fclose(logFile);  // Close immediately - guaranteed on disk
}

#ifdef WIN32
WSADATA m_wsd;              ///< WSA startup information (Windows only)
#endif

using namespace nlohmann;   //json
using namespace std;

#define FLASH_SPEED 500
#define BUTTON_WIDTH    45
#define BUTTON_HEIGHT   22
#define SCREEN_WIDTH    400
#define SCREEN_HEIGHT   400
#define NUM_SQUARES     64

#define GAME_MODE_IDLE  0
#define GAME_MODE_SETUP 1
#define GAME_MODE_MOVE  2
#define GAME_MODE_END   3
#define GAME_MODE_DEMO  4
#define GAME_MODE_PLAYING  5
#define MOVE_UP 'U'
#define MOVE_DOWN 'D'
#define PIECE_DOWN "piece_down"
#define PIECE_UP "piece_up"
#define SETUP_BUTTON_ID "setupbuttonid"
#define CLEAR_BUTTON_ID "clearbuttonid"

class SimBoard : public Component {
protected:
    SDL_Rect m_rectSquare;
    bool m_flipped = false;
    SDL_Color m_blackColor;
    SDL_Color m_whiteColor;
    bool m_flashState = false;
    bool m_led[NUM_SQUARES];
    bool m_pieces[NUM_SQUARES];
    bool m_flash[NUM_SQUARES];
    const char m_lan[NUM_SQUARES][3] = {
            "a8","b8","c8","d8","e8","f8","g8","h8",
            "a7","b7","c7","d7","e7","f7","g7","h7",
            "a6","b6","c6","d6","e6","f6","g6","h6",
            "a5","b5","c5","d5","e5","f5","g5","h5",
            "a4","b4","c4","d4","e4","f4","g4","h4",
            "a3","b3","c3","d3","e3","f3","g3","h3",
            "a2","b2","c2","d2","e2","f2","g2","h2",
            "a1","b1","c1","d1","e1","f1","g1","h1"
    };

public:
    SimBoard(int x, int y, int w, int h) : Component("board", x, y, w, h) {
        m_rectSquare = {x, y, w / 8, h / 8};
        m_flipped = false;
        m_whiteColor = {0x4B,0x99,0xC5,0xFF};
        m_blackColor = {0x00,0x6A,0xA6,0xFF};

        for(int i=0; i<NUM_SQUARES; i++) {
            m_led[i]=false;
            m_pieces[i]=false;
            m_flash[i]=false;
        }
    }

    void toggleFlash() {
        m_flashState = !m_flashState;
    }

    const char* lan(int pos) {
        return m_lan[pos];
    }
    const char* lan(int x, int y) {
        return lan(y * 8 + x);
    }
    void setFlipped(bool f) { m_flipped = f; }
    bool isFlipped() const { return m_flipped; }
    void setPiece(int n,bool state) {
        m_pieces[n] = state;
    }
    void setPiece(int x, int y, bool state) {
        setPiece(y * 8 + x,state);
    }
    bool piece(int x, int y) {
        return piece(y * 8 + x);
    }
    bool piece(int n) {
        return m_pieces[n];
    }
    void setLed(int x, int y, bool state) {
        setLed(y*8+x,state);
    }
    void setLed(int n,bool state) {
        m_led[n] = state;
    }
    void setLedAll(bool state) {
        for(int i=0; i<NUM_SQUARES; i++) {
            m_led[i] = state;
        }
    }
    bool led(int n) {
        return m_led[n];
    }
    bool led(int x, int y) {
        return led(y * 8 + x);
    }
    void setFlashing(int n, bool state) {
        m_flash[n] = state;
    }
    bool isFlashing(int n) {
        return m_flash[n];
    }
    bool isFlashing(int x, int y) {
        return isFlashing(y * 8 + x);
    }

    void queryPieces(bool *dest) {
        for(int i=0; i<NUM_SQUARES; i++) {
            dest[i] = m_pieces[i];
        }
    }
    void queryLeds(bool *dest) {
        for(int i=0; i<NUM_SQUARES; i++) {
            dest[i] = m_led[i];
        }
    }

    void draw(SDL_Renderer *renderer) {
        drawSquares(renderer);
    }
    void drawSquares(SDL_Renderer *renderer) {
        int white=1;
        int i=0;
        SDL_Color color;
        for(int y=0; y<8; y++) {
            for(int x=0; x<8; x++) {
                int xx= m_rectSquare.x + m_rectSquare.w * x;
                int yy= m_rectSquare.y + m_rectSquare.h * y;
                SDL_Rect r = {xx, yy, m_rectSquare.w, m_rectSquare.h};
                int di = m_flipped ? (63 - i) : i;  // flipped display index
                if(white) {
                    color = m_whiteColor;
                    white=0;
                } else {
                    color = m_blackColor;
                    white=1;
                }
                if(m_pieces[di]) {
                    color.r+=100;
                    color.g+=100;
                    color.b+=100;
                }
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &r);

                SDL_Color ledColor = {255,255,255,255};
                if(m_flash[di] && !m_flashState) {
                    ledColor = {0,0,0,255};
                }
                if(m_led[di] || m_flash[di]) {
                    SDL_SetRenderDrawColor(renderer, ledColor.r, ledColor.g, ledColor.b, ledColor.a);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                }
                SDL_Rect led = {xx+5, yy+5, 5, 5};
                SDL_RenderFillRect(renderer, &led);
                i++;
            }
            white=!white;
        }
    }
};

class SimServer : public TelnetServer, Window {
protected:
    char m_buffer[1024];
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SimBoard m_board;
    int m_expectedPosition[64];
    long m_lastTicks;
    int m_gameMode = GAME_MODE_IDLE;
    int m_moveActions[4];
    int m_moveLocations[4];
    int m_movesNeeded=0;
    int m_moveIndex=0;
    Label* m_pstatusLabel;
    int m_engineDest = -1;
    int m_engineSrc = -1;
    bool m_lastHighlight[64] = {};
    int m_pendingMoveFrom = -1; // square of piece currently lifted
    bool m_ignoreCastle[64] = {}; // squares to ignore after auto-castling
public:
    SimServer(const SockAddr& saBind) :
            TelnetServer(saBind),
            Window("",0,0,0,0),
            m_window(nullptr),
            m_renderer(nullptr),
            m_board(0, 0, SCREEN_WIDTH, SCREEN_WIDTH),
            m_lastTicks(0),
            m_pstatusLabel(nullptr)
    {
        memset(m_expectedPosition,0,sizeof m_expectedPosition);
        memset(m_moveActions,0,sizeof m_moveActions);
        memset(m_moveLocations,0,sizeof m_moveLocations);
        m_movesNeeded = 0;
        m_moveIndex = 0;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
            return;
        }
        m_window = SDL_CreateWindow(
                "Chessbox Simulator",
                50, 50,
                SCREEN_WIDTH, SCREEN_HEIGHT+BUTTON_HEIGHT,
                SDL_WINDOW_RESIZABLE);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
        setupFonts();
        m_renderer = SDL_CreateRenderer(m_window, -1, 0);
        if(!m_renderer) {
            printf("m_renderer error %s\n",SDL_GetError());
        }
        addLogo();
        addStatusLabel(saBind);

        setupDefaultChessPosition();
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        draw(m_renderer);
        SDL_RenderPresent(m_renderer);
    }
    ~SimServer() {
        SDL_DestroyWindow(m_window);
        SDL_DestroyRenderer(m_renderer);
        if(m_pstatusLabel) {
            delete m_pstatusLabel;
            m_pstatusLabel = nullptr;
        }
        SDL_Quit();
    }
    void setupFonts() {
        FontManager::instance()->add("small","Inconsolata-Medium.ttf",10);
        FontManager::instance()->add("normal","Inconsolata-Medium.ttf",16);
        FontManager::instance()->add("large","Inconsolata-Medium.ttf",26);
    }
    void addLogo() {
        Sprite* logo = new Sprite(m_renderer, "assets/logo-sm.png");
        addComponent(logo);
    }
    void addStatusLabel(const SockAddr& saBind) {
        //status label to show what port sim is waiting on
        char buff[100];
        snprintf(buff,sizeof(buff),"Sim waiting for connection on port %u",saBind.port());
        m_pstatusLabel = new Label("status", 10, 10, SCREEN_WIDTH, SCREEN_WIDTH);
        m_pstatusLabel->setText(buff);
        addComponent(m_pstatusLabel);
    }
    virtual void connected(SocketInstance sock,SockAddr& sa) {
        simlog("CLIENT CONNECTED from %s", sa.dottedDecimal());
        simlog("m_renderer is %s", m_renderer ? "VALID" : "NULL");
        char buff[80];
        snprintf(buff,sizeof(buff),"Connection from %s opened\n",sa.dottedDecimal());
        m_pstatusLabel->setText(buff);
        json j;
        j["message"] = "Chessbox controller says hello";
        j["version"] = "1.00.00";
        j["success"] = true;
        println(sock, j.dump().c_str());
        addComponent(&m_board);
        simlog("Board added, component count = %zu", m_components.size());
        int x=0,y=SCREEN_HEIGHT,w=BUTTON_WIDTH,h=BUTTON_HEIGHT,gap=5;
        TTF_Font* font = FontManager::instance()->font("normal");
        SDL_Color btnColor = {102, 217, 0, 255}; // lime green
        TextButton* level=new TextButton(SETUP_BUTTON_ID,"Setup",x,y,w,h,font);
        level->setBackgroundColor(btnColor);
        addButton(level);
        addComponent(level);
        TextButton* clearbn=new TextButton(CLEAR_BUTTON_ID,"Clear",x+w+1,y,w,h,font);
        clearbn->setBackgroundColor(btnColor);
        addButton(clearbn);
        addComponent(clearbn);

        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);
        draw(m_renderer);
        SDL_RenderPresent(m_renderer);
        simlog("connected() draw complete");
    }
    virtual void closed(SocketInstance sock,SockAddr& sa) {
        printf("Connection from %s closed\n",sa.dottedDecimal());
    }
    int parseFrom(const char* lan) {
        int col=tolower(lan[0])-'a';
        int row=8-(lan[1]-'1')-1;
        return row*8+col;
    }
    int parseTo(const char* lan) {
        int col=tolower(lan[2])-'a';
        int row=8-(lan[3]-'1')-1;
        return row*8+col;
    }
    int toPos(int x,int y) {
        if (m_board.isFlipped()) {
            x = 7 - x;
            y = 7 - y;
        }
        return y*8+x;
    }
    virtual void processLine(SocketInstance sock,SockAddr& sa,char* line) {
        // Skip logging frequent heartbeat messages
        if (strstr(line, "highlight") == nullptr)
            simlog("RECV: %s", line);
        try {
            json src=json::parse(line);
            json result;
            result["success"]=false;
            if(src.contains("action")) {
                string action=src["action"];
                if(action.compare("quit")==0) {
                    //if we quit, the socket gets closed
                    quit(sock, src, result);
                } else {
                    if(action.compare("ping")==0) ping(sock, src, result);
                    else if(action.compare("setup_pieces")==0) setupPieces(sock, src, result);
                    else if(action.compare("reset_board")==0) { clearAllPiecesLeds(); setupDefaultChessPosition(); result["success"]=true; }
                    else if(action.compare("flip_board")==0) { bool f=src.value("flipped",false); m_board.setFlipped(f); result["success"]=true; }
                    else if(action.compare("flash")==0) flash(sock, src, result);
                    else if(action.compare("led")==0) led(sock, src, result);
                    else if(action.compare("led_all")==0) ledAll(sock, src, result);
                    else if(action.compare("move")==0) move(sock, src, result);
                    else if(action.compare("highlight")==0) highlight(sock, src, result);
                    else if(action.compare("query_leds")==0) queryLeds(sock, src, result);
                    else if(action.compare("query_pieces")==0) queryPieces(sock, src, result);
                    // Only send responses for queries — skip for fire-and-forget actions
                    if(action.compare("ping")==0 ||
                       action.compare("query_leds")==0 ||
                       action.compare("query_pieces")==0) {
                        println(sock, result.dump().c_str());
                    }
                }
            } else {
                result["errors"] = {"missing action"};
                println(sock,result.dump().c_str());
            }
        } catch (json::exception& ex) {
            printf("parse error at byte %s\n",ex.what());
            json j;
            j["success"] = false;
            j["errors"] = {"invalid json"};
            try { println(sock,j.dump().c_str()); } catch (...) {}
        } catch (std::exception& ex) {
            simlog("processLine exception: %s", ex.what());
        } catch (...) {
            simlog("processLine unknown exception");
        }
    }

    void quit(SocketInstance sock,json& src,json& result) {
        result["success"]=true;
        result["message"]="Good bye";
        println(sock,result.dump().c_str());
        sock.close();
    }
    void ping(SocketInstance sock,json& src,json& result) {
        result["success"]=true;
        result["action"]="pong";
    }
    void flash(SocketInstance sock,json& src,json& result) {
        bool on=src["on"];
        auto squares=src["squares"];
        if (!on && squares.empty()) {
            // Clear all flashing
            for (int i = 0; i < 64; i++) m_board.setFlashing(i, false);
        } else {
            for(unsigned i=0; i<squares.size(); i++) {
                std::string sqStr = squares.at(i);
                int col = tolower(sqStr[0]) - 'a';
                int row = 8 - (sqStr[1] - '1') - 1;
                int sq = row * 8 + col;
                m_board.setFlashing(sq, on);
            }
        }
        result["success"]=true;
    }
    void led(SocketInstance sock,json& src,json& result) {
        bool on=src["on"];
        auto squares = src["squares"];
        for(unsigned i=0; i<squares.size(); i++) {
            int sq=squares.at(i);
            m_board.setLed(sq, on);
        }
        result["success"]=true;
    }
    void ledAll(SocketInstance sock,json& src,json& result) {
        bool on=src["on"];
        m_board.setLedAll(on);
        result["success"]=true;
    }
    void move(SocketInstance sock, json& src, json& result) {
        string lan = src["lan"];
        int from = parseFrom(lan.c_str());
        int to = parseTo(lan.c_str());

        simlog("MOVE action: lan=%s from=%d to=%d piece_at_from=%s",
               lan.c_str(), from, to, m_board.piece(from) ? "YES" : "NO");

        // Update piece positions. Track destination as m_engineDest so
        // processPieceUp won't accidentally clear it during engine move confirmation.
        // Check if this is a castling rook move first
        bool isCastleRookMove = (
            (from==63 && to==61) || (from==56 && to==59) ||
            (from==7  && to==5)  || (from==0  && to==3)
        );
        // Use explicit "engine" flag from controller instead of guessing
        bool wasEngineMove = !isCastleRookMove && src.value("engine", false);
        m_board.setPiece(from, false);
        m_board.setFlashing(from, false);
        m_board.setLed(from, false);
        m_board.setPiece(to, true);
        if (wasEngineMove) {
            m_engineDest = to;
            m_engineSrc = from;
        }
        // Clear all LEDs on every move — controller heartbeat will re-light correct squares
        m_board.setLedAll(false);
        memset(m_lastHighlight, 0, sizeof(m_lastHighlight));

        simlog("MOVE: from=%d to=%d engine=%d", from, to, wasEngineMove ? 1 : 0);
        // Mark castling rook moves to ignore physical clicks
        // Only for the specific rook squares involved in castling
        if (isCastleRookMove) {
            m_ignoreCastle[from] = true;
        }
        result["success"] = true;
    }
    void highlight(SocketInstance sock, json& src, json& result) {
        bool newHL[NUM_SQUARES] = {};
        std::string squareList = "";
        if (src.contains("squares")) {
            for (auto& sq : src["squares"]) {
                std::string lanStr = sq.get<std::string>();
                squareList += lanStr + " ";
                if (lanStr.size() >= 2) {
                    int col = tolower(lanStr[0]) - 'a';
                    int row = '8' - lanStr[1];
                    int pos = row * 8 + col;
                    if (pos >= 0 && pos < NUM_SQUARES) newHL[pos] = true;
                }
            }
        }
        bool anySet = false;
        for (int i = 0; i < NUM_SQUARES; i++) if (newHL[i]) { anySet = true; break; }
        if (!anySet) {
            m_board.setLedAll(false);
            memset(m_lastHighlight, 0, sizeof(m_lastHighlight));
        } else {
            for (int i = 0; i < NUM_SQUARES; i++) {
                if (m_lastHighlight[i] && !newHL[i]) m_board.setLed(i, false);
                if (newHL[i]) m_board.setLed(i, true);
                m_lastHighlight[i] = newHL[i];
            }
        }
        result["success"] = true;
    }
    void queryLeds(SocketInstance sock,json& src,json& result) {
        json leds = json::array({});
        for(int i=0; i<NUM_SQUARES; i++) {
            if(m_board.led(i)) {
                leds.push_back(i);
            }
        }
        result["leds_on"] = leds;
        result["success"] = true;
    }
    void queryPieces(SocketInstance sock,json& src,json& result) {
        json switches = json::array({});
        for(int i=0; i<NUM_SQUARES; i++) {
            if(m_board.piece(i)) {
                switches.push_back(i);
            }
        }
        result["has_piece"] = switches;
        result["success"] = true;
    }
    void setupPieces(SocketInstance sock,json& src,json& result) {
            vector<int> squares = src["squares"];
            m_board.setLedAll(false);
            m_gameMode = GAME_MODE_IDLE;                //assume the board is setup correctly
            memset(m_expectedPosition,0,sizeof m_expectedPosition);
            for(int i=0; i<NUM_SQUARES; i++) {
                //check if this square should be set
                if(containsSquare(i,squares)) {
                    m_expectedPosition[i] = 1;
                    if(!m_board.piece(i)) {
                        m_board.setLed(i,true);
                        m_gameMode = GAME_MODE_SETUP;   //user has to manipulate the board, go into setup mode.
                    }
                } else {
                    if(m_board.piece(i)) {
                        m_board.setFlashing(i,true);
                        m_gameMode = GAME_MODE_SETUP;   //user has to manipulate the board, go into setup mode.
                    }
                }
            }
            result["success"]=true;
    }
    void processPieceUp(int pos) {
        if (m_ignoreCastle[pos]) { m_ignoreCastle[pos] = false; m_pendingMoveFrom = -1; return; }
        // Don't clear piece if this is the engine move destination being confirmed
        if (pos == m_engineDest) {
            m_engineDest = -1;
            m_pendingMoveFrom = -1;
            m_board.setLed(pos, false);
            memset(m_lastHighlight, 0, sizeof(m_lastHighlight));
            // Send piece_up to controller so it can confirm the engine move
            if (m_sClient != 0) {
                json j;
                j["success"] = true;
                j["action"] = PIECE_UP;
                j["square"] = pos;
                j["lan"] = m_board.lan(pos);
                try { println(m_sClient, j.dump().c_str()); } catch (...) {}
            }
            return;
        }
        // New move starting - clear any stale engine tracking
        m_engineDest = -1;
        m_engineSrc = -1;
        m_board.setPiece(pos, false);
        m_board.setFlashing(pos, false);  // Clear hint flash if any

        if(m_sClient!=0) {
            json j;
            j["success"]=true;
            j["action"]=PIECE_UP;
            j["square"]=pos;
            j["lan"]=m_board.lan(pos);
            try { println(m_sClient, j.dump().c_str()); } catch (...) {}
        }
    }
    void processPieceDown(int pos) {
        if (m_ignoreCastle[pos]) { m_ignoreCastle[pos] = false; m_pendingMoveFrom = -1; return; }
        if (pos == m_engineSrc) {
            m_engineSrc = -1;
            m_board.setLed(pos, false);
            return;
        }
        m_board.setPiece(pos,true);
        m_board.setFlashing(pos, false);  // Clear hint flash if any

        if(m_sClient!=0) {
            json j;
            j["success"]=true;
            j["action"]=PIECE_DOWN;
            j["square"]=pos;
            j["lan"]=m_board.lan(pos);
            try { println(m_sClient, j.dump().c_str()); } catch (...) {}
        }
        if (false) {  // Legacy GAME_MODE_SETUP placeholder

            }  // end legacy placeholder
    }

    //turn off all pieces and leds
    void clearAllPiecesLeds() {
        for(int i=0; i<64; i++) {
            m_board.setPiece(i, false);
            m_board.setLed(i, false);
            m_board.setFlashing(i,false);
        }
    }

    //turn on pieces where new game would be
    void setupDefaultChessPosition() {
        //put pieces on A8 thru H7
        for(int i=0; i<16; i++) {
            m_board.setPiece(i, true);
        }
        //put pieces on A2 thru H1
        for(int i=48; i<64; i++) {
            m_board.setPiece(i, true);
        }
    }

    virtual void processButtonClicked(Button* b) {
        if(b->id() == SETUP_BUTTON_ID) {
            clearAllPiecesLeds();
            setupDefaultChessPosition();
            m_gameMode = GAME_MODE_PLAYING;
            // Tell controller to reset
            if (m_sClient != 0) {
                json j;
                j["action"] = "reset";
                try { println(m_sClient, j.dump().c_str()); } catch (...) {}
            }
        } else if(b->id() == CLEAR_BUTTON_ID) {
            clearAllPiecesLeds();
        }
    }

    void mouse(SDL_Event* event) {
        int w=SCREEN_WIDTH / 8;
        int h=SCREEN_WIDTH / 8;
        int x=event->button.x / w;
        int y=event->button.y / h;
        if(event->button.y<SCREEN_WIDTH) {    //yup width, since the board is square
            int pos=toPos(x, y);
            if(m_board.piece(pos) && m_pendingMoveFrom == -1) {
                // No piece in hand - lift this piece
                m_pendingMoveFrom = pos;
                processPieceUp(pos);
            } else if(m_pendingMoveFrom != -1) {
                // Piece in hand - place it (capture or normal move)
                m_pendingMoveFrom = -1;
                processPieceDown(pos);
            } else if (m_engineSrc != -1) {
                // Empty square with no piece in hand, but engine move pending
                // Allow click on engine src square for confirmation
                processPieceDown(pos);
            }
            // Ignore clicks on empty squares with no piece in hand and no engine move pending
        }
    }

    virtual void idle() {
        static int idleCount = 0;
        static long lastRenderTicks = 0;
        idleCount++;
        if (idleCount % 500 == 0) {
            simlog("idle() tick %d, components=%zu, renderer=%s", 
                   idleCount, m_components.size(), m_renderer ? "VALID" : "NULL");
        }
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.y < SCREEN_WIDTH)
                        mouse(&event);
                    else
                        processMouseEvent(&event);
                    break;
                case SDL_MOUSEBUTTONUP:
                    break;
                case SDL_QUIT:
                    m_bRunning=false;
                    break;
                case SDL_KEYDOWN:
                    if(SDL_SCANCODE_ESCAPE==event.key.keysym.scancode)
                        m_bRunning=false;
                    break;
                default:
                    break;
            }
        }
        long now = SDL_GetTicks();
        update(now);
        // Only redraw at ~30fps (every 33ms) to avoid blocking TCP processing
        if (now - lastRenderTicks >= 33) {
            lastRenderTicks = now;
            SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
            SDL_RenderClear(m_renderer);
            SDL_Rect redArea = {0, SCREEN_HEIGHT, SCREEN_WIDTH, BUTTON_HEIGHT};
            SDL_SetRenderDrawColor(m_renderer, 200, 0, 0, 255);
            SDL_RenderFillRect(m_renderer, &redArea);
            draw(m_renderer);
            SDL_RenderPresent(m_renderer);
        }
        switch(m_gameMode) {
            case GAME_MODE_MOVE: idleMove(); break;
        }
    }
    virtual void update(long ticks) {
        Window::update(ticks);
        long delta = ticks-m_lastTicks;
        if(delta>=FLASH_SPEED) {
            m_board.toggleFlash();
            m_lastTicks = ticks;
        }
    }
    void idleMove() {

    }

    bool containsSquare(int sq,vector<int> squares) {
        for (vector<int>::iterator it = squares.begin() ; it != squares.end(); ++it) {
            if(sq == *it) return true;
        }
        return false;
    }
};

int main(int argc,char* argv[]) {
    int port=9999;
    SockAddr saBind((ULONG)INADDR_ANY,port);
    SimServer server(saBind);
    printf("Server running on port %d\n",port);
    server.runServer();
    printf("Done\n");
    return 0;
}