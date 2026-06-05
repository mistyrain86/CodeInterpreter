# CodeFab — Custom Language Interpreter

C++20 기반의 커스텀 인터프리터 프로젝트입니다.
Lexer → Parser → Checker → Interpreter 4단계 파이프라인으로 동작합니다.

---
## 👥 팀 소개

**팀명:** Don't Touch

| 역할 | 이름 |
|:----:|------|
| 팀장 | 최연우 |
| 팀원 | 이상현 · 윤동현 · 이혜림 |

---

## 📋 코드 리뷰 그라운드룰

### 1. PR 리뷰 세션 진행
PR을 올린 후 팀원이 함께 리뷰할 수 있는 시간을 가진다.

> AI를 활용해 작성된 코드는 특히 **작성 의도를 직접 설명**할 수 있어야 한다.

### 2. 사전 협의된 내용은 Direct Merge 허용
단, **PR Comment에 해당 협의 히스토리**를 반드시 작성한다.

### 3. AI 활용 코드의 Commit 메시지 규칙
AI를 활용해 작성된 코드는 Commit 메시지에 **핵심 프롬프트 및 구현 의도**를 포함해야 한다.

### 4. AI 코드 리뷰 Agent 운영
팀 전용 AI 리뷰 에이전트를 양성한다.
- 리뷰 규칙은 지속적으로 업데이트
- 신규 Commit 발생 시 **자동으로 리뷰** 수행

### 5. AI 코드 구현 프로세스
AI를 통해 코드를 구현할 때 아래 순서를 따른다.

PLAN 수립  →  PLAN 검증  →  기능별 순차 구현

> 리뷰 시에도 **Plan에 대한 검증 여부**를 확인한다.

---

---

## 빌드

Visual Studio 2022에서 `CodeInterpreter.slnx`를 열고 **Release | x64** 로 빌드합니다.

> **Debug 빌드**는 GoogleMock 테스트 러너로 진입하므로, 인터프리터 실행은 **Release** 빌드를 사용하세요.

빌드 후 실행 파일 위치: `CodeInterpreter\Release\CodeInterpreter.exe`

---

## 실행 방법

### 1. 인터랙티브 모드 (REPL)

터미널에서 실행 파일을 직접 실행하면 대화형 프롬프트가 나타납니다.

```
CodeInterpreter.exe
```

```
CodeFab Interpreter
여러 줄 입력 후 빈 줄을 입력하면 실행됩니다. 종료: Ctrl+Z (Windows) / Ctrl+D (Linux)
>>> var x = 10;
... print x + 5;
...
15
>>>
```

| 프롬프트 | 의미 |
|---------|------|
| `>>> ` | 새 입력 대기 |
| `... ` | 이전 줄에서 이어받는 중 |

- **빈 줄** 입력 → 누적된 코드 실행
- **Ctrl+Z** (Windows) / **Ctrl+D** (Linux) → 종료

여러 줄 프로그램 예시:

```
>>> var x = "global";
... {
...   var x = "inner";
...   print x;
... }
... print x;
...
inner
global
>>>
```

---

### 2. 파이프 모드

소스 코드를 파이프로 전달하면 한 번에 실행합니다.

```bat
echo "print 1 + 2 * 3;" | CodeInterpreter.exe
:: 출력: 7
```

```bat
echo "var x = 10; var y = 20; print x + y;" | CodeInterpreter.exe
:: 출력: 30
```

---

### 3. 파일 입력 모드

소스 코드를 파일에 저장한 후 리디렉션으로 실행합니다.

**`program.cf` 예시:**
```
var a = 5;
var b = 3;
print a + b;
if (a > b) {
    print "a가 더 큽니다";
}
for (var i = 0; i < 3; i = i + 1) {
    print i;
}
```

```bat
CodeInterpreter.exe < program.cf
:: 출력:
:: 8
:: a가 더 큽니다
:: 0
:: 1
:: 2
```

---

## 언어 명세

### 데이터 타입

| 타입 | 예시 | 비고 |
|------|------|------|
| 숫자 | `3`, `3.14`, `-1` | 정수는 `.0` 없이 출력 (`5.0` → `5`) |
| 문자열 | `"hello"` | 큰따옴표로 감쌈 |
| 불리언 | `true` / `false` | 소문자 키워드 |
| nil | 초기화 없는 var | `nil`로 출력 |

