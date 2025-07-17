# 인증 시스템 실행 흐름 분석 (커밋 eef3e76 기반)

## 1. 개요

해당 커밋은 인증 시스템에 대한 대대적인 리팩토링을 포함합니다. 주요 변경점은 다음과 같습니다.

- **인증 서버 분리**: 기존 Unreal Engine 게임 서버가 모두 처리하던 인증 로직을 별도의 **Node.js 기반 외부 인증 서버**로 분리했습니다.
- **역할과 책임 명확화**:
    - **Node.js 서버**: 사용자 계정 생성, 비밀번호 검증, JWT 토큰 발급 및 검증 등 순수 인증만 담당합니다.
    - **Unreal Engine 서버**: Node.js 서버에서 발급된 토큰을 받아 게임 로직을 처리하고, `DatabaseModule`을 통해 게임 관련 데이터를 관리합니다.
- **DDD 원칙 강화**: 각 모듈의 책임이 명확해졌습니다.
    - `AuthModule`은 이제 외부 인증 서버와의 통신을 담당하는 **Application Service** 역할에 집중합니다.
    - `DatabaseModule`은 순수하게 게임 데이터(유저 프로필, 재화 등) CRUD에만 집중하며, 더 이상 인증 관련 로직(비밀번호 해싱 등)을 포함하지 않습니다.

---

## 2. 관련 모듈 및 역할

| 모듈/서버 | 주요 역할 | 관련 클래스/파일 |
|---|---|---|
| **ClientModule** | **(클라이언트)** 로그인/회원가입 UI 상호작용 | `LoginPlayerController` |
| **AuthClientModule** | **(클라이언트)** UI와 서버 통신(RPC)을 연결하는 서비스 | `AuthService` |
| **MyGame (Core)** | **(서버)** 클라이언트의 RPC를 수신하는 PlayerController | `GGwaPlayerController` |
| **AuthModule** | **(UE 서버)** 외부 인증 서버와 통신을 조율하는 Application Service | `AuthSubsystem` |
| **DatabaseModule** | **(UE 서버)** 게임 데이터베이스 접근 (인증 정보 제외) | `DatabaseManager` |
| **Node.js 서버** | **(외부 인증 서버)** 실제 인증 처리 및 JWT 토큰 관리 | `app.js` |

---

## 3. Use Case 1: 신규 유저 회원가입

### 실행 흐름 (Step-by-Step)

1.  **[Client] `LoginPlayerController`**: 유저가 UI에 아이디와 비밀번호를 입력하고 '회원가입' 버튼을 클릭합니다. `AuthService`의 `RequestRegister` 함수를 호출합니다.

2.  **[Client] `AuthService`**: `LoginPlayerController`로부터 받은 정보를 사용하여 `Server_Register` RPC를 호출하여 서버에 요청을 보냅니다.

3.  **[UE Server] `GGwaPlayerController`**: `Server_Register_Implementation` RPC가 실행됩니다. 이 함수는 `AuthSubsystem`을 가져와 `RequestServerRegistration` 함수를 호출합니다.

4.  **[UE Server] `AuthSubsystem`**:
    - `RequestServerRegistration`이 호출되면, HTTP 요청 DTO(`FAuthRequestDTO`)를 생성합니다.
    - 이 DTO를 JSON으로 변환하여 **Node.js 인증 서버**의 `/register` 엔드포인트로 HTTP POST 요청을 보냅니다. ( `SendRegistrationToAuthServer` 함수 사용)

5.  **[Node.js Server] `app.js`**:
    - `/register` 요청을 수신합니다.
    - 받은 아이디가 DB에 이미 존재하는지 확인합니다.
    - 비밀번호를 `bcrypt`를 사용해 **해싱(Hashing)**합니다.
    - 해싱된 비밀번호와 함께 유저 정보를 `users` 테이블에 저장합니다.
    - 성공 또는 실패 메시지를 JSON 형태로 Unreal Engine 서버에 응답합니다.

6.  **[UE Server] `AuthSubsystem`**:
    - Node.js 서버로부터 받은 HTTP 응답을 처리합니다 (`OnRegistrationResponse` 핸들러).
    - 응답 결과를 분석하여 성공 여부와 메시지를 `OnServerRegistrationComplete` 델리게이트에 브로드캐스트합니다.

7.  **[UE Server] `GGwaPlayerController`**:
    - `AuthSubsystem`의 델리게이트(`OnAuthSubsystemRegistrationComplete`)를 구독하고 있다가 결과가 오면 `Client_OnRegistrationResult` RPC를 호출하여 클라이언트에 결과를 전달합니다.

8.  **[Client] `LoginPlayerController` & `AuthService`**:
    - `Client_OnRegistrationResult`가 실행되면, `AuthService`는 이 결과를 UI에 반영하여 유저에게 회원가입 성공 또는 실패 피드백을 보여줍니다.

---

## 4. Use Case 2: 기존 유저 로그인

### 실행 흐름 (Step-by-Step)

