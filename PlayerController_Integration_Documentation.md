# PlayerController 통합 및 UI 관리 개선 문서

## 1. 개요

이 문서는 `LoginPlayerController`의 기능을 `GGwaPlayerController`로 통합하고, 맵별 자동 UI 관리를 위한 `UIManagerSubsystem`을 구현한 작업을 설명합니다.

## 2. 주요 변경사항

### 2.1 LoginPlayerController → GGwaPlayerController 통합

#### Before (분리된 구조)
```
LoginPlayerController (ClientModule)
├── UI 관리 (위젯 생성/제거)
├── AuthService 인스턴스 관리
├── 사용자 입력 처리 (RequestLogin/RequestRegistration)
└── ClientTravel 처리

GGwaPlayerController (MyGame)
├── RPC 통신 (Server_Login/Server_Register)
├── AuthSubsystem과의 연동
└── 게임 로직 처리
```

#### After (통합된 구조)
```
GGwaPlayerController (MyGame)
├── [Client] UI 관리 및 사용자 입력 처리
├── [Client] AuthService 인스턴스 관리
├── [Client] ClientTravel 처리
├── [Server] RPC 통신 (Server_Login/Server_Register)
├── [Server] AuthSubsystem과의 연동
├── [Both] AuthRPCInterface 구현
└── [Both] 게임 로직 처리
```

### 2.2 새로 추가된 기능

#### GGwaPlayerController에 통합된 LoginPlayerController 기능

1. **클라이언트 측 인증 기능**:
   - `RequestRegistration(Username, Password)`: UI에서 호출되는 등록 요청
   - `RequestLogin(Username, Password)`: UI에서 호출되는 로그인 요청
   - `ConnectToGameServer(ServerAddress, Token)`: 토큰을 포함한 서버 연결

2. **Blueprint 이벤트**:
   - `OnRegistrationResult_BP`: 등록 결과를 UI에 전달
   - `OnLoginResult_BP`: 로그인 결과를 UI에 전달

3. **AuthService 콜백 처리**:
   - `OnRegistrationComplete`: AuthService의 등록 완료 콜백
   - `OnLoginComplete`: AuthService의 로그인 완료 콜백

#### UIManagerSubsystem (새로 생성)

1. **자동 맵별 UI 관리**:
   - `MapWidgetMap`: 맵 이름과 위젯 클래스 매핑
   - `OnMapChanged`: 맵 전환 시 자동 위젯 교체
   - `CurrentWidget`: 현재 활성 위젯 캐싱

2. **수동 제어 기능**:
   - `SetWidgetForMap`: 런타임 위젯 설정
   - `RefreshCurrentWidget`: 현재 위젯 새로고침
   - `RemoveCurrentWidget`: 현재 위젯 제거

## 3. 아키텍처 분석

### 3.1 클라이언트/서버 역할 분리

#### 클라이언트 (IsLocalController())
```cpp
#if !UE_SERVER && defined(CLIENTMODULE_API)
    // UI 입력 → AuthService → AuthRPCInterface → Server RPC
    void RequestLogin(Username, Password) {
        AuthService->RequestLogin(Username, Password, Delegate);
    }
#endif
```

#### 서버 (HasAuthority())
```cpp
// RPC 수신 → AuthSubsystem → 외부 JWT 서버 → 클라이언트 응답
void Server_Login_Implementation(Username, Password) {
    AuthSubsystem->RequestServerAuthentication(Username, Password, ClientIP, this);
}
```

### 3.2 의존성 주입을 통한 모듈 분리

```cpp
// AuthRPCInterface를 통한 추상화
class IAuthRPCInterface {
    virtual void RequestServerLogin(Username, Password) = 0;
    virtual void RequestServerRegistration(Username, Password) = 0;
    virtual bool IsAuthRPCAvailable() const = 0;
    virtual void Request_Client_TravelToGameWorld(MapURL) = 0;
};

// GGwaPlayerController가 인터페이스 구현
class AGGwaPlayerController : public APlayerController, public IAuthRPCInterface
```

### 3.3 컴파일 조건부 분리

