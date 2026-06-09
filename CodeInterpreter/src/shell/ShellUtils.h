#pragma once

#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "CheckError.h"
#include "ParseError.h"
#include "RuntimeError.h"

// ── 1. 에러 타입별 stderr 출력 ───────────────────────────────────────
inline void printError(const ParseError& e)        { std::cerr << "[구문 오류] "   << e.what() << "\n"; }
inline void printError(const CheckError& e)        { std::cerr << "[의미 오류] "   << e.what() << "\n"; }
inline void printError(const RuntimeError& e)      { std::cerr << "[런타임 오류] " << e.what() << "\n"; }
inline void printError(const std::runtime_error& e){ std::cerr << "[오류] "        << e.what() << "\n"; }

// ── 2. 파일을 줄 단위로 읽어 반환 (실패 시 예외) ──────────────────────
inline std::vector<std::string> readFileLines(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("[오류] 파일을 찾을 수 없습니다: " + path);
    std::vector<std::string> lines;
    std::string ln;
    while (std::getline(file, ln)) lines.push_back(ln);
    return lines;
}

// ── 3. 빈 줄 기준 청크 분리 후 handler 호출 ──────────────────────────
// handler(startLine, src) → false 반환 시 즉시 중단
// handler 내부에서 던진 예외는 그대로 전파됨
inline void forEachChunk(
    const std::vector<std::string>& lines,
    const std::function<bool(int startLine, const std::string& src)>& handler)
{
    std::ostringstream current;
    int chunkStart = 0;
    for (int i = 0; i <= (int)lines.size(); i++) {
        bool isBlank = (i == (int)lines.size()) ||
                       lines[i].find_first_not_of(" \t\r\n") == std::string::npos;
        if (isBlank) {
            if (!current.str().empty()) {
                if (!handler(chunkStart, current.str())) return;
                current.str(""); current.clear();
            }
            chunkStart = i + 1;
        } else {
            current << lines[i] << '\n';
        }
    }
}
