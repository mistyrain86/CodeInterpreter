# CodeFab — Custom Language Interpreter

C++20 기반의 커스텀 인터프리터 프로젝트입니다.
Lexer → Parser → Optimizer → Checker → Interpreter 5단계 파이프라인으로 동작합니다.

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

## 실행 방법

실행 파일에 전달하는 **인자(args)** 에 따라 동작 모드가 결정됩니다.

```
CodeInterpreter.exe                    → REPL 모드 (인자 없음)
CodeInterpreter.exe run <파일경로>     → 파일 실행 모드
CodeInterpreter.exe debug <파일경로>  → 디버그 모드
```

> `run` · `debug` 는 **CLI 인자**이며 언어 키워드가 아닙니다.

### 언어 키워드

아래 단어들은 예약어로, 변수명으로 사용할 수 없습니다.

```
var  print  if  else  for  true  false  func  return
```

**키워드에 해당하지 않는 모든 식별자는 변수명으로 인식됩니다.**

---

### 1. REPL 모드

인자 없이 실행하면 대화형 프롬프트가 시작됩니다.

```
CodeInterpreter.exe
```

```
CodeFab Interpreter (REPL 모드)
종료: exit 또는 quit
> var x = 10;
> print x + 5;
15
> exit
```

- 프롬프트 `> ` 가 나타나면 CodeFab 코드를 한 줄 입력하고 Enter를 누르면 **즉시 실행**됩니다.
- **전역 변수와 함수는 세션이 끝날 때까지 유지됩니다.**
- `exit` 또는 `quit` 입력 시 종료됩니다.
- 별도 `help` 명령은 없으며, 이 README가 사용 가이드입니다.

**상태 유지 예시:**

```
> func greet(name) { return "안녕, " + name; }
> var msg = greet("CodeFab");
> print msg;
안녕, CodeFab
```

---

### 2. 파일 실행 모드

`.cf` 파일을 작성한 후 `run` 인자와 함께 실행합니다.

```
CodeInterpreter.exe run scripts/hello.cf
```

**`hello.cf` 예시:**

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

func factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
print factorial(5);
```

```
CodeFab Interpreter (FILE 모드)
[FILE] 소스코드 로딩: scripts/hello.cf
8
a가 더 큽니다
0
1
2
120
```

---

### 3. 디버그 모드

`debug` 인자와 함께 실행하면 문장(Stmt) 단위로 실행을 제어할 수 있습니다.

```
CodeInterpreter.exe debug scripts/debug_test.cf
```

```
CodeFab Interpreter (DEBUG 모드)
종료: exit 또는 quit
[DEBUG] 소스코드 로딩: scripts/debug_test.cf
[DEBUG] 12번째 줄에서 정지 -> var a = 3;
>
```

정지 시 `> ` 프롬프트에 아래 커맨드를 입력합니다.

| 커맨드 | 설명 |
|--------|------|
| `step` | 다음 문장에서 정지 (블록 내부 진입) |
| `next` | 다음 문장에서 정지 (블록 내부 건너뜀) |
| `continue` | 다음 breakpoint까지 실행 |
| `break <줄>` | 해당 줄에 breakpoint 설정 |
| `remove <줄>` | 해당 줄 breakpoint 해제 |
| `Breakpoints` | 설정된 breakpoint 목록 출력 |
| `watch <변수>` | 변수 감시 등록 (정지마다 자동 출력) |
| `unwatch <변수>` | 변수 감시 해제 |
| `watched` | 감시 중인 변수 목록과 현재 값 출력 |
| `inspect` | 현재 스코프 전체 변수/값/타입 출력 |
| `exit` / `quit` | 디버그 세션 종료 |

> **알 수 없는 커맨드**를 입력하면 전체 커맨드 목록이 출력됩니다.

**사용 예시:**

```
[DEBUG] 12번째 줄에서 정지 -> var a = 3;
> watch a
[WATCH] 'a' 감시 등록
> step
[DEBUG] 13번째 줄에서 정지 -> var b = 7;
[WATCH] a = 3
> inspect
----- 현재 스코프 변수 -----
[전역] a = 3 (Number)
> continue
[DEBUG] 실행 완료
```

---

## 언어 명세

### 데이터 타입

| 타입 | 예시 | 비고 |
|------|------|------|
| 숫자 | `3`, `3.14`, `-1` | 정수는 `.0` 없이 출력 (`5.0` → `5`) |
| 문자열 | `"hello"` | 큰따옴표로 감쌈 |
| 불리언 | `true` / `false` | 소문자 키워드 |
| null | 초기화 없는 var, return 없는 함수 반환값 | `null`로 출력 |

### 연산자 및 우선순위

```
단항(-,!)  >  곱셈/나눗셈/나머지(*,/,%)  >  덧셈/뺄셈(+,-)  >  비교(<,<=,>,>=)  >  동등(==,!=)
```

| 분류 | 연산자 | 예시 | 결과 |
|------|--------|------|------|
| 산술 | `+` `-` `*` `/` `%` | `1 + 2 * 3` | `7` |
| 단항 | `-` `!` | `!true` | `false` |
| 비교 | `<` `<=` `>` `>=` | `3 > 5` | `false` |
| 동등 | `==` `!=` | `1 == 1` | `true` |
| 문자열 연결 | `+` | `"Hi" + "!"` | `Hi!` |

### 변수

```
var x = 10;     // 선언 및 초기화
var y;          // null로 초기화
x = x + 1;     // 재할당
```

### 함수

```
// 함수 선언
func add(a, b) {
    return a + b;
}
print add(3, 7);    // 10

// return 없는 함수는 null 반환
func noop() { var x = 1; }
print noop();       // null

// 재귀
func factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
print factorial(5); // 120

// 클로저 (외부 변수 캡처)
var x = 10;
func getX() { return x; }
print getX();       // 10
```

### 배열

```
var arr = Array(5);     // 크기 5짜리 배열 (초기값 null)
arr[0] = 10;
arr[1] = 20;
print arr[0];           // 10
print arr[1];           // 20

// for 루프와 함께 사용
for (var i = 0; i < 5; i = i + 1) {
    arr[i] = i * 2;
}
print arr[3];           // 6
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
print x;               // global

var count = 0;
{
    count = count + 1; // 바깥 변수 수정 가능
}
print count;           // 1
```

### 주석

```
// 한 줄 주석 (# 은 지원하지 않음)
```

---

## 에러 처리

에러 발생 시 stderr에 메시지를 출력합니다.
파일 실행 모드에서는 에러 종류에 따라 비정상 종료됩니다.

| 단계 | 에러 종류 | 출력 형식 |
|------|---------|---------|
| Lexer | 어휘 오류 | `[오류] [라인 N] 어휘 오류: ...` |
| Parser | 구문 오류 | `[구문 오류] [라인 N] 구문 오류: ...` |
| Checker | 의미 오류 | `[의미 오류] [라인 N] 의미 오류: ...` |
| Interpreter | 런타임 오류 | `[런타임 오류] [라인 N] 런타임 오류: ...` |

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

// 런타임 오류: 0으로 나누기
print 10 / 0;
→ [런타임 오류] 0으로 나눌 수 없습니다.

// 런타임 오류: 배열 범위 초과
var arr = Array(3); print arr[5];
→ [런타임 오류] 인덱스 범위를 벗어났습니다. (5)
```

> **참고:** 전역 스코프에서의 중복 `var` 선언은 에러가 아닌 덮어쓰기로 처리됩니다.
