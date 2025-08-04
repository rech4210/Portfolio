
# GAS 멀티플레이 데디서버 기반 게임플레이 구현

## 환경 요약


URL : https://github.com/rech4210/Portfolio

구성원 : 1인

엔진/네트워크: Unreal Engine 5, Gameplay Ability System(GAS), Dedicated Server Multiplay

인프라: Docker 기반 MySQL DB, Node.js 기반 JWT 인증 서버



## 1. 프로젝트 개요


해당 프로젝트는 Unreal Engine 5를 활용하여 구현한 멀티플레이 GAS 구조로, **Gameplay Ability System (GAS)** 를 기반으로 플레이어 능력과 효과를 관리합니다. 전체 코드는 클라이언트와 서버 로직을 분리한 모듈 구조로 작성되었으며, 서버 로직은 로컬 DB 서버와 연결되어 작업을 수행합니다. 목표는 GAS를 활용해 안정적인 데디서버 아키텍처를 구축하고, 클라이언트에 게임플레이 정보를 정확히 **동기화**하는 것입니다.


## 2. 아키텍처 & 모듈 구성
![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/1.아키텍처.png)


| 모듈명                        | 역할 요약                                                       |
| -------------------------- | ----------------------------------------------------------- |
| **MyGame**                 | 게임의 핵심 도메인 (캐릭터, GameMode, GAS 시스템 등) 및 공용 코드 기반 제공                  |
| **ClientModule**           | HUD, 위젯, 입력 등 클라이언트 전용 표현과 인터랙션 처리                          |
| **ServerModule**           | Dedicated Server 런타임 관리 및 전체 기능 통합, 상태 조율                   |
| **Shared Feature Modules** | Inventory, Skill, Equipment, Shop, Auth 등 기능 단위로 분리된 재사용 모듈 |
| **GameSharedModule**       | 인터페이스, 공용 구조체, 계약 등 모듈 간 공통 기반 제공                           |
| **DatabaseModule**         | SQL 실행, DTO 매핑, 커넥션 관리 등 DB 관련 기능 전담                        |



### 아키텍처 모듈 구성의 필요성
프로젝트는 MMORPG를 목표로 하며, 클라이언트와 서버가 독립적으로 발전할 수 있도록 멀티 모듈 아키텍처를 채택했다. MyGame 모듈은 핵심 캐릭터/게임플레이 로직을 담당하고, ServerModule은 서버 전용 초기화와 기능 모듈 통합을 담당한다
클라이언트 전용 기능은 ClientModule, 공용 인터페이스와 구조체는 GameSharedModule로 분리된다

- 데디케이트 서버 환경: 서버 코드는 DB 접근, 인증, 비동기 처리 등 무거운 로직을 포함하므로 클라이언트와 분리해야 한다. 예를 들어 ServerModule이 DB를 초기화하고 기능 모듈을 연결함으로써 서버 전용 동작을 수행한다

- 응집성과 재사용성 향상: SkillModule, InventoryModule, ShopModule, EquipmentModule, AuthModule과 같은 Shared Feature Module을 별도 모듈로 두어 클라이언트와 서버 양쪽에서 재사용한다. 이러한 분리는 기능 추가나 수정 시 다른 영역에 영향을 주지 않는다.

- 확장성: 모듈 간 의존성을 명시하고 인터페이스를 통해 통신하므로, 새로운 기능을 독립적으로 추가할 수 있다. 예를 들어 외부 AI 분석기 모듈을 추가할 때 ServerModule에만 의존하도록 설계하면 된다.

---

## 전투 로그 AI 분석기 도입 – 모듈 분리 필요성 및 클라이언트 연동 시나리오

![scenario](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/Scenario.png)

### 문제 정의

1. 서버 기능 확장 시 클라이언트 코드까지 영향을 주는 구조
   - 전투 로그 수집 및 AI 분석기 도입과 같은 기능 확장 시, 서버-클라이언트 간 결합도가 높아 변경의 영향 범위가 넓었음.

2. 도메인 기능이 여러 위치에 분산되어 재사용과 테스트가 어려움
   - Inventory, Skill, Equipment 등 핵심 기능이 분산되어 있어, 일관된 접근과 기능 캡슐화가 미흡했음.

3. 클라이언트 HUD 연동 시 책임 분리 구조 미흡
   - 서버 데이터 수신과 HUD 반영 로직 간의 경계가 불분명하여, UI 확장 시 의존성 증가 및 유지보수 비용이 발생함.


### 해결 방안

1. 모듈 아키텍처 기반 구조로 책임 분리
  - 서버, 클라이언트, 공용 기능을 각각 독립 모듈로 구성하여 변경의 영향을 최소화함.

2. 공통 기능은 Feature Module로 캡슐화
  - SkillModule, InventoryModule 등 각 기능을 독립적으로 설계하여 분석기에서 구독 방식으로 데이터를 접근하도록 구조화함.

3. IOC 기반 클라이언트 UI 갱신 구조 도입
  - 서버 결과는 PlayerController를 통해 전달되며, 실제 HUD 업데이트는 ClientComponent가 수행하여 SRP 유지.
  - 클라이언트 표현 계층을 명확히 분리하여 UI 확장을 유연하게 처리 가능하도록 함.