```cpp
// 클라이언트 전용 코드
#if !UE_SERVER && defined(CLIENTMODULE_API)
    TObjectPtr<UAuthService> AuthService;
    void RequestLogin(...);
    void OnLoginComplete(...);
#endif

// 서버 전용 코드 (또는 공통 코드)
void Server_Login_Implementation(...);
void OnAuthSubsystemAuthenticationComplete(...);
```

## 4. 실행 흐름

### 4.1 통합된 로그인 흐름

1. **UI 입력**: 사용자가 UI에서 로그인 정보 입력
2. **GGwaPlayerController::RequestLogin**: 클라이언트에서 AuthService로 요청 전달
3. **AuthService**: AuthRPCInterface를 통해 서버 RPC 호출
4. **GGwaPlayerController::Server_Login_Implementation**: 서버에서 AuthSubsystem으로 위임
5. **AuthSubsystem**: 외부 JWT 서버로 인증 요청
6. **JWT 서버**: 인증 처리 후 토큰 발급
7. **AuthSubsystem**: 응답 수신 후 이벤트 브로드캐스트
8. **GGwaPlayerController**: 클라이언트로 결과 전송 (Client_OnLoginResult)
9. **AuthService**: 클라이언트에서 결과 처리
10. **UI 업데이트**: Blueprint 이벤트를 통해 UI 갱신

### 4.2 UIManagerSubsystem 작동 흐름

1. **초기화**: `FWorldDelegates::OnPostLoadMapWithWorld` 등록
2. **맵 로드**: 새 맵이 로드되면 `OnMapChanged` 호출
3. **맵 이름 추출**: `UGameplayStatics::GetCurrentLevelName` 사용
4. **위젯 검색**: `MapWidgetMap`에서 해당 맵의 위젯 클래스 찾기
5. **이전 위젯 제거**: 기존 `CurrentWidget` 정리
6. **새 위젯 생성**: `CreateWidget` 후 `AddToViewport`
7. **캐싱**: `CurrentWidget`에 새 위젯 저장

## 5. 사용법

### 5.1 GGwaPlayerController 사용 (Blueprint)

```cpp
// 등록 요청
Event RequestRegistration(Username: "user123", Password: "password")

// 로그인 요청  
Event RequestLogin(Username: "user123", Password: "password")

// 결과 처리
Event OnLoginResult_BP(bSuccess: true, Token: "jwt_token", UserId: "user_id")
    -> ConnectToGameServer("127.0.0.1:7777", Token)
```

### 5.2 UIManagerSubsystem 설정 (Blueprint/Editor)

```cpp
// MapWidgetMap 설정 예시
{
    "LoginLevel" -> LoginWidget_BP,
    "GameLevel" -> GameHUD_BP,
    "MenuLevel" -> MainMenu_BP
}
```

## 6. 마이그레이션 가이드

### 6.1 기존 LoginPlayerController 사용 코드

**Before:**
```cpp
ALoginPlayerController* LoginPC = Cast<ALoginPlayerController>(GetController());
LoginPC->RequestLogin(Username, Password);
```

**After:**
```cpp
AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(GetController());
PC->RequestLogin(Username, Password);
```

### 6.2 Blueprint 마이그레이션

1. **PlayerController 클래스 변경**:
   - `LoginPlayerController` → `GGwaPlayerController`

2. **함수 이름은 동일하게 유지**:
   - `RequestLogin`, `RequestRegistration`, `ConnectToGameServer`
   - `OnLoginResult_BP`, `OnRegistrationResult_BP`

3. **UIManagerSubsystem 활용**:
   - 기존 수동 위젯 관리 코드 제거
   - `MapWidgetMap` 설정으로 자동 관리 활성화

## 7. 향후 계획

### 7.1 LoginPlayerController 제거 일정

1. **Phase 1 (현재)**: Deprecated 마킹 및 경고 메시지
2. **Phase 2**: 사용 중인 코드 마이그레이션 완료
3. **Phase 3**: LoginPlayerController 완전 제거

### 7.2 추가 개선 사항

1. **UIManagerSubsystem 확장**:
   - 위젯 전환 애니메이션 지원
   - 커스텀 전환 조건 설정

2. **GGwaPlayerController 최적화**:
   - 더 세분화된 클라이언트/서버 분리
   - 성능 최적화 및 메모리 관리 개선