### 연산자 및 우선순위

```
단항(-,!)  >  곱셈/나눗셈(*,/)  >  덧셈/뺄셈(+,-)  >  비교(<,<=,>,>=)  >  동등(==,!=)
```

| 분류 | 연산자 | 예시 | 결과 |
|------|--------|------|------|
| 산술 | `+` `-` `*` `/` | `1 + 2 * 3` | `7` |
| 단항 | `-` `!` | `!true` | `false` |
| 비교 | `<` `<=` `>` `>=` | `3 > 5` | `false` |
| 동등 | `==` `!=` | `1 == 1` | `true` |
| 문자열 연결 | `+` | `"Hi" + "!"` | `Hi!` |

### 변수

```
var x = 10;     // 선언 및 초기화
var y;          // nil로 초기화
x = x + 1;     // 재할당
```

### 제어 흐름

```
// if / else
if (조건) print "yes";
if (조건) { ... } else { ... }

// for
for (var i = 0; i < 3; i = i + 1) {
    print i;
}
```

### 블록 스코프

```
var x = "global";
{
    var x = "inner";   // 별개의 변수 (shadowing)
    print x;           // inner
}
print x;               // global  ← 블록 밖은 영향 없음

var count = 0;
{
    count = count + 1; // 바깥 변수 수정은 가능
}
print count;           // 1
```

### 주석

```
// 한 줄 주석 (# 은 지원하지 않음)
```

---

## 에러 처리

에러 발생 시 stderr에 메시지를 출력하고, 파이프/파일 모드에서는 아래 종료 코드를 반환합니다.

| 단계 | 에러 종류 | exit 코드 | 출력 예시 |
|------|---------|---------|---------|
| Lexer | 어휘 오류 | 4 | `[오류] [라인 1] 어휘 오류: 인식할 수 없는 문자 '@'` |
| Parser | 구문 오류 | 1 | `[구문 오류] [라인 1] 구문 오류: ...` |
| Checker | 의미 오류 | 2 | `[의미 오류] [라인 1] 의미 오류: ...` |
| Interpreter | 런타임 오류 | 3 | `[런타임 오류] [라인 1] 런타임 오류: ...` |

### 주요 에러 케이스

```
// 구문 오류: 세미콜론 누락
print 1 + 2
→ [구문 오류] 값 출력 뒤에 ';'가 필요합니다.

// 의미 오류: 로컬 스코프 중복 선언
{ var a = 1; var a = 2; }
→ [의미 오류] 이미 이 스코프에 같은 이름의 변수가 있습니다. ('a')

// 의미 오류: 자기 참조 초기화
{ var a = a; }
→ [의미 오류] 자신의 초기화식에서 지역변수를 읽을 수 없습니다. ('a')

// 런타임 오류: 미정의 변수
print notDefined;
→ [런타임 오류] 미정의된 변수 'notDefined'.

// 런타임 오류: 타입 불일치
print 1 + "HI";
→ [런타임 오류] 피연산자는 두 숫자 또는 두 문자열이어야 합니다.
```

> **참고:** 전역 스코프에서의 중복 `var` 선언은 에러가 아닌 덮어쓰기로 처리됩니다.

---

## 테스트 실행

**Debug | x64** 빌드 후 실행하면 139개 단위/통합 테스트가 자동으로 수행됩니다.

```
CodeInterpreter\Debug\CodeInterpreter.exe
```

```
[==========] 139 tests from 11 test suites ran.
[  PASSED  ] 139 tests.
```

---

## 프로젝트 구조

```
CodeInterpreter/
├── Token.h / Value.h              # 공통 타입
├── Expr.h / Stmt.h                # AST 노드
├── Lexer.h/cpp                    # 어휘 분석
├── Parser.h/cpp                   # 구문 분석
├── Checker.h/cpp                  # 정적 의미 분석
├── Environment.h/cpp              # 변수 환경 (스코프 체이닝)
├── Interpreter.h/cpp              # 실행 엔진
├── LangFactory.h/cpp              # 파이프라인 조립 (DI)
├── ParseError.h / CheckError.h / RuntimeError.h
├── Mocks.h / TestUtils.h          # 테스트 인프라
├── *Test.cpp                      # 컴포넌트별 테스트
├── TestMain.cpp                   # 테스트 진입점 (Debug)
└── main.cpp                       # 인터프리터 진입점 (Release)
```