```sql
기획자:
"다음 업데이트에서 전투 로그를 수집해 AI 분석기를 붙이고 싶습니다.
게임 밸런스 데이터를 실시간으로 분석하려면 어떤 방식으로 구현해야 할까요?"

프로그래머:
"서버에 AI 분석 모듈을 신규로 추가하면 가능합니다.
전투 로그는 GameSharedModule의 인터페이스를 통해 접근하도록 설계되어 있어, 클라이언트 코드에는 영향이 없습니다.  
DB 접근은 기존의 DatabaseModule이 처리하므로 기존 기능과 충돌도 발생하지 않습니다."

기획자:
"그렇다면 클라이언트 업데이트 없이 서버만 교체하면 되겠네요.
그럼 분석 결과를 유저 HUD에 실시간으로 표시할 수 있을까요?"

프로그래머:
"해당 기능은 클라이언트 업데이트가 필요합니다.
현재 클라이언트는 컨트롤러를 통해 서버 데이터를 수신하며, 표시 관련 처리는 컨트롤러에 부착된 ClientComponent가 담당합니다.
IOC 패턴 기반으로, 컨트롤러는 데이터를 전달하고 컴포넌트가 UI 반영을 위임 수행하는 구조입니다.
분석 결과 역시 이 구조에 따라 연동 가능합니다."

기획자:
"그렇다면 HUD 출력은 다음 스프린트에서 적용하고, 서버 분석 기능만 우선 도입하겠습니다."
```

---

### 도커 설정

개발자마다 다른 MySQL 환경, 수동 스키마/시드 적용, 불확실한 테스트 환경.  
이런 비효율과 불확실성을 제거하기 위해 MySQL을 Docker로 컨테이너화하여 동일한 DB 환경을 즉시 재현하고, 스키마 및 데이터 초기화를 자동화하며, 개발·CI 전반에서 일관된 검증 흐름을 확보하고자 도입했다.

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/2.도커설정.png)

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/3.도커설정.png)



-----


# 3. 주요 기능 구현

## DDD 로그인 시스템

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/4.로그인DDD.png)

### 로그인 DDD 구성

| 계층                             | 클래스 / 구조                | 주요 역할           | 설명                                                                                                                                                                                                  |
| ------------------------------ | ----------------------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Subsystem (App Layer)**      | `UAuthSubsystem`        | 요청 진입 및 오케스트레이션 | 서버 권한 검증과 파라미터 확인 후 외부 인증 서버 또는 도메인 서비스로 라우팅, 레포지토리/도메인 서비스 초기화 및 의존성 주입을 담당한다. |
| **Domain Service**             | `UAuthDomainService`    | 도메인 로직 수행       | 회원 가입과 로그인에 대한 유효성 검증, 계정 상태 체크, 비밀번호 검증, 감사 로그 기록, 토큰 생성 등 비즈니스 규칙을 수행.                                                                                                                                   |
| **Component (Aggregate Root)** | `UAuthComponent`        | 상태 유지 및 검증      | 사용자 계정 상태·행위를 캡슐화하며 로그인 가능 여부, 계정 잠금/삭제, 비밀번호 규칙 검사, 감사 로그 추가 등을 담당.                                                                                                                                       |
| **Repository**                 | `UAuthRepository`       | 데이터 접근          | `DatabaseManager`를 통해 사용자 정보를 비동기로 조회/저장한다.                                                                                             |
| **RPC Interface**              | `IAuthRPCInterface`     | 네트워크 추상화        | 클라이언트와 서버 사이의 인증 통신을 추상화하며, 서버 등록·로그인·토큰 기반 접속 등 메서드를 정의한다.                                                                                                                                                |
| **Client Service**             | `UAuthService`          | UI/입력 처리        | 클라이언트에서 사용자 입력을 받아 `IAuthRPCInterface`를 통해 서버에 전달하고 인증 상태를 관리한다.                                                                                                                                           |
| **Client Component**           | `UClientAuthComponent`  | 클라이언트 인증 처리     | 로컬 플레이어 컨트롤러에서 초기화되며 `UAuthService`를 생성하고, 회원가입·로그인 요청을 서비스에 위임한다. 서버 응답을 받아 블루프린트 이벤트로 UI에 전달하며 UI 서브시스템을 반환한다.                                                                                           |
| **Player Controller**          | `AGGwaPlayerController` | 브리지·토큰 전달       | 서버 측에서는 RPC를 통해 `AuthSubsystem`의 등록·로그인 요청을 호출하고, 클라이언트 측에서는 `ClientAuthComponent` 및 UI 컴포넌트를 생성·등록한다.                                     |

---

### 1. 도입 계기(문제 정의)  
MMORPG 환경에서 인증과 로그인 로직이 게임플레이 로직과 섞이면서 책임이 불분명해지고, 보안 정책과 상태 관리가 흩어져 일관된 대응이 어렵다. 또한 저장소 구현체에 강하게 결합되어 있어 인프라 변화에 유연하지 못하며, 클라이언트 UI 반영과 인증 결과 사이의 책임 경계가 모호하다.

- **책임 혼재**: 게임 서버가 인증과 핵심 게임플레이(전투/월드 시뮬레이션)를 함께 처리해 단일 책임 원칙(SRP)이 깨지고, 변경 시 파급 범위가 넓어짐  
- **보안 대응의 분산**: 비밀번호 정책, 로그인 실패 제어, 계정 잠금, 이상 징후(브루트포스/도용) 대응이 일관된 중심 구조 없이 흩어져 있어 효과적 방어가 어려움  
- **도메인 응집성 부족**: 인증 관련 기능(정책, 실패 추적, 상태 관리 등)이 분산되어 재사용성과 테스트가 떨어짐  
- **저장소 결합도 높음**: 구체적인 저장소 구현체에 의존하면 향후 Redis/NoSQL 등의 대체가 어렵고 유연성 저하  
- **클라이언트/서버 표현 책임 모호**: 인증 결과를 클라이언트 UI로 반영하는 흐름에서 책임이 뒤섞여 UI 확장 시 의존성 증가

---

### 2. 해결 방법

#### 2.1 DDD 기반 모듈 계층화  
`AuthSubsystem (애플리케이션 계층)` → `AuthDomainService (도메인 서비스)` → `AuthComponent (Aggregate Root)` → `AuthRepository (리포지토리)` 구조로 설계하여 각 계층이 명확한 책임을 갖도록 함. 변경이나 테스트 시 영향 범위를 국소화해 유지보수성과 일관성을 확보한다.

