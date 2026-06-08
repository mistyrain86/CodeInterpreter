#include "Debugger.h"
#include "Interpreter.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include "Token.h"
#include "Value.h"
#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>

// 값 타입 이름 반환
static std::string typeName(const Value& v) {
    if (std::holds_alternative<std::monostate>(v))             return "Nil";
    if (std::holds_alternative<double>(v))                     return "Number";
    if (std::holds_alternative<std::string>(v))                return "String";
    if (std::holds_alternative<bool>(v))                       return "Boolean";
    if (std::holds_alternative<ArrayType>(v))                  return "Array";
    return "Function";
}

Debugger::Debugger(const std::string& path) : m_path(path) {}

// ── 디버그 루프 진입 ──────────────────────────────────────────────────
void Debugger::run() {
    // 1. 파일 읽기
    std::ifstream file(m_path);
    if (!file.is_open()) {
        std::cerr << "[오류] 파일을 찾을 수 없습니다: " << m_path << "\n";
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    // 2. 소스 줄 캐싱 (줄 번호 표시용)
    std::istringstream lineStream(source);
    std::string ln;
    while (std::getline(lineStream, ln)) m_sourceLines.push_back(ln);

    // 3. 파이프라인 구성 + StmtHook 등록
    LangFactory factory;
    auto* interp = dynamic_cast<Interpreter*>(factory.getInterpreter());

    interp->setStmtHook([this, interp](Stmt& stmt) {
        onBeforeStmt(stmt, *interp);
    });

    std::cout << "[DEBUG] 소스코드 로딩: " << m_path << "\n";

    // 4. 실행 (Lex → Parse → Check → Interpret)
    try {
        factory.run(source);
    }
    catch (const ParseError& e)        { std::cerr << "[구문 오류] "   << e.what() << "\n"; }
    catch (const CheckError& e)        { std::cerr << "[의미 오류] "   << e.what() << "\n"; }
    catch (const RuntimeError& e)      { std::cerr << "[런타임 오류] " << e.what() << "\n"; }
    catch (const std::runtime_error& e){ std::cerr << "[오류] "        << e.what() << "\n"; }

    std::cout << "[DEBUG] 실행 완료\n";
}

// ── Stmt 실행 직전 호출되는 Hook ────────────────────────────────────
void Debugger::onBeforeStmt(Stmt& stmt, Interpreter& interp) {
    int  line         = stmt.getLine();
    bool atBreakpoint = m_breakpoints.count(line) > 0;

    // 정지 조건 판단
    if (!atBreakpoint) {
        if (!m_stepMode) return;                              // continue 모드
        if (interp.executeDepth() > m_nextDepth) return;     // next: 더 깊은 stmt 건너뜀
    }

    // 해당 줄 소스 텍스트 (앞 공백 제거)
    std::string srcLine;
    if (line > 0 && line <= (int)m_sourceLines.size()) {
        srcLine = m_sourceLines[line - 1];
        srcLine.erase(0, srcLine.find_first_not_of(" \t"));
    }

    // 정지 알림
    if (atBreakpoint)
        std::cout << "[DEBUG] " << line << "번째 줄에서 정지 (breakpoint) -> " << srcLine << "\n";
    else
        std::cout << "[DEBUG] " << line << "번째 줄에서 정지 -> " << srcLine << "\n";

    printWatches(interp);

    // 커맨드 루프 — step/next/continue 입력 시 탈출
    std::string input;
    while (true) {
        std::cout << "> ";
        std::cout.flush();
        if (!std::getline(std::cin, input)) return;

        if (input == "step") {
            m_stepMode  = true;
            m_nextDepth = INT_MAX;   // 모든 깊이에서 정지
            break;
        }
        if (input == "next") {
            m_stepMode  = true;
            m_nextDepth = interp.executeDepth();  // 현재 깊이에서만 정지
            break;
        }
        if (input == "continue") {
            m_stepMode  = false;
            m_nextDepth = 0;
            break;
        }
        processCommand(input, interp);
    }
}

// ── 커맨드 디스패처 ───────────────────────────────────────────────────
void Debugger::processCommand(const std::string& input, Interpreter& interp) {
    if (input.empty()) return;   // 빈 Enter 무시
    if (input.size() > 6 && input.substr(0, 6) == "break ") {
        try { cmdBreak(std::stoi(input.substr(6))); }
        catch (...) { std::cout << "사용법: break <줄번호>\n"; }
    }
    else if (input.size() > 7 && input.substr(0, 7) == "remove ") {
        try { cmdRemove(std::stoi(input.substr(7))); }
        catch (...) { std::cout << "사용법: remove <줄번호>\n"; }
    }
    else if (input == "Breakpoints") {
        cmdBreakpoints();
    }
    else if (input.size() > 6 && input.substr(0, 6) == "watch ") {
        cmdWatch(input.substr(6));
    }
    else if (input.size() > 8 && input.substr(0, 8) == "unwatch ") {
        cmdUnwatch(input.substr(8));
    }
    else if (input == "watched") {
        cmdWatched(interp);
    }
    else if (input == "inspect") {
        cmdInspect(interp);
    }
    else {
        std::cout << "알 수 없는 커맨드: " << input << "\n";
        std::cout << "  step            다음 stmt에서 정지 (블록 내부 진입)\n";
        std::cout << "  next            다음 stmt에서 정지 (블록 내부 건너뜀)\n";
        std::cout << "  continue        다음 breakpoint까지 실행\n";
        std::cout << "  break <줄>      breakpoint 설정\n";
        std::cout << "  remove <줄>     breakpoint 해제\n";
        std::cout << "  Breakpoints     breakpoint 목록 출력\n";
        std::cout << "  watch <변수>    변수 감시 등록\n";
        std::cout << "  unwatch <변수>  변수 감시 해제\n";
        std::cout << "  watched         감시 중인 변수 목록과 현재 값 출력\n";
        std::cout << "  inspect         현재 스코프 전체 변수/값/타입 출력\n";
    }
}

// ── 개별 커맨드 구현 ──────────────────────────────────────────────────

void Debugger::cmdBreak(int line) {
    m_breakpoints.insert(line);
    std::cout << "[DEBUG] " << line << "번째 줄에 breakpoint 설정\n";
}

void Debugger::cmdRemove(int line) {
    m_breakpoints.erase(line);
    std::cout << "[DEBUG] " << line << "번째 줄 breakpoint 제거\n";
}

void Debugger::cmdBreakpoints() {
    if (m_breakpoints.empty()) {
        std::cout << "설정된 breakpoint 없음\n";
        return;
    }
    std::cout << "[Breakpoints]\n";
    for (int bp : m_breakpoints) std::cout << "  줄 " << bp << "\n";
}

void Debugger::cmdWatch(const std::string& var) {
    m_watches.insert(var);
    std::cout << "[WATCH] '" << var << "' 감시 등록\n";
}

void Debugger::cmdUnwatch(const std::string& var) {
    m_watches.erase(var);
    std::cout << "[WATCH] " << var << " 감시 해제\n";
}

void Debugger::cmdWatched(Interpreter& interp) {
    if (m_watches.empty()) {
        std::cout << "감시 중인 변수 없음\n";
        return;
    }
    for (const auto& var : m_watches) {
        try {
            Token t{TokenType::IDENTIFIER, var, std::monostate{}, 0};
            Value v = interp.currentEnv()->get(t);
            std::cout << "[WATCH] " << var << " = " << interp.stringify(v) << "\n";
        } catch (...) {
            std::cout << "[WATCH] " << var << " = (미정의)\n";
        }
    }
}

void Debugger::cmdInspect(Interpreter& interp) {
    std::cout << "-- 현재 스코프 변수 ----------\n";

    // env 체인: currentEnv(로컬) → ... → global(enclosing==nullptr)
    std::vector<const Environment*> chain;
    const Environment* cur = interp.currentEnv().get();
    while (cur) {
        chain.push_back(cur);
        cur = cur->m_enclosing.get();
    }

    // chain[0] = 가장 안쪽(로컬), chain[last] = 전역
    for (int i = 0; i < (int)chain.size(); i++) {
        bool        isGlobal = (i == (int)chain.size() - 1);
        std::string label    = isGlobal ? "[전역]" : "[로컬]";
        for (const auto& [k, v] : chain[i]->m_values) {
            if (k == "Array") continue;   // 내장 함수 제외
            std::cout << label << " " << k
                      << " = "  << interp.stringify(v)
                      << " ("   << typeName(v) << ")\n";
        }
    }
}

void Debugger::printWatches(Interpreter& interp) {
    for (const auto& var : m_watches) {
        try {
            Token t{TokenType::IDENTIFIER, var, std::monostate{}, 0};
            Value v = interp.currentEnv()->get(t);
            std::cout << "[WATCH] " << var << " = " << interp.stringify(v) << "\n";
        }
        catch (...) {
            std::cout << "[WATCH] " << var << " = (미정의)\n";
        }
    }
}
