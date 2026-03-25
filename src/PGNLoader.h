#ifndef PGNLOADER_H
#define PGNLOADER_H

#include <string>
#include <vector>
#include <utility>

class PGNLoader {
public:
    bool load(const std::string& pgnText);
    bool loadFile(const std::string& path);

    const std::vector<std::string>& moves() const { return m_moves; }
    const std::string& header(const std::string& key) const;
    bool empty() const { return m_moves.empty(); }
    void clear() { m_moves.clear(); m_headers.clear(); }

private:
    std::vector<std::string>                         m_moves;
    std::vector<std::pair<std::string,std::string>>  m_headers;
    std::string m_empty;

    void parseHeaders(const std::string& text);
    void parseMoves(const std::string& text);
    std::string stripComments(const std::string& text);
};

#endif