1.  **[Client] `LoginPlayerController`**: 유저가 UI에 아이디와 비밀번호를 입력하고 '로그인' 버튼을 클릭합니다. `AuthService`의 `RequestLogin` 함수를 호출합니다.

2.  **[Client] `AuthService`**: `LoginPlayerController`로부터 받은 정보를 사용하여 `Server_Login` RPC를 호출합니다.

3.  **[UE Server] `GGwaPlayerController`**: `Server_Login_Implementation` RPC가 실행됩니다. 이 함수는 `AuthSubsystem`의 `RequestServerAuthentication` 함수를 호출합니다.

4.  **[UE Server] `AuthSubsystem`**:
    - `RequestServerAuthentication`이 호출되면, HTTP 요청 DTO를 생성합니다.
    - 이 DTO를 JSON으로 변환하여 **Node.js 인증 서버**의 `/login` 엔드포인트로 HTTP POST 요청을 보냅니다. (`SendAuthenticationToAuthServer` 함수 사용)

5.  **[Node.js Server] `app.js`**:
    - `/login` 요청을 수신합니다.
    - DB에서 요청된 아이디의 유저 정보를 조회합니다.
    - `bcrypt.compare`를 사용해 유저가 입력한 비밀번호와 DB에 저장된 해시를 비교합니다.
    - 비밀번호가 일치하면, 해당 유저의 ID를 담은 **JWT(JSON Web Token)**를 생성하고 서명합니다.
    - 생성된 토큰과 유저 ID를 JSON 형태로 Unreal Engine 서버에 응답합니다.

6.  **[UE Server] `AuthSubsystem`**:
    - Node.js 서버로부터 받은 HTTP 응답을 처리합니다 (`OnAuthenticationResponse` 핸들러).
    - 응답이 성공적이면, JSON에서 **토큰**과 **유저 ID**를 파싱합니다.
    - **(중요)** 이제 `DatabaseModule`의 `DatabaseManager`를 통해 `GetUserById`를 호출하여 해당 유저의 **게임 데이터(레벨, 재화 등)**가 DB에 존재하는지 확인합니다.
        - 만약 데이터가 없다면, 신규 유저로 간주하고 기본 게임 데이터를 생성해줍니다 (`CreateUserAccount` 호출).
    - `OnServerAuthenticationComplete` 델리게이트에 성공 여부, 토큰, 유저 ID를 담아 브로드캐스트합니다.

7.  **[UE Server] `GGwaPlayerController`**:
    - `AuthSubsystem`의 델리게이트(`OnAuthSubsystemAuthenticationComplete`)를 구독하고 있다가 결과가 오면 `Client_OnLoginResult` RPC를 호출하여 토큰과 유저 ID를 클라이언트에 전달합니다.
    - 동시에, 성공 시 `Client_TravelToGameWorld`를 호출하여 클라이언트의 월드 이동을 지시합니다.

8.  **[Client] `LoginPlayerController` & `AuthService`**:
    - `Client_OnLoginResult`가 실행되면, `AuthService`는 받은 토큰과 유저 ID를 안전하게 저장합니다. 이 토큰은 이후 서버에 API를 요청할 때마다 인증 헤더에 포함됩니다.
    - `Client_TravelToGameWorld`가 실행되면, 클라이언트는 지정된 게임 월드 맵으로 이동하여 게임 플레이를 시작합니다.

---

## 5. DDD 관점 분석

- **계층 분리**:
    - **Application Layer**: `AuthSubsystem`이 외부 서비스(Node.js)와의 통신을 조율하고 전체 유스케이스를 오케스트레이션하는 역할을 완벽하게 수행합니다.
    - **Domain Layer**: `AuthDomainService`는 이제 비즈니스 로직보다는 데이터 검증 등 간단한 역할만 수행하거나, 복잡한 게임 관련 인증 규칙이 추가될 경우 확장될 수 있습니다.
    - **Infrastructure Layer**: `AuthRepository`는 이제 외부 HTTP 통신을 추상화하는 역할을 하며, `DatabaseManager`는 DB 접근을 담당하는 명확한 인프라 계층이 되었습니다.

- **모듈 의존성**:
    - `AuthModule`은 더 이상 `DatabaseModule`에 직접적으로 강하게 의존하지 않습니다. 인증 자체는 외부 서버에 위임하고, 인증 성공 후 게임 데이터 조회를 위해 `DatabaseModule`을 사용하는 흐름으로 변경되어 의존성이 더 명확해졌습니다.
    - `DatabaseModule`은 비밀번호 해싱과 같은 인증 관련 책임에서 완전히 벗어나 순수 데이터 관리 책임만 갖게 되어 **단일 책임 원칙(SRP)**을 잘 따르게 되었습니다.

- **결론**: 이번 리팩토링은 인증과 게임 로직을 성공적으로 분리하여 각 모듈과 서버가 자신의 책임에만 집중할 수 있도록 만든 좋은 DDD 사례입니다. 시스템의 확장성과 유지보수성이 크게 향상되었습니다.