---

## 6. 개선된 Replicated UI 관리 시스템

### 6.1 새로운 아키텍처 개요

**Before (로컬 UI 관리)**:
```
UIManagerSubsystem
├── Local MapWidgetMap (TMap<FName, TSubclassOf<UUserWidget>>)
├── FWorldDelegates::OnPostLoadMapWithWorld (컴파일 에러 발생)
└── 클라이언트별 독립적 UI 설정
```

**After (Replicated UI 관리)**:
```
Server: GGwaGameMode
├── BeginPlay() → UIConfigCacheActor 생성
├── CacheActor→InitializeDefaultMappings()
└── GameState→SetCacheActor() → Multicast_InitCacheActor()

Replicated: UIConfigCacheActor
├── MapWidgetMap (Replicated TMap)
├── SetWidgetForMap() (Server Authority)
└── OnRep_MapWidgetMap() (Client Notification)

Client: UIManagerSubsystem
├── SetCacheActor() (from GameState)
├── OnMapChanged() (Manual Trigger)
└── RequestSetWidgetForMap() (Cache Actor Delegation)
```

### 6.2 새로운 클래스 구조

#### AGGwaGameState
```cpp
class AGGwaGameState : public AGameStateBase
{
    UPROPERTY(Replicated)
    TObjectPtr<AUIConfigCacheActor> CacheActor;
    
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_InitCacheActor(AUIConfigCacheActor* NewActor);
};
```

#### AUIConfigCacheActor
```cpp
class AUIConfigCacheActor : public AActor
{
    UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite)
    TMap<FName, TSubclassOf<UUserWidget>> MapWidgetMap;
    
    UFUNCTION()
    void OnRep_MapWidgetMap();
    
    void SetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);
};
```

#### UUIManagerSubsystem (개선됨)
```cpp
class UUIManagerSubsystem : public UGameInstanceSubsystem
{
    UPROPERTY(Transient)
    TObjectPtr<AUIConfigCacheActor> CacheActor;
    
    void SetCacheActor(AUIConfigCacheActor* NewCacheActor);
    void RequestSetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);
};
```

### 6.3 실행 흐름

#### 서버 초기화 흐름
1. **`AGGwaGameMode::BeginPlay()`**: 서버에서만 실행
2. **UIConfigCacheActor 생성**: `SpawnActor<AUIConfigCacheActor>()`
3. **기본 매핑 초기화**: `CacheActor->InitializeDefaultMappings()`
4. **GameState에 할당**: `GameState->SetCacheActor(CacheActor)`
5. **클라이언트 알림**: `Multicast_InitCacheActor(CacheActor)` 호출

#### 클라이언트 동기화 흐름
1. **멀티캐스트 수신**: `Multicast_InitCacheActor_Implementation()` 실행
2. **UIManager 알림**: `UIManagerSubsystem->SetCacheActor(NewActor)`
3. **현재 위젯 갱신**: `RefreshCurrentWidget()` 자동 호출
4. **맵별 UI 표시**: 캐시된 매핑에 따라 적절한 위젯 생성

#### 동적 설정 변경 흐름
1. **클라이언트 요청**: `UIManagerSubsystem->RequestSetWidgetForMap()`
2. **서버 처리**: `UIConfigCacheActor->SetWidgetForMap()` (Authority 체크)
3. **자동 복제**: `DOREPLIFETIME_CONDITION_NOTIFY` 통해 모든 클라이언트에 전파
4. **클라이언트 갱신**: `OnRep_MapWidgetMap()` 호출로 로컬 상태 동기화

### 6.4 장점

1. **서버 권한 관리**: UI 설정이 서버에서 중앙 관리되어 모든 클라이언트가 동일한 UI를 가짐
2. **자동 동기화**: 새로 접속하는 클라이언트도 자동으로 최신 UI 설정을 받음
3. **컴파일 안정성**: `FWorldDelegates` 의존성 제거로 컴파일 에러 해결
4. **확장성**: 런타임에 서버에서 UI 설정을 동적으로 변경 가능

---

**변경일**: 2025년 7월 18일  
**작성자**: 시스템 통합팀  
**리뷰어**: 아키텍처팀
