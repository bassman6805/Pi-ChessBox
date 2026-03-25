#ifndef UCICLIENT_H
#define UCICLIENT_H

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include <sstream>
#include "process.hpp" // TinyProcessLib

class UCIClient {
public:
    UCIClient(std::string path) : m_hasNewMove(false), m_skillLevel(5), m_depth(8), m_moveTimeMs(3000), m_process(path, "", 
        [this](const char* bytes, size_t n) {
            std::string output(bytes, n);
            std::cout << "[STOCKFISH] " << output << std::flush;

            size_t pos = output.find("bestmove ");
            if (pos != std::string::npos) {
                std::lock_guard<std::mutex> lock(m_moveMutex);
                std::string movePart = output.substr(pos + 9);
                std::stringstream ss(movePart);
                ss >> m_lastBestMove; 
                m_hasNewMove = true;
            }
        },
        nullptr, 
        true 
    ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        sendUCINewGame();
        setSkillLevel(m_skillLevel);
    }

    void setPosition(const std::string& fen) {
        std::string cmd = "position fen " + fen + "\n";
        m_process.write(cmd);
    }

    void sendGo(int ms) {
        m_hasNewMove = false;
        std::string cmd = "go";
        if (m_depth > 0)
            cmd += " depth " + std::to_string(m_depth);
        if (m_moveTimeMs > 0)
            cmd += " movetime " + std::to_string(m_moveTimeMs);
        if (m_depth == 0 && m_moveTimeMs == 0)
            cmd += " movetime " + std::to_string(ms); // fallback
        cmd += "\n";
        m_process.write(cmd);
    }

    void setMoveTime(int ms) {
        m_moveTimeMs = ms; // 0 = unlimited
        fprintf(stderr, "MOVETIME: set to %d ms\n", ms);
    }

    int getMoveTime() const { return m_moveTimeMs; }

    void setDepth(int depth) {
        if (depth < 1) depth = 1;
        if (depth > 15) depth = 15;
        m_depth = depth;
        fprintf(stderr, "DEPTH: set to %d\n", depth);
    }

    int getDepth() const { return m_depth; }

    void sendUCINewGame() {
        m_process.write("uci\nucinewgame\nisready\n");
    }

    void setSkillLevel(int level) {
        if (level < 0) level = 0;
        if (level > 20) level = 20;
        m_skillLevel = level;
        std::string cmd = "setoption name Skill Level value " + std::to_string(level) + "\n";
        m_process.write(cmd);
        fprintf(stderr, "SKILL LEVEL: set to %d\n", level);
    }

    int getSkillLevel() const { return m_skillLevel; }

    bool hasNewMove() { return m_hasNewMove; }

    std::string getAndClearMove() {
        std::lock_guard<std::mutex> lock(m_moveMutex);
        m_hasNewMove = false;
        return m_lastBestMove;
    }

    bool isStarted() { 
        return m_process.get_id() > 0; 
    } 

private:
    TinyProcessLib::Process m_process;
    std::mutex m_moveMutex;
    std::string m_lastBestMove;
    std::atomic<bool> m_hasNewMove;
    int m_skillLevel;
    int m_depth;
    int m_moveTimeMs; // 0 = unlimited, >0 = cap in ms
};

#endif