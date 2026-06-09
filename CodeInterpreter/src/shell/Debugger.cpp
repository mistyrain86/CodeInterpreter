#include "Debugger.h"
#include "IInterpreter.h"
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

static std::string typeName(const Value& v) {
    if (std::holds_alternative<std::monostate>(v))             return "Nil";
    if (std::holds_alternative<double>(v))                     return "Number";
    if (std::holds_alternative<std::string>(v))                return "String";
    if (std::holds_alternative<bool>(v))                       return "Boolean";
    if (std::holds_alternative<ArrayType>(v))                  return "Array";
    return "Function";
}

Debugger::Debugger(const std::string& path) : m_path(path) {}

void Debugger::run() {
    std::ifstream file(m_path);
    if (!file.is_open()) {
        std::cerr << "[오류] 파일을 찾을 수 없습니다: " << m_path << "\n";
        return;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    run(ss.str(), std::cin);
}

void Debugger::run(const std::string& source, std::istream& cmdIn) {
    m_sourceLines.clear();
    std::istringstream lineStream(source);
    std::string ln;
    while (std::getline(lineStream, ln)) m_sourceLines.push_back(ln);

    LangFactory factory;
    IInterpreter* interpIface = factory.getInterpreter();
    auto*         interp      = dynamic_cast<Interpreter*>(interpIface);
    if (!interp) return;

    interpIface->setStmtHook([this, interp, &cmdIn](Stmt& stmt) {
        onBeforeStmt(stmt, *interp, cmdIn);
    });

    std::cout << "CodeFab Interpreter (DEBUG 모드)\n";
    std::cout << "종료: exit 또는 quit\n";
    std::cout << "[DEBUG] 소스코드 로딩: " << m_path << "\n";

    // 각 청크 앞에 빈 줄 prefix를 붙여 파서의 줄 번호를 유지
    bool hasError = false;

    // 커맨드 루프 헬퍼 (gap 정지 / 오류 전 정지 공용)
    auto commandLoop = [&]() {
        std::string input;
        while (true) {
            std::cout << "> ";
            std::cout.flush();
            if (!std::getline(cmdIn, input)) return;
            if (input == "step")     { m_stepMode = true;  m_nextDepth = INT_MAX;               return; }
            if (input == "next")     { m_stepMode = true;  m_nextDepth = interp->executeDepth(); return; }
            if (input == "continue") { m_stepMode = false; m_nextDepth = 0;                     return; }
            if (input == "exit" || input == "quit") {
                std::cout << "[DEBUG] 디버그 세션을 종료합니다.\n";
                throw DebugSessionExit{};
            }
            processCommand(input, *interp);
        }
    };

    // step 모드에서 파싱 오류 발생 전 해당 줄에서 정지
    auto stepPauseBeforeError = [&](int firstLine) {
        if (!m_stepMode || interp->executeDepth() > m_nextDepth) return;
        if (m_lastStmtLine >= firstLine) return;   // 이미 이 줄에서 멈췄으면 재발동 방지
        std::string srcLine;
        if (firstLine > 0 && firstLine <= (int)m_sourceLines.size()) {
            srcLine = m_sourceLines[firstLine - 1];
            srcLine.erase(0, srcLine.find_first_not_of(" \t"));
        }
        if (srcLine.empty()) srcLine = "(빈 줄)";
        std::cout << "[DEBUG] " << firstLine << "번째 줄에서 정지 -> " << srcLine << "\n";
        printWatches(*interp);
        commandLoop();
        m_lastStmtLine = firstLine;
    };

    auto runChunk = [&](int startLine, const std::string& chunk) {
        std::string source(startLine, '\n');  // 줄 번호 offset 유지
        source += chunk;
        try {
            factory.run(source);
        }
        catch (const DebugSessionExit&)    { throw; }
        catch (const ParseError& e)        { stepPauseBeforeError(startLine + 1); std::cerr << "[구문 오류] "   << e.what() << "\n"; hasError = true; }
        catch (const CheckError& e)        { stepPauseBeforeError(startLine + 1); std::cerr << "[의미 오류] "   << e.what() << "\n"; hasError = true; }
        catch (const RuntimeError& e)      { std::cerr << "[런타임 오류] " << e.what() << "\n"; hasError = true; }
        catch (const std::runtime_error& e){ std::cerr << "[오류] "        << e.what() << "\n"; hasError = true; }
    };

    // 청크 경계(빈 줄)에 브레이크포인트가 설정된 경우 정지하는 헬퍼
    auto checkGapBreakpoints = [&](int nextChunkStart) {
        for (int bp : m_breakpoints) {
            if (bp > m_lastStmtLine && bp <= nextChunkStart) {  // <= : 청크 시작 줄 포함
                // 실제 소스 줄 텍스트 표시 (빈 줄이면 "(빈 줄)")
                std::string srcLine;
                if (bp > 0 && bp <= (int)m_sourceLines.size()) {
                    srcLine = m_sourceLines[bp - 1];
                    srcLine.erase(0, srcLine.find_first_not_of(" \t"));
                }
                if (srcLine.empty()) srcLine = "(빈 줄)";
                std::cout << "[DEBUG] " << bp << "번째 줄에서 정지 (breakpoint) -> " << srcLine << "\n";
                printWatches(*interp);
                commandLoop();
                m_lastStmtLine = bp;  // 재발동 방지
                break;
            }
        }
    };

    try {
        std::ostringstream current;
        int chunkStartLine = 0;
        for (int i = 0; i <= (int)m_sourceLines.size(); i++) {
            if (hasError) break;
            bool isBlank = (i == (int)m_sourceLines.size()) ||
                           m_sourceLines[i].find_first_not_of(" \t\r\n") == std::string::npos;
            if (isBlank) {
                if (!current.str().empty()) {
                    runChunk(chunkStartLine, current.str());
                    current.str(""); current.clear();
                }
                chunkStartLine = i + 1;
            } else {
                // 새 청크 시작 직전: 갭에 걸린 브레이크포인트 검사
                if (current.str().empty()) checkGapBreakpoints(i + 1);
                current << m_sourceLines[i] << '\n';
            }
        }
    }
    catch (const DebugSessionExit&) { return; }

    std::cout << "[DEBUG] 실행 완료\n";
}

void Debugger::onBeforeStmt(Stmt& stmt, Interpreter& interp, std::istream& cmdIn) {
    int  line         = stmt.getLine();
    if (line == 0) return;                                    // BlockStmt 등 건너뜀

    bool atBreakpoint = false;
    for (int bp : m_breakpoints) {
        if (bp > m_lastStmtLine && bp <= line) { atBreakpoint = true; break; }
    }
    m_lastStmtLine = line;

    if (!atBreakpoint) {
        if (!m_stepMode) return;
        if (interp.executeDepth() > m_nextDepth) return;
    }

    std::string srcLine;
    if (line > 0 && line <= (int)m_sourceLines.size()) {
        srcLine = m_sourceLines[line - 1];
        srcLine.erase(0, srcLine.find_first_not_of(" \t"));
    }

    if (atBreakpoint)
        std::cout << "[DEBUG] " << line << "번째 줄에서 정지 (breakpoint) -> " << srcLine << "\n";
    else
        std::cout << "[DEBUG] " << line << "번째 줄에서 정지 -> " << srcLine << "\n";

    printWatches(interp);

    std::string input;
    while (true) {
        std::cout << "> ";
        std::cout.flush();
        if (!std::getline(cmdIn, input)) return;

        if (input == "step") {
            m_stepMode  = true;
            m_nextDepth = INT_MAX;
            break;
        }
        if (input == "next") {
            m_stepMode  = true;
            m_nextDepth = interp.executeDepth();
            break;
        }
        if (input == "continue") {
            m_stepMode  = false;
            m_nextDepth = 0;
            break;
        }
        if (input == "exit" || input == "quit") {
            std::cout << "[DEBUG] 디버그 세션을 종료합니다.\n";
            throw DebugSessionExit{};
        }
        processCommand(input, interp);
    }
}

void Debugger::processCommand(const std::string& input, Interpreter& interp) {
    if (input.empty()) return;
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
        std::cout << "  exit / quit     디버그 세션 종료\n";
    }
}

