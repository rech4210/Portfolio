# Boss System Design & Debug Log (2025-06-07)

## 📌 1. BTService → Controller → Component 설계 흐름
- 보스 AI 흐름: Service → Controller → StateComponent / SkillComponent 등 분기
- Controller 과부하 문제 인식 → 책임 분산 구조 고민
- 데이터 전달 vs. 로직 실행 위치에 대한 설계 기준 정립

## 📌 2. Enum 관리 전략
- 상태, 페이즈, 액션 등 명확히 Enum 분리
- 공통 Enum은 Shared 또는 Core 모듈로 분리 관리
- 예시: EBossState, EBossPhase, EBossAction

---

## 📌 3. GameplayEffect(GE) 보스 미적용 이슈
- GE가 GGwaAttributeSet 전용 → 보스는 EnemyAttributeSet이라 적용 실패
- 해결: UBaseAttributeSet 공통 정의 → 둘 다 상속
- GE는 UBaseAttributeSet 기반으로 작성

---

## 📌 4. PhaseIndex 증가 Clamp 오류
- 후위 증가 연산자(PhaseIndex++) 사용 시 Clamp에 증가 전 값이 들어가 문제 발생
- 전위 증가(++PhaseIndex) 또는 두 줄로 분리해 안전하게 처리

---

## 📌 5. 적/아군 구분 구조 설계
- GameplayTag 기반: "Faction.Player", "Faction.Enemy"
- UGwaFactionLibrary::IsEnemy(Self, Target)로 통일된 판별 처리
- GameModeType(PvE, PvP) 기반 분기 처리

---

## 📌 6. BeginPlay 중복 호출 → 보스 2마리 생성
- 레벨에 배치된 Actor는 서버/클라 양쪽에서 BeginPlay 실행됨
- 해결: HasAuthority() 체크 추가하여 서버에서만 실행되도록 제한

---

## 📌 7. 매직 넘버 처리 개선
- #define 대신 static constexpr 또는 const 사용
- 클래스 내 상수 선언으로 책임 명확화

---

## 📌 8. Blackboard Key 접근 개선
- SetValueAsX("Key") → TEXT("Key") 또는 FName("Key") 사용
- 타입 안전성 및 리팩토링 호환성 향상

---

## 📌 9. AttributeSet 델리게이트 바인딩 타이밍 문제
- BeginPlay 시점에 ASC가 Init되지 않아 Delegate 바인딩 실패 가능
- 해결: InitASC 이후에 컴포넌트 바인딩 처리하도록 수정

---

## ✅ 핵심 정리

| 항목               | 처리 방식                             |
|--------------------|----------------------------------------|
| GE 범용 적용        | 공통 AttributeSet 도입 (UBaseAttributeSet) |
| Phase 증가 처리     | 전위 증가 or 분리 처리                 |
| BeginPlay 중복 방지 | HasAuthority() 확인 필수              |
| 상태 관리          | Enum 분리 및 Shared 관리               |
| 적/아군 구분        | Tag + 유틸 함수 통합                   |
| 매직 넘버 제거      | constexpr 사용                         |
| Blackboard 안정성   | TEXT("Key") 사용                       |
| ASC 델리게이트 관리 | Init 이후 바인딩 수행                  |

---

📅 **작성일**: 2025-06-07  
👤 **작성자**: 제자님  
💡 **비고**: 중간 설계 기록이자 구조 정리 문서. 리팩토링 및 시스템 확장 기준으로 삼을 것.
