# MyGame 프로토타입 개발 가이드라인

## 1. 핵심 원칙: 빠르고, 일관성 있게

이 문서는 MyGame 프로토타입의 빠른 기능 개발을 위한 가이드입니다. 포트폴리오 제작을 목표로 하므로, 복잡한 최적화보다는 **동작하는 기능을 빠르게 추가하는 것**에 집중합니다.

1.  **도메인 모델 우선 (Model-First)**: 항상 C++ 코드(`Component`, `DTO` 등)를 먼저 설계하고, 데이터베이스 스키마는 이를 따라가도록 합니다.
2.  **`Schema_Temp.md`는 계약서**: 데이터베이스 구조의 모든 변경은 `Schema_Temp.md`에 먼저 기록하고, 그 DDL을 실제 DB에 적용합니다.
3.  **패턴 재사용**: 새로운 기능을 만들 때 기존 시스템(`Shop`, `Inventory` 등)의 구조(`Subsystem` -> `DomainService` -> `Component` -> `Repository`)를 그대로 복제하여 사용합니다. 바퀴를 재발명하지 않습니다.
4.  **"Happy Path"에 집중**: 일단 주 기능이 성공적으로 동작하는 경로에 집중합니다. 모든 예외 상황 처리는 나중으로 미룹니다.

---

## 2. 새 기능 추가를 위한 단계별 워크플로우

새로운 **"기능(Feature)"**을 추가하는 일반적인 과정을 설명합니다. (예: 길드, 펫, 업적 등)
이 워크플로우를 따르면 기존 아키텍처와의 일관성을 유지하며 빠르게 기능을 구현할 수 있습니다.

### Step 1: 도메인 정의 (C++ 코드 작업)

가장 먼저, 게임 로직의 핵심이 되는 C++ 클래스들을 정의합니다.

-   **DTO 정의**: `DatabaseModule/Public/DatabaseManager.h`에 새 기능의 데이터 구조체 `FNewFeatureDTO`를 추가합니다.
    ```cpp
    USTRUCT()
    struct FNewFeatureDTO
    {
        GENERATED_BODY()
        // FeatureID, State, Progress 등 기능에 필요한 데이터...
    };
    ```
-   **애그리거트 루트(Aggregate Root) 생성**: `GGwaPlayerState`에 추가될 `UNewFeatureComponent`를 생성합니다. 이 컴포넌트는 플레이어의 새 기능 관련 상태(`TArray<FNewFeatureDTO>`)를 소유하고 관리합니다.
-   **도메인 서비스 생성 (선택 사항)**: 기능의 수락 조건, 보상 계산 등 여러 애그리거트에 걸친 복잡한 비즈니스 로직이 필요하다면 `UNewFeatureDomainService`를 만듭니다. 단순 CRUD만 있다면 생략 가능합니다.

### Step 2: 데이터베이스 스키마 업데이트

C++ 모델이 정의되었으면, 이를 저장할 테이블을 설계합니다.

-   **`Schema_Temp.md` 수정**: 문서의 DDL 섹션에 `FNewFeatureDTO`를 기반으로 한 `CREATE TABLE new_features (...)` 쿼리를 추가합니다.
-   **DB에 적용**: 추가한 `CREATE TABLE` 쿼리를 실제 개발 DB에서 실행하여 테이블을 생성합니다.

### Step 3: 리포지토리(Repository) 계층 구현

`DatabaseManager`가 새 테이블과 통신할 수 있도록 함수를 추가합니다.

-   **함수 선언**: `DatabaseManager.h`에 `LoadNewFeaturesForPlayer`, `SaveNewFeaturesForPlayer`와 같은 데이터 접근 함수를 선언합니다.
-   **함수 구현**: `DatabaseManager.cpp`에서 선언한 함수들의 본체를 구현합니다. `Schema_Temp.md`에 정의된 테이블과 컬럼 이름에 맞춰 정확한 SQL 쿼리를 작성합니다.

### Step 4: 애플리케이션(Application) 계층 구현

플레이어의 입력이나 게임 월드 이벤트로부터 실제 로직이 시작되는 부분을 만듭니다.

-   **서브시스템(Subsystem) 생성**: `UNewFeatureSubsystem`을 생성합니다. 이 서브시스템은 외부(UI, 콘솔 명령 등)로부터의 요청을 받는 창구 역할을 합니다.
-   **유스케이스(Use Case) 메서드 추가**: `DoSomethingUseCase(APlayerState* Player, ...)`와 같은 메서드를 서브시스템에 추가합니다.
-   **로직 오케스트레이션**: 서브시스템의 메서드는 `NewFeatureDomainService`나 `NewFeatureComponent`의 함수를 호출하여 실제 작업을 위임하고, 최종적으로 `DatabaseManager`를 통해 결과를 저장합니다.

### Step 5: 간단한 테스트

기능이 동작하는지 빠르게 확인합니다.

-   **콘솔 명령 또는 단축키 활용**: 언리얼 에디터에서 특정 키를 누르면 `NewFeatureSubsystem`의 유스케이스 메서드가 호출되도록 임시 코드를 작성합니다.
-   **로그 확인**: 기능 실행 시점에서 `UE_LOG`를 사용하여 `UNewFeatureComponent`의 상태 변화나 DB 저장 성공 여부를 출력합니다.
-   **결과**: "기능 실행 -> 컴포넌트 상태 변경 -> DB 저장"의 흐름이 로그를 통해 확인되면, 기본 기능 구현이 완료된 것입니다.

---

## 3. 빠른 개발을 위한 "하지 말아야 할 것" (지금 당장은)

-   **과도한 최적화 금지**: `Schema_Temp.md`의 고급 최적화(파티셔닝, 프로시저 등)는 지금 적용하지 않습니다. 기능 구현이 우선입니다.
-   **완벽한 테스트 코드 금지**: 모든 함수에 대한 단위 테스트(Unit Test) 대신, 핵심 기능의 흐름을 검증하는 통합 테스트(Integration Test)에 집중합니다.
-   **화려한 UI 금지**: 기능 테스트는 디버그용 UI나 콘솔 명령으로 충분합니다. 최종 UI는 프로토타입 완성 후에 작업합니다.

이 가이드라인을 따르면 DDD의 장점을 유지하면서도 프로토타입 개발 속도를 크게 향상시킬 수 있습니다.