void Debugger::cmdBreak(int line) {
    if (!m_breakpoints.insert(line).second) {
        std::cout << "[DEBUG] " << line << "번째 줄에 이미 breakpoint가 존재합니다.\n";
        return;
    }
    std::cout << "[DEBUG] " << line << "번째 줄에 breakpoint 설정\n";
}

void Debugger::cmdRemove(int line) {
    if (m_breakpoints.erase(line) == 0) {
        std::cout << "[DEBUG] " << line << "번째 줄에 설정된 breakpoint가 없습니다.\n";
        return;
    }
    std::cout << "[DEBUG] " << line << "번째 줄 breakpoint 제거\n";
}

void Debugger::cmdBreakpoints() {
    if (m_breakpoints.empty()) {
        std::cout << "[DEBUG] " << "설정된 breakpoint가 없습니다.\n";
        return;
    }
    std::cout << "[Breakpoints]\n";
    for (int bp : m_breakpoints) std::cout << "   Line " << bp << "\n";
}

void Debugger::cmdWatch(const std::string& var) {
    m_watches.insert(var);
    std::cout << "[WATCH] '" << var << "' 감시 등록\n";
}

void Debugger::cmdUnwatch(const std::string& var) {
    m_watches.erase(var);
    std::cout << "[WATCH] '" << var << "' 감시 해제\n";
}

void Debugger::cmdWatched(Interpreter& interp) {
    if (m_watches.empty()) {
        std::cout << "[WATCH] 감시 중인 변수가 없습니다.\n";
        return;
    }

    printWatches(interp);
}

void Debugger::cmdInspect(Interpreter& interp) {
    std::cout << "----- 현재 스코프 변수 -----\n";

    std::vector<const Environment*> chain;
    const Environment* cur = interp.currentEnv().get();
    while (cur) {
        chain.push_back(cur);
        cur = cur->enclosing().get();
    }

    for (int i = 0; i < (int)chain.size(); i++) {
        bool        isGlobal = (i == (int)chain.size() - 1);
        std::string label    = isGlobal ? "[전역]" : "[로컬]";
        for (const auto& [k, v] : chain[i]->values()) {
            if (k == "Array") continue;
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