#### 2.2 DIP + SRP/IoC 위임

- **DIP**: 저장소 접근을 추상 인터페이스로 정의하고, 실제 구현체(MySQL, Redis, NoSQL 등)를 주입하여 교체와 테스트, 환경별 구성 변경을 유연하게 한다.  
- **SRP + IoC 위임**:  
  - `PlayerController`는 `ClientComponent`를 소유만 하며, 인증 결과나 입력이 들어오면 실제 **인증 처리와 UI 업데이트 책임은 `ClientComponent`가 갖는다.**  
  - `PlayerController`는 이벤트 전달자 역할만 하여 책임을 최소화한다.  
- **확장/결합도 보완**:  
  필요한 추가 기능이 생기면 `ClientComponent`를 통해 확장 가능한 서브컴포넌트를 제공함으로써 느슨한 결합을 유지하면서 기능 제공이 가능하며 `PlayerController`의 God Object 방지.  
- **효과**: 인프라 교체와 표현 계층 변경이 독립적으로 가능해지고, 각 모듈이 단일 책임을 지며, 확장 시에도 결합도 낮은 구조를 유지한다.

#### 2.3 외부 인증 서버 위임
![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/5.유저DB데이터.png)

- 별도의 **Node.js 기반 인증 서버**가 다음을 담당:
  - 비밀번호 해싱  
  - JWT 토큰 발급/검증  
  - 계정 상태 관리 (잠금/삭제 등)  

#### 2.4 감사 로그 (Audit) & 보안 강화 및 유효성 검증
![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/6.유저로그인회원가입추적로깅.png)

- `AuthComponent`가 사용자명/비밀번호 정책(최소 길이, 특수문자 등), 계정 잠금/복구 상태, 삭제 상태 등을 관리.  
- `AuthDomainService`는 보안 담당, 상태 기반 흐름을 도입해 잠금/복구 판단을 일관되게 처리한다.  

`AuthDomainService`가 다음과 같은 주요 이벤트를 일관되게 기록:  
- 계정 생성 , 로그인 시도 실패/성공  , 계정 잠금 및 복원  
이 로그는 보안 사고 분석, 이상 징후 탐지, 운영 의사 결정(예: 정책 조정) 등에 활용 가능하다.


### 로그인 시스템 변경 케이스
```sql
QA:
"로그인 관련 변경할 때마다 전반에 파급되고, 보안 대응도 흩어져서 2단계 인증이나 정책 변경이 너무 힘들고, 실패 원인 추적도 어렵습니다."

프로그래머:
"기존엔 인증, 정책, 상태, 표현이 뒤섞여 있었고, UI 반영 책임도 컨트롤러에 있어 재사용과 확장이 힘들었어요. 그래서 계층을 나눴습니다:
AuthSubsystem → AuthDomainService → AuthComponent → AuthRepository.
민감한 로직은 외부 JWT 서버로 격리하고, 정책/이상 감지는 도메인에서 중앙 처리하며, UI는 표준 코드만 받아 독립적으로 표현하게 했어요. 저장소는 MySQL(주) + Redis/NoSQL(보조) 구조로 추상화했습니다.
유저의 세션 데이터는 Redis에 저장한다던지 하는 식으로요"

QA:
"그럼 바꾸고 나서 달라진 점은요?"

프로그래머:
- 보안 판단이 일관되게 중앙화되고 감사 로그가 표준화됨
- 기능 확장은 도메인만 건드리면 되고 UI는 실패 코드만 처리, 필요할 경우 Component로 기능 추가
- 정책 변경은 정책 함수만 바뀌어 파급이 사라짐
- 표현과 비즈니스가 분리되어 UI 확장이 비즈니스 로직에 독립적
- 인프라 교체(DB 취사 적용)가 부드러워짐

프로그래머:
"이제 실패 타입만 매핑하면 되고, 새 인증 흐름 붙여도 프론트 수정 거의 없어요. 메시지도 일관됩니다."
```

### JWT 기반 로그인 서버 로직 흐름
#### 호출 흐름
 - Controller -> ClientAuthComponent -> Auth DDD System -> JWT Server -> Controller -> UIComponent

