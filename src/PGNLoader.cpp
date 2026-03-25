#include "PGNLoader.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cctype>

bool PGNLoader::loadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "PGNLoader: cannot open %s\n", path.c_str());
        return false;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return load(ss.str());
}

bool PGNLoader::load(const std::string& pgnText) {
    clear();
    parseHeaders(pgnText);
    parseMoves(pgnText);
    fprintf(stderr, "PGNLoader: loaded %zu moves\n", m_moves.size());
    return !m_moves.empty();
}

void PGNLoader::parseHeaders(const std::string& text) {
    // Parse [Key "Value"] pairs manually
    size_t pos = 0;
    while (pos < text.size()) {
        size_t lb = text.find('[', pos);
        if (lb == std::string::npos) break;
        size_t rb = text.find(']', lb);
        if (rb == std::string::npos) break;

        std::string tag = text.substr(lb + 1, rb - lb - 1);
        // Find key (first word)
        size_t ks = 0;
        while (ks < tag.size() && isspace(tag[ks])) ks++;
        size_t ke = ks;
        while (ke < tag.size() && !isspace(tag[ke])) ke++;
        std::string key = tag.substr(ks, ke - ks);

        // Find value between quotes
        size_t q1 = tag.find('"', ke);
        size_t q2 = (q1 != std::string::npos) ? tag.find('"', q1 + 1) : std::string::npos;
        std::string value;
        if (q1 != std::string::npos && q2 != std::string::npos)
            value = tag.substr(q1 + 1, q2 - q1 - 1);

        if (!key.empty())
            m_headers.push_back(std::make_pair(key, value));

        pos = rb + 1;
    }
}

std::string PGNLoader::stripComments(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    int braceDepth = 0;
    bool inLine = false;
    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '{') { braceDepth++; continue; }
        if (c == '}') { if (braceDepth > 0) braceDepth--; continue; }
        if (braceDepth > 0) continue;
        if (c == ';') { inLine = true; continue; }
        if (inLine && c == '\n') { inLine = false; result += ' '; continue; }
        if (inLine) continue;
        result += c;
    }
    return result;
}

void PGNLoader::parseMoves(const std::string& text) {
    // Find move section — after last header
    size_t pos = 0;
    while (pos < text.size()) {
        size_t lb = text.find('[', pos);
        if (lb == std::string::npos) break;
        size_t rb = text.find(']', lb);
        if (rb == std::string::npos) break;
        pos = rb + 1;
    }

    std::string moveSection = stripComments(text.substr(pos));

    std::istringstream iss(moveSection);
    std::string token;
    while (iss >> token) {
        if (token.empty()) continue;

        // Skip result tokens
        if (token == "1-0" || token == "0-1" || token == "1/2-1/2" || token == "*") continue;

        // Strip move number prefix like "1." "12." or "1..." 
        size_t dotPos = token.rfind('.');
        if (dotPos != std::string::npos) {
            token = token.substr(dotPos + 1);
            if (token.empty()) continue;
        }

        // Skip pure numbers
        if (!token.empty() && isdigit(token[0])) continue;

        // Valid SAN: starts with letter or 'O' (castling)
        if (!token.empty() && (isalpha(token[0]) || token[0] == 'O')) {
            // Strip trailing annotations like !, ?, +, #
            while (!token.empty() && (token.back() == '!' || token.back() == '?'))
                token.pop_back();
            if (!token.empty())
                m_moves.push_back(token);
        }
    }
}

const std::string& PGNLoader::header(const std::string& key) const {
    for (const auto& h : m_headers)
        if (h.first == key) return h.second;
    return m_empty;
}