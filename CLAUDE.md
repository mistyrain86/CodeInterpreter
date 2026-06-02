# TradingSystem — C++ 코드 컨벤션

이 프로젝트의 모든 C++ 코드는 아래 컨벤션을 반드시 준수한다.

---

## 1. 네이밍

| 대상 | 스타일 | 예시 |
|------|--------|------|
| 클래스/구조체 | PascalCase | `PlayerController` |
| 함수/메서드 | camelCase | `getPlayerName()` |
| 변수 | camelCase | `playerScore` |
| 멤버 변수 | camelCase + `m_` 접두사 | `m_playerScore` |
| 상수 | SCREAMING_SNAKE_CASE | `MAX_PLAYER_COUNT` |
| 네임스페이스 | snake_case 또는 소문자 | `game_utils` |
| 파일명 | PascalCase 또는 snake_case | `PlayerController.cpp` |

---

## 2. 들여쓰기 & 중괄호

- 들여쓰기: **스페이스 4칸** (탭 사용 금지)
- 줄 길이: **100자 이하**
- 중괄호: 같은 줄에 열고, 닫는 중괄호는 단독 줄

```cpp
class Player {
public:
    void update(float deltaTime) {
        if (m_isAlive) {
            m_position += m_velocity * deltaTime;
        }
    }

private:
    float m_health;
    bool  m_isAlive;
};
```

---

## 3. 헤더 파일

- 헤더 가드: `#pragma once` 사용
- include 순서: 표준 라이브러리 → 외부 라이브러리 → 로컬 헤더

```cpp
#pragma once

#include <string>    // 표준 라이브러리
#include "Player.h"  // 로컬 헤더
```

---

## 4. 메모리 관리

- 원시 포인터(`new`/`delete`) 직접 사용 **금지**
- 스마트 포인터를 사용한다

```cpp
std::unique_ptr<Player> m_player;
std::shared_ptr<Asset>  m_asset;
```

---

## 5. 접근 제한자 순서

```cpp
class MyClass {
public:    // 1. 공개 인터페이스
protected: // 2. 상속용
private:   // 3. 내부 구현
};
```

---

## 6. 기타 핵심 규칙

- 헤더 파일에서 `using namespace std;` **절대 금지**
- `const`를 적극 활용한다
- 소스 파일(`.cpp`) 내에서는 `using namespace`를 허용하나 권장하지 않는다

---

## 7. 코드 작성 지침 (Claude에게)

코드를 생성하거나 수정할 때 위 컨벤션을 **항상** 적용한다:

- 멤버 변수는 반드시 `m_` 접두사를 붙인다
- 스마트 포인터를 기본으로 사용한다
- `#pragma once`로 헤더 가드를 작성한다
- 클래스 접근 제한자는 `public → protected → private` 순서로 작성한다
- 컨벤션에 어긋나는 기존 코드를 발견하면 수정 시 함께 교정한다

---

## 8. 커밋 메시지 작성 지침 (Claude에게)

작업 완료 후 커밋 메시지를 추천하거나 작성할 때, 아래의 팀 맞춤 커밋 컨벤션을 엄격하게 준수한다.

### 포맷 (Format)
[type] 제목

본문 (선택 사항)

꼬리말 (선택 사항)

### 제약 조건 (Constraints)
1. **타입 제한**: 오직 아래의 3가지 대괄호(`[]`) 타입만 사용해야 한다.
   - `[feature]`: 새로운 기능 추가
   - `[fix]`: 버그 및 오류 수정
   - `[refactoring]`: 기능 변경 없는 코드 구조 개선, 리팩토링
2. **제목 (Subject Line)**:
   - 50자 이내로 작성한다.
   - 끝에 마침표(`.`)를 절대 찍지 않는다.
   - 한국어 명사형 어투로 끝맺는다. (예: "~기능 추가", "~버그 수정")
3. **본문 (Body)**:
   - 줄 바꿈은 72자 기준을 준수한다.
   - '어떻게' 구현했는지보다 '무엇을', '왜' 변경했는지 설명한다.
   - 항목이 많을 경우 가독성을 위해 글머리 기호(`-`)를 사용한다.

### 작성 예시 (Example)
```commit
[feature] 주문 체결 엔진 고도화

대용량 트래픽 상황에서 주문 처리 속도를 높이기 위해 
기존 선형 탐색 기반 로직을 해시 맵 기반 체결 알고리즘으로 개선함.

- 매칭 속도 평균 40% 향상
- 관련 메모리 릭 이슈 방지를 위해 스마트 포인터 전면 도입
```