| 이름                                              | 설명                               | 링크                                                                                                                                                      |
| ----------------------------------------------- | -------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `AuthSubsystem::Initialize`                     | 레포지토리와 도메인 서비스 생성 및 의존성 주입 (초기화) | [AuthSubsystem.cpp L15-L33](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/AuthSubsystem.cpp#L15-L33)                        |
| `AuthSubsystem::RequestServerAuthentication`    | 로그인 요청 검증 및 외부 인증 서버 호출          | [AuthSubsystem.cpp L99-L131](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/AuthSubsystem.cpp#L99-L131)                    |
| `AuthSubsystem::SendAuthenticationToAuthServer` | 외부 JWT 서버로 로그인 요청 전송             | [AuthSubsystem.cpp L223-L245](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/AuthSubsystem.cpp#L223-L245)                    |
| `AuthSubsystem::OnAuthenticationResponse`       | JWT 서버 응답 파싱 및 토큰/오류 처리          | [AuthSubsystem.cpp L328-L462](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/AuthSubsystem.cpp#L328-L462)                    |
| `AuthDomainService::AuthenticateUser`           | 로그인 도메인 로직                       | [AuthDomainService.cpp L95-L203](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/Domain/AuthDomainService.cpp#L95-L203)     |
| `AuthComponent`                       | 계정 상태 검증 (로그인 가능 여부)             | [AuthComponent.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthModule/Private/Domain/AuthComponent.cpp)                 |
| `AuthService::RequestLogin`                     | 클라이언트 측 로그인 요청 처리                | [AuthService.cpp L43-L72](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/AuthClientModule/Private/AuthService.cpp#L43-L72)                      |
| `ClientAuthComponent::RequestLogin`             | AuthService로 로그인 요청 전달           | [ClientAuthComponent.cpp L49-L63](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/ClientModule/Private/Player/ClientAuthComponent.cpp#L49-L63) |

---


## DDD 스킬 시스템
<br>

![3.DDD구조](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/3.DDD구조.png)


| 파일                                                | 설명                                                                                                                                 | Permalink                                                                                                                                         |
| ------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **SkillSubsystem**         | 게임 인스턴스 서브시스템으로 도메인 서비스에 대한 호출을 조정하고, 네트워크 권한과 트랜잭션 경계를 처리한다. 스킬 로딩/저장/갱신/스왑 요청을 받아 도메인 서비스로 위임한다.                                 | [SkillSubsystem.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/SkillSubsystem.cpp)                               |
| **SkillDomainService** | 도메인 서비스로서 스킬 등록·해제·쿨다운 갱신 등 비즈니스 로직을 실행한다. `SkillComponent`를 통해 도메인 규칙을 확인한 뒤 저장 작업을 수행하고, 실패 시 이벤트로 알린다.                          | [SkillDomainService.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/SkillDomainService.cpp)                       |
| **SkillComponent**         | 애그리거트 루트. 플레이어의 스킬 슬롯을 관리하고 도메인 규칙을 보호한다. 스킬 등록/해제/스왑, 쿨다운 계산과 UI에 대한 replication을 담당한다.                                           | [SkillComponent.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Components/SkillComponent.cpp)                    |
| **SkillSlot**                   | 도메인 엔티티. 슬롯 인덱스, 스킬 ID, 데이터 자산, 마지막 사용 시간 등을 저장하고 초기화·클리어·쿨다운 계산 등의 기본 메서드를 제공한다.                                                  | [SkillSlot.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Entities/SkillSlot.cpp)                                |
| **SkillDataAsset**                              | 게임 컨텐츠에 저장된 스킬 정의를 나타내며, 스킬 ID, 쿨타임, 비용, 애니메이션 등 시각 효과 데이터를 포함한다.                                                                  | [SkillDataAsset.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Public/Data/SkillDataAsset.h)                               |
| **SkillDtoMapper**         | SQL 결과를 스킬 슬롯/마스터 DTO로 변환하고, DTO를 SQL 파라미터로 매핑한다. 또한 DTO의 유효성을 검사한다.                                                               | [SkillDtoMapper.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Mappers/SkillDtoMapper.cpp)                       |
| **SkillModelBuilder**   | DTO와 DataAsset을 합쳐 도메인 모델을 생성하고, 도메인 모델을 replication 데이터(`FSkillSlotReplicationData`)와 DTO로 변환한다. 쿨다운 계산과 스케일링 등 비즈니스 계산 로직을 포함한다. | [SkillModelBuilder.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Mappers/SkillModelBuilder.cpp)                 |
| **ISkillRepositoryInterface**                   | 리포지토리 인터페이스. 스킬 슬롯 로드/저장, 쿨다운 업데이트, 마스터 데이터 로드 등 영속성 관련 메서드를 정의한다.                                                                 | [ISkillRepositoryInterface.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Public/Repositories/ISkillRepositoryInterface.h) |


![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/9.스킬정보.png)
> DB 스킬 데이터 구조, 실질 DataAsset을 AssetManager에서 관리한다.


## 문제 1. 데이터 저장소 유연성 부족

#### 문제 정의
- 저장소가 특정 구현(MySQL)에 강하게 결합되어 있어 Redis/NoSQL 도입이나 교체, 테스트 격리가 어렵다.

#### 해결 메커니즘
- `USkillRepository`를 추상 인터페이스로 정의하고, `USkillDomainService`에 DI로 주입.  
- 테스트에서는 `MockRepository`로 도메인 로직만 검증.  
- DIP 적용으로 저장소 교체와 테스트 환경 분리 용이.

<br>  

## 문제 2. SQL 스키마 변경에 따른 도메인 로직 영향

#### 문제 정의
- SQL 테이블 컬럼 구조가 변경되면, 도메인 로직이 직접 컬럼에 의존한 상태에서는 비즈니스 코드까지 연쇄적으로 영향을 받아 오염의 범위가 커지는 문제가 발생했다. 이는 곧 유지보수 부담으로 다가온다.

#### 해결 메커니즘
- `FSkillSlotDatabaseDTO`는 DB 스키마를 그대로 반영하는 DTO 역할만 수행하여 스키마 표현을 격리.  
- `USkillDtoMapper`가 DTO ↔ 도메인 모델 간의 변환을 전담해 매핑 책임을 분리.  
- 도메인 모델은 필요한 필드(`SlotIndex`, `SkillId`, `LastUsedTime`)만 사용하고, 쿨다운 계산 등 핵심 로직은 스키마 변경과 무관하게 유지됨.  
- 스키마 변경 시 수정 대상은 오직 Mapper로 한정되어 영향 범위를 최소화.


<br>  

## 문제 3. Component 기반 위임 구조의 필요성

#### 문제 정의
- 기존에 `UObject` 기반으로 상태를 처리하면 언리얼 엔진의 복제 매커니즘 복제의 일관성과 세부 변경 동기화가 어렵고, Actor에 복잡한 상태 로직이 섞이면 역할 분리가 흐려진다.

#### 해결 메커니즘
- `USkillComponent`가 등록/해지/교환 시 슬롯 상태와 쿨다운 검증 등 도메인 규칙을 캡슐화하여 **불변 조건을 보장**.  
- 슬롯 상태 복제는 `FFastArraySerializer` 기반으로 전환하여:  
  - 변경된 항목만 부분적으로 효율적으로 동기화  
  - 복제 일관성 확보
- 이 복제된 상태를 바탕으로 클라이언트 HUD가 실시간 갱신되어 **서버-클라이언트 간 상태 불일치 위험을 줄임**.  

---

## SQL 구조 변경 시나리오 - DDD 기반 구조 도입
```sql
QA: `user_skill_slots` 테이블에 `cooldown_end_time` 컬럼을 추가하려고 하는데, SQL 구조 변경이 도메인 로직(쿨다운 계산 등)에 영향을 줄까 걱정됩니다.  
프로그래머 (기존 구조): 맞습니다. 지금은 도메인 로직이 `LastUsedTime + BaseCooltime` 같은 스키마 세부에 직접 의존해서, 컬럼이 바뀌면 쿨다운 판단 전체가 흔들리고 회귀 테스트도 무거워집니다.  

QA: 그럼 어떻게 안전하게 바꿀 수 있죠?  
프로그래머 (DDD 적용):  
- `FSkillSlotDatabaseDTO`에 `cooldown_end_time` 필드를 추가하고,  
- `USkillDtoMapper`만 수정해서  
  - 값이 있으면 그걸로 남은 쿨다운을 계산하고,  
  - 없으면 기존 `LastUsedTime + BaseCooltime` 경로로 fallback  
  하도록 분기 처리합니다.  
- 스킬 메타/에셋 정의는 `PrimaryAsset`으로 분리되어 있어서 SQL 스키마 변경이 그쪽에 미치는 영향은 거의 없고, 핵심 도메인 로직도 그대로 안정적으로 동작합니다.  
- 결과적으로 도메인 서비스나 컴포넌트는 전혀 수정할 필요가 없으니 변경 여파가 매퍼 내부에 국한됩니다.  

QA: `cooldown_end_time`에 이상한 값이 들어오면요?  
프로그래머: 매퍼에서 방어합니다.  
- `NULL`이면 기존 계산을 쓰고,  
- 너무 미래인 값 등 비정상적 입력이면 `RemainingCooldown`을 0으로 정리하거나 로그를 남기고 안전하게 fallback 처리합니다.  

QA: 빠르게 테스트해볼 수 있나요?  
프로그래머: 실제 DB 없이 `MockRepository`에 테스트용 DTO를 넣고, 매퍼 → 도메인 모델 → 도메인 서비스/컴포넌트 흐름을 검증하면 회귀 테스트가 완료됩니다. 기존 경로도 같은 테스트로 동시에 점검됩니다.  

```

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/7.스킬로드.png)
> DDD 스킬 시스템 기반 플레이어간 다른 스킬 로딩

<br>

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/8.유저스킬.png)

> 유저가 등록한 스킬과 스킬슬롯에 대한 정보를 Table로 저장

<br>

---
## **GAS 시스템 기반 게임플레이**

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/10.GAS시스템.png)

| 이름                           | 설명                                                                                                                                                                             | PERMALINK                                                                                                                                                                                                                                                                                        |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `PlayerState`            | 생성자에서 ASC(AbilitySystemComponent), AttributeSet, 관련 서브컴포넌트들을 만들고 복제 설정을 켠다.  | [GGwaPlayerState.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaPlayerState.h) / [GGwaPlayerState.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaPlayerState.cpp)                                       |
| `PlayerController`       | 폰을 소유할 때 PlayerState에서 ASC를 가져와 입력을 바인딩하고, 상태 변경 시 UI 초기화를 트리거한다.                                                             | [GGwaPlayerController.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaPlayerController.h) / [PlayerController.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaPlayerController.cpp)                   |
| `AbilitySystemComponent` | 게임플레이 이펙트 처리와 큐 실행을 담당하며, 초기화 시 캐릭터와 연결(`InitAbilityActorInfo`)되어 능력/스킬을 부여받고 캐싱되어 입력→스킬 실행 흐름을 가능하게 한다.         | [GGwaAbilitySystemComponent.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h) / [GGwaAbilitySystemComponent.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/GAS/GGwaAbilitySystemComponent.cpp) |
| `AttributeSet`           | 체력/마나/방어력 등 속성을 정의하고, 변경이 있을 때 복제 콜백(`OnRep_`)을 통해 클라이언트 UI 갱신을 돕는다.                                                                           | [GGwaAttributeSet.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/GAS/GGwaAttributeSet.h)                                                                                                                                                                         |
| `Character`              | `PossessedBy`에서 PlayerState의 ASC를 가져와 `InitAbilityActorInfo`로 연결하고, 기본 이동/스킬 능력을 부여하며 입력 흐름을 통해 스킬 실행을 가능하게 한다.                  | [GGwaCharacter.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaCharacter.h) / [GGwaCharacter.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaCharacter.cpp)                                                |

[1]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaPlayerState.h "Portfolio/Source/MyGame/Public/Shared/Player/GGwaPlayerState.h at main · rech4210/Portfolio · GitHub"
[2]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaPlayerState.cpp "Portfolio/Source/MyGame/Private/Shared/Player/GGwaPlayerState.cpp at main · rech4210/Portfolio · GitHub"
[3]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaPlayerController.h "Portfolio/Source/MyGame/Public/Shared/Player/GGwaPlayerController.h at main · rech4210/Portfolio · GitHub"
[4]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaPlayerController.cpp "Portfolio/Source/MyGame/Private/Shared/Player/GGwaPlayerController.cpp at main · rech4210/Portfolio · GitHub"
[5]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h "Portfolio/Source/MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h at main · rech4210/Portfolio · GitHub"
[6]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/GAS/GGwaAbilitySystemComponent.cpp "Portfolio/Source/MyGame/Private/Shared/GAS/GGwaAbilitySystemComponent.cpp at main · rech4210/Portfolio · GitHub"
[7]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/GAS/GGwaAttributeSet.h "Portfolio/Source/MyGame/Public/Shared/GAS/GGwaAttributeSet.h at main · rech4210/Portfolio · GitHub"
[8]: https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/Player/GGwaCharacter.h "Portfolio/Source/MyGame/Public/Shared/Player/GGwaCharacter.h at main · rech4210/Portfolio · GitHub"
[9]: https://github.com/rech4210/Portfolio "GitHub - rech4210/Portfolio"

<br>

### **Ability 실행**


#### GA 스킬 사용 로직 간략 흐름

1. **스킬 캐스팅 시도**  
   SkillCastingService::TryCastSkill이 캐릭터의 SkillComponent와 AbilitySystemComponent를 확인하고, 스킬 데이터(asset)를 가져온 뒤, ASC에서 능력 스펙 핸들을 찾아 TryActivateAbility를 호출해 스킬을 활성화 시도한다.

2. **능력 활성화**  
   UGA_Skill1 같은 Gameplay Ability는 ActivateAbility에서 활성화 가능성 검증, 비용 커밋, 아바타 유효성 체크를 수행한다.

3. **타겟팅 & 몽타주**  
   - 타겟 전략이 Self면 자기 자신 대상으로 하고, 아니라면 AbilityTask_WaitTargetData로 마우스 위치 기반 타겟 데이터를 기다린다.  
   - 유효한 타겟이 들어오면 위치를 캐시하고, GGwaPlayMontageAndWaitForEvent로 캐스트 몽타주를 재생해 완료/중단 콜백을 바인딩한다.

4. **이펙트 및 쿨타임 적용**  
   몽타주 완료 시 OnMontageCompleted에서 SkillContext를 만들어 목표를 탐지하고, MakeOutgoingSpec으로 스킬 효과 및 쿨타임 Gameplay Effect를 생성한다.  
   - 자기 대상이면 자신에게, 대상형이면 각 대상의 ASC에 효과를 적용하고 쿨타임은 자기 자신에게 적용한다.  
   - 마지막으로 EndAbility로 능력을 종료한다.

| 이름 | 설명 | 링크 |
|------|------|------|
| `SkillCastingService` | 스킬 캐스팅 서비스. 캐릭터로부터 SkillComponent와 ASC를 찾아 유효한 스킬 데이터를 로드하고 TryActivateAbility로 능력을 실행한다. | [SkillCastingService.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Services/SkillCastingService.cpp) |
| `GA_Base` | 모든 스킬 능력의 기반 클래스. FSkillContext를 포함하고 컨텍스트 빌드 함수와 타겟 ASC를 찾는 유틸리티를 제공한다. | [GA_Base.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/GAS/Skill/GA_Base.h) |
| `GA_Skill1` | 예제 스킬. ActivateAbility에서 타겟팅을 시작하고 몽타주를 재생한 뒤, 완료 시 효과와 쿨타임 Gameplay Effect를 생성해 대상에게 적용한다. | [GA_Skill1.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/GAS/Skill/GA_Skill1.cpp) |
| `SkillDataAsset` | 스킬 메타 정보를 담은 데이터 자산. 스킬 ID, 타겟 전략, Ability/Effect 클래스, 쿨타임, 비용, 애니메이션 몽타주 등을 정의하여 스킬 실행 시 활용된다. | [SkillDataAsset.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Public/Data/SkillDataAsset.h) |
| `SkillComponent` | 스킬 슬롯과 스킬 데이터 복제를 담당. FSkillSlotReplicationData로 슬롯 정보를 동기화하고, 쿨다운 및 캐시된 스킬 데이터를 관리한다. | [SkillComponent.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Public/Components/SkillComponent.h) |
| `ULocalDataBaseLoader` | 로컬 데이터베이스/데이터 자산을 로딩하고 매핑하여 능력 시스템과 스킬 시스템이 사용할 수 있도록 제공하는 캐시 계층. DataAsset 기반 메타와 DB 데이터를 결합해 안정적인 컨텍스트를 구성한다. | [ULocalDataBaseLoader.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Public/Utill/LocalDataBaseLoader.h) / [ULocalDataBaseLoader.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/SkillModule/Private/Utill/LocalDataBaseLoader.cpp) |



Ability가 실행되면 `GameplayEffect`가 적용되며, GE의 `SetByCaller`를 사용해 `SkillID`를 함께 전달합니다.  
> 하지만 `SetByCaller`는 메모리 기반 데이터로 구성되기 때문에 네트워크 직렬화가 어렵고, 클라이언트와 서버 간 안정적인 데이터 공유가 불가능합니다.

이를 위해 Unreal의 **AssetManager**를 활용하여, 서버와 클라이언트가 공유할 수 있는 **PrimaryAssetId 기반 자산 동기화 방식**을 사용합니다.  
런타임에 자산 로드가 이루어지며, ULocalDataBaseLoader를 통해 캐시된 에셋을 사용할 수 있습니다.

**SkillPolicy 전략 :** 타겟팅 방식은 `SkillDataAsset`에 정의된 전략에 따라 분기되며, 범위형 스킬의 경우 커스텀 TargetActor를 생성하고, Task 를 통해 플레이어 입력을 기다립니다.

```cpp
if (SkillDataAsset->TargetStrategyClass->IsChildOf(USkillTarget_Self::StaticClass())) {
	OnTargetDataReceived(FGameplayAbilityTargetDataHandle());
}
else {
    ASkillTargetActor_Mouse* TargetActor = NewObject<ASkillTargetActor_Mouse>(this);
    UAbilityTask_WaitTargetData* DataTask = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(...);
    DataTask->ValidData.AddDynamic(this, &UGA_Skill1::OnTargetDataReceived);
    DataTask->Cancelled.AddDynamic(this, &UGA_Skill1::OnTargetDataCancelled);
    DataTask->ReadyForActivation();
}
```



![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/10.플레이어간스킬사용GE적용.png)
> GE가 적용되고, 타겟 ASC에 접근하여 체력이 수정되었습니다.


---

### 스킬 ID 기반 에셋 조회 및 로딩 흐름 (SkillID ↔ AssetID 매핑)

<br>

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/11.스킬에셋.png)  
#### 설계 이유
 - **디자이너 친화적 스킬 정의**: 기획자가 스킬을 쉽게 관리하도록 `AssetManager` 기반의 `SkillDataAsset` 구조를 도입.
 - AssetManager의 스킬 자산의 로딩, 의존성, 생명주기를 중앙 관리하고 비동기 preload 및 스트리밍 최적화로 안정적이고 빠른 접근을 보장한다.

#### 스킬 로드과정
 - 서버는 DB에서 스킬 ID 컬럼을 기반으로 `ULocalDataBaseLoader`를 통해 대응하는 `PrimaryAssetId`를 조회한다.  
 - 게임 시작 시 `AssetManager`로 스킬 자산을 선로드하고, 이를 바탕으로 `SkillID → PrimaryAssetId` 매핑 테이블을 만든다.  
 - 런타임에는 이 매핑을 써서 `USkillDataAsset`을 안정적으로 참조한다.

 이로써 직렬화가 어려운 데이터를 직접 전송하지 않고도, 공통 자산 참조를 통해 안정적으로 스킬 데이터를 동기화할 수 있게 됨.


```cpp
void ULocalDataBaseLoader::Initialize(){
	UAssetManager& Manager = UAssetManager::Get();
	TArray<FPrimaryAssetId> Ids;
	Manager.GetPrimaryAssetIdList("Skill", Ids);

	for (const FPrimaryAssetId& Id : Ids){
		SkillIdToAssetId.Add(Data->SkillID, Id);
	}
}
```

---

## **서버 → 클라이언트 UI 동기화 / DataAsset 기반 HUD 처리**

#### 설계 이유

- **SRP 위임**: PlayerController는 전달/수신만 하고 UI 갱신 책임은 ClientUIComponent에 넘겨 단일 책임을 지켜 확장성과 테스트 용이성을 확보한다.  
- **클라이언트-서버 분리**: 서버가 진실(source of truth)인 스킬 상태를 관리하고 Replicate + RPC로 필요한 데이터를 클라이언트에 전달하여 상태 복제와 UI 트리거를 분리한다.  


#### 서버-클라이언트 UI 동기화 플로우

1. **스킬 상태 변경 (서버)**  
   서버가 SkillComponent 내부에서 스킬 상태를 바꾸면 해당 배열이 **Replicate**되어 `FSkillSlotReplicationArray`가 동기화 준비된다.

2. **HUD 데이터 전달 (서버 → 클라이언트 RPC)**  
   서버는 `AGGwaPlayerController::SkillHUDReplication`을 **RPC**로 호출해 복제된 슬롯 배열을 클라이언트 쪽 컨트롤러로 보낸다.

3. **SRP에 따라 위임 (컨트롤러 → UI 컴포넌트)**  
   PlayerController는 책임 분리(SRP) 원칙대로 받은 데이터 처리를 UI 전담 `ClientUIComponent`에 위임한다.

4. **UI 갱신 (클라이언트)**  
   `ClientUIComponent`가 슬롯/보스 데이터를 받고 로컬 컨텍스트(AssetManager 등)로 필요한 스킬 정보를 재구성한 뒤 HUD 위젯에 반영한다. ASC와 AttributeSet의 **Replicated** 속성 변화 및 GameplayEffect 콜백을 구독해 체력/마나, 버프, 스킬 상태 등을 실시간으로 업데이트한다.


   | 파일                     | 설명                                                                                     | Permalink                                                                                                                                 |
   |--------------------------|------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
   | `GGwaPlayerController`   | 서버에서 스킬 슬롯 배열을 클라이언트로 전달하고 UI 매니저로 위임하는 `SkillHUDReplication` 등을 처리하며, 서버 연결 흐름도 관리한다. | [GGwaPlayerController.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/Player/GGwaPlayerController.cpp) |
   | `ClientUIComponent`      | HUD/위젯 초기화, ASC/AttributeSet 바인딩, 서버에서 받은 스킬 및 보스 데이터를 받아 UI를 갱신한다.                    | [ClientUIComponent.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/ClientModule/Private/Player/ClientUIComponent.cpp)        |
   | `GGwaHUD`                | 능력 데이터와 보스 데이터를 하위 위젯에 전달해 전체 HUD 상태를 조율한다.                                            | [GGwaHUD.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/ClientModule/Private/UI/GGwaHUD.cpp)                                |
   | `GGwaWidget`             | ASC/AttributeSet 변경과 효과 적용을 받아 툴팁, 스킬 바, 체력/마나 바 등을 실시간 업데이트한다.                        | [GGwaWidget.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/ClientModule/Private/UI/Widget/GGwaWidget.cpp)                   |

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/12.ASCAT.png)

> 서버로부터 초기 데이터셋을 받아 HUD를 구성하고 GAS 기반 AttributeSet으로 플레이어의 스탯을 설정

#### 버프 및 스킬 쿨타임 UI 처리

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/13.UI매핑.png)

위 구조로 서버는 단순히 Data만 클라이언트에 복제하면, 클라이언트는 일관된 방식으로 HUD를 구성할 수 있습니다.

---

### 보스 캐릭터 구현

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/14.보스BT.png)

> 보스의 Behavior Tree

| 이름 | 설명 | 링크 |
|------|------|------|
| `BossCharacter` | 보스 루트. ASC 초기화, 능력 부여, 속성 관찰자 바인딩, 보스 데이터 캐싱 및 타겟 탐지 등을 수행한다. | [BossCharacter.h](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Public/Shared/AI/BossCharacter.h) / [BossCharacter.cpp](https://github.com/rech4210/Portfolio/blob/27c92a3b332970da3d2fdef841c3a48a90db0ce5/Source/MyGame/Private/Shared/AI/BossCharacter.cpp) |

보스 캐릭터는 Behavior Tree를 기반으로 동작하며, 총 3단계의 페이즈로 구성되어 있습니다.  
타겟이 감지되면 공격을 시작하고, 일부 페이즈에서는 광역 공격과 쿨다운 관리도 함께 처리됩니다.

![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/15.보스공격.png)

> 근접 공격과 피격에 따른 체력 감소

-   보스는 공격하여 피해를 입히고, 플레이어와 보스는 각각 **GAS** 시스템을 가지고 있으며, 피격 시 실시간으로 수치가 반영됩니다.

---
## 4. 배운 점

**1. 모듈화 & DDD (DIP, SRP)**  
- 책임 분리: Subsystem → DomainService → Component → Repository → DBManager로 계층을 나눠 변경 시 영향 범위를 좁혔다.
- 인프라 유연성: 저장소를 추상화해서 MySQL 외에 Redis/NoSQL 같은 보조 DB를 시험 적용할 수 있었다.  
- 확장성: 새로운 기능을 기존 코드 수정 없이 느슨하게 붙일 수 있게 설계되었다.  
- 테스트 용이성: MockRepository 등을 활용해 실제 DB 없이도 빠른 검증을 수행할 수 있게 하였다.

**2. GAS × Dedicated Server 통합**  
- 서버/클라이언트 역할 분리: 서버가 진실을 관리하고, 클라이언트는 복제된 상태와 이벤트로 UI를 즉시 반응시키는 협업 모델을 배웠다.  
- 복제: OnRep_, RPC, Replicate 흐름과 서버 측 요청등 네트워크 흐름을 이해하게 되었다.  
- 직렬화 대응: 네트워크간의 메모리 복제 문제를 primitive 타입 인자로 AssetManager 기반 SkillDataAsset ↔ PrimaryAssetId 매핑으로 우회해 안정적인 컨텍스트 재구성을 했다.  
- 실전적 동기화 이해: Unreal 복제 정책, FastArraySerializer, RPC 위임 등을 실제 시스템에서 경험하며 개념을 체감했다.

**3. Docker 기반 DB 환경**  
- 환경 일치: 로컬, CI, 스테이징 간 동일한 데이터베이스 상태를 재현해 "내 환경만 되는" 문제를 개선하고자 했다.
- 초기화 자동화: 스키마와 시드를 자동 적용해 수동 설정 실수를 제거했다.  
- 지속성 보장: 볼륨을 통해 컨테이너 재시작에도 데이터가 유지되어 개발 중 손실 위험을 낮췄다.

**4. UE::Tasks 사용 이유**  
- Tick 블로킹 방지: I/O나 DB 작업을 게임 스레드와 분리해 프레임 드랍을 피했다. (이는 게임 스레드의 핵심 로직이 거의 싱글 스레드 기반이기 때문이다)
- 서버 스케일링 대응: 동시 다수 요청을 병렬로 처리할 수 있기에 안정적인 대응이 가능하다.  
- 스레드 안전성: 공식 비동기 API로 비동기 결과를 GameThread로 안전하게 수행하는 래퍼를 제공한다.

**5. JWT 기반 인증**  
- 책임 분리: 민감한 인증 로직을 외부 JWT 서버에 위임해 게임 서버의 보안 경계를 명확히 했다.  
- 감사 로그 중앙화: AuthDomainService가 주요 이벤트를 기록해 이상 징후 탐지를 수행했다.  
- 정책 국소화: 비밀번호 정책 변경 등이 전체 시스템에 파급되지 않고 도메인 내부에서 처리되도록 했다.

**6. 트랜잭션 & 원자성**  
- 일관성 확보: 복합 도메인 작업을 단일 트랜잭션이나 방어적 매퍼로 처리해 상태 정합성을 지켰다.  

---

## 5. 구조 개선 방향

**1. DBModule 응집 완화**  
- 문제: 현재 모든 DTO/CRUD가 하나의 DBManager에 몰려 있어 응집이 과도하다.  
- 개선: CharacterDBManager, SkillDBManager처럼 컨텍스트별 관리자들로 분리하고, 공통 로직은 내부 유틸로 추출하며 외부에는 인터페이스만 노출한다.

**2. 아웃박스 패턴 도입 (원자성 강화)**  
- 목적: 상태 변경과 그에 따른 후속 처리(예: 슬롯 갱신 → HUD 반영)를 동일 트랜잭션 맥락 안에서 보장한다.  
- 설계: 변경 시 아웃박스 테이블에 이벤트를 기록하고, 별도 워커가 이를 읽어 내부 이벤트 버스나 경량 메시지 큐로 발행한다.

**3. UI 책임 분리 & 뷰모델 도입**  
- 문제: UI 수신부터 렌더링까지 하나의 컴포넌트에 몰려 있어 확장/테스트가 어렵다.  
- 개선: SkillHUDViewModel 같은 중간 표현을 두어 상태 해석을 분리하고, 위젯은 단순히 바인딩만 하도록 한다.

**4. CI/CD 고도화**  
![Image](https://raw.githubusercontent.com/rech4210/Portfolio/27c92a3b332970da3d2fdef841c3a48a90db0ce5/ScreenShot/16.CI.png)

현재 Git Action으로 CI를 시도하였으나 미흡하다고 판단.
- 스키마-도메인 동기화 테스트: 스키마 변경 발생 시 테스트 DB를 띄워 전체 흐름(예: 스킬 등록/쿨다운)을 자동으로 검증한다.  
- Feature Flag / Canary Release: 변경을 제한된 그룹에 먼저 적용해 문제를 조기에 발견한다.
