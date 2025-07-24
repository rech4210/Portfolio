# 3-Layer Mapping Architecture 사용 가이드

## 📋 개요

새로운 3계층 매핑 아키텍처를 사용하여 스킬 시스템의 데이터 흐름을 관리하는 방법을 설명합니다.

## 🏗️ 아키텍처 구조

```
SQL Database ↔ DatabaseDTO ↔ SkillDataAsset(VO) ↔ DomainModel ↔ SkillSlot(Entity)
     ↑              ↑              ↑                 ↑              ↑
DtoMapper    AssetMapper    ModelBuilder      SkillComponent
```

### 계층 설명
- **SQL Database**: 실제 MySQL/MariaDB 데이터베이스
- **DatabaseDTO**: 데이터베이스와 일대일 매핑되는 구조체 (FSkillSlotDatabaseDTO, FSkillMasterDatabaseDTO)
- **SkillDataAsset(VO)**: UE5 콘텐츠 시스템의 Value Object - **원본 구조 유지 필수**
- **DomainModel**: 비즈니스 로직이 포함된 도메인 모델 (FSkillDomainModel)
- **SkillSlot(Entity)**: 실제 게임 오브젝트 (USkillSlot)

## 🔧 매퍼 인터페이스

### 1. ISkillDtoMapper
**목적**: SQL ↔ DatabaseDTO 변환
```cpp
// SQL 결과 → DatabaseDTO
FSkillSlotDatabaseDTO slotDTO = dtoMapper->MapSlotFromSqlResult(sqlRow);

// DatabaseDTO → SQL 파라미터
TMap<FString, FString> params = dtoMapper->MapSlotToSqlParams(slotDTO);

// 검증
FString errorMsg;
bool isValid = dtoMapper->ValidateSlotDTO(slotDTO, errorMsg);
```

### 2. ISkillAssetMapper
**목적**: DatabaseDTO ↔ SkillDataAsset(VO) 변환
**주의**: USkillDataAsset은 VO이므로 기존 구조를 훼손하지 않고 필요한 부분만 동기화
```cpp
// DatabaseDTO → SkillDataAsset (VO 무결성 유지)
USkillDataAsset* dataAsset = assetMapper->MapDtoToDataAsset(databaseDTO);

// SkillDataAsset → DatabaseDTO (필요한 필드만 추출)
FSkillMasterDatabaseDTO dto = assetMapper->MapDataAssetToDto(skillDataAsset);

// DataAsset 캐싱 및 로딩
USkillDataAsset* dataAsset = assetMapper->LoadOrCreateDataAsset(skillId);
assetMapper->CacheDataAsset(skillId, dataAsset);
```

### 3. ISkillModelBuilder
**목적**: SkillDataAsset → DomainModel 빌딩
```cpp
// 도메인 모델 생성 (DatabaseDTO + SkillDataAsset)
FSkillDomainModel domainModel = modelBuilder->BuildDomainModel(slotDTO, skillDataAsset);

// 엔티티 생성
USkillSlot* skillSlot = modelBuilder->BuildSkillSlotEntity(domainModel);

// 비즈니스 로직 계산
float remainingCooldown = modelBuilder->CalculateRemainingCooldown(lastUsedTime, baseCooldown);
bool canUse = modelBuilder->CanUseSkill(lastUsedTime, baseCooldown, currentMana, requiredMana);
```

## 💡 올바른 데이터 흐름

### 1. 스킬 슬롯 로딩 (Database → Entity)

```cpp
// 1. DatabaseManager에서 SQL → DatabaseDTO
TArray<FSkillSlotDatabaseDTO> slotDTOs = await DatabaseManager->LoadUserSkillSlots(userId, "ActionBar");
TArray<FSkillMasterDatabaseDTO> masterDTOs = await DatabaseManager->LoadSkillMasterData({});

// 2. AssetMapper로 DatabaseDTO → SkillDataAsset (VO 유지)
TArray<USkillDataAsset*> skillDataAssets = assetMapper->MapDtosToDataAssets(masterDTOs);

// 3. ModelBuilder로 DomainModel 생성 (DatabaseDTO + SkillDataAsset)
TArray<FSkillDomainModel> domainModels;
for (const FSkillSlotDatabaseDTO& slotDTO : slotDTOs)
{
    USkillDataAsset* matchingAsset = FindMatchingAsset(slotDTO.SkillId, skillDataAssets);
    if (matchingAsset)
    {
        FSkillDomainModel domainModel = modelBuilder->BuildDomainModel(slotDTO, matchingAsset);
        domainModels.Add(domainModel);
    }
}

// 4. ModelBuilder로 Entity 생성
TArray<USkillSlot*> skillSlots = modelBuilder->BuildSkillSlotEntities(domainModels);

// 5. SkillComponent에서 관리
skillComponent->Server_SetSkillSlots(skillSlots);
```

### 2. 스킬 슬롯 저장 (Entity → Database)

```cpp
// 1. SkillComponent에서 Entity → DatabaseDTO 추출
TArray<FSkillSlotDatabaseDTO> slotDTOs = skillComponent->ExtractDTOsFromSkillSlots(userId);

// 2. DatabaseManager로 저장 (DatabaseDTO → SQL)
bool success = await DatabaseManager->SaveUserSkillSlots(slotDTOs);
```

### 3. 스킬 등록 (VO 무결성 유지)

```cpp
// 1. 기존 SkillDataAsset(VO) 로드 (에디터에서 설정된 완전한 구조)
USkillDataAsset* skillDataAsset = assetMapper->LoadOrCreateDataAsset(skillId);

// 2. 도메인 규칙 검증
if (!skillComponent->CanRegisterSkill(slotIndex, skillDataAsset))
{
    return false;
}

// 3. 스킬 등록 (VO는 원본 유지)
bool success = skillComponent->RegisterSkill(slotIndex, skillDataAsset);

// 4. 데이터베이스 동기화 (필요한 경우)
FSkillSlotDatabaseDTO slotDTO = CreateSlotDTO(userId, slotKey, skillId, slotIndex);
await DatabaseManager->SaveUserSkillSlots({slotDTO});
```

## ⚠️ 중요한 설계 원칙

### 1. Value Object (VO) 무결성 유지
```cpp
// ❌ 잘못된 방법: VO 구조 훼손
skillDataAsset->GEClass = nullptr;  // 기존 GameplayEffect 클래스 제거
skillDataAsset->AbilityClass = nullptr;  // 기존 Ability 클래스 제거

// ✅ 올바른 방법: VO 구조 유지하면서 필요한 부분만 동기화
void USkillAssetMapper::SyncDtoToDataAsset(const FSkillMasterDatabaseDTO& DTO, USkillDataAsset* DataAsset)
{
    // 데이터베이스 필드만 동기화, VO의 다른 필드들(GEClass, AbilityClass 등)은 보존
    DataAsset->SkillID = DTO.SkillId;
    DataAsset->CoolTime = DTO.BaseCooltime;
    DataAsset->CostAmount = DTO.BaseCost;
    // GEClass, AbilityClass, TargetStrategyClass 등은 건드리지 않음
}
```

### 2. 계층별 책임 분리
```cpp
// DatabaseDTO: SQL 스키마 반영
struct FSkillSlotDatabaseDTO {
    int32 UserId;      // user_skill_slots.user_id
    FString SlotKey;   // user_skill_slots.slot_key
    int32 SkillId;     // user_skill_slots.skill_id
    int32 SlotIndex;   // user_skill_slots.slot_index
    // ... SQL 필드만
};

// SkillDataAsset: UE5 VO (에디터 설정 유지)
class USkillDataAsset {
    int32 SkillID;                          // 기본 정보
    TSubclassOf<UGameplayAbility> AbilityClass;  // UE5 전용
    TSubclassOf<UGameplayEffect> GEClass;        // UE5 전용
    UAnimMontage* CastMontage;                   // UE5 전용
    // ... 에디터에서 설정된 모든 필드 유지
};

// DomainModel: 비즈니스 로직
struct FSkillDomainModel {
    // DatabaseDTO + SkillDataAsset의 조합
    float RemainingCooldown;  // 계산된 값
    bool bCanUse;            // 비즈니스 규칙 적용
    // ...
};
```

### 3. 매퍼 체인 검증
```cpp
// 각 단계에서 검증 수행
FString errorMsg;

// 1. DatabaseDTO 검증
if (!dtoMapper->ValidateSlotDTO(slotDTO, errorMsg)) {
    UE_LOG(LogTemp, Error, TEXT("DatabaseDTO Validation Failed: %s"), *errorMsg);
    return false;
}

// 2. SkillDataAsset(VO) 검증
if (!assetMapper->ValidateDataAsset(skillDataAsset, errorMsg)) {
    UE_LOG(LogTemp, Error, TEXT("SkillDataAsset Validation Failed: %s"), *errorMsg);
    return false;
}

// 3. DomainModel 검증
if (!modelBuilder->ValidateDomainModel(domainModel, errorMsg)) {
    UE_LOG(LogTemp, Error, TEXT("DomainModel Validation Failed: %s"), *errorMsg);
    return false;
}
```

## 🔒 도메인 규칙 (변경 없음)

### 1. SkillComponent Aggregate Rules
- 슬롯 인덱스는 0 이상 MaxSkillSlots 미만이어야 함
- 동일한 스킬은 중복 등록할 수 없음
- 쿨다운 중인 스킬은 사용할 수 없음
- 마나가 부족하면 스킬을 사용할 수 없음

### 2. 데이터 무결성 규칙
- UserId는 1 이상이어야 함
- SlotKey는 비워둘 수 없음
- SkillId는 0 이상이어야 함 (0은 빈 슬롯)
- SkillLevel은 1 이상이어야 함

## � 마이그레이션 요약

### 변경 사항
1. **FSkillSlotDTO_V2** → **FSkillSlotDatabaseDTO** (네임스페이스 규칙)
2. **FSkillMasterDTO** → **FSkillMasterDatabaseDTO** (네임스페이스 규칙)  
3. **FSkillAssetData 제거** → **USkillDataAsset 직접 사용** (VO 무결성)
4. **SkillSystem** → **SkillModule** (모듈명 수정)

### 핵심 보장사항
- ✅ **SQL 스키마 완전 호환**: DatabaseDTO가 테이블 구조와 1:1 매칭
- ✅ **VO 무결성 유지**: USkillDataAsset의 모든 필드 보존
- ✅ **비즈니스 로직 분리**: DomainModel에서 계산 로직 처리
- ✅ **확장 가능성**: 각 계층이 독립적으로 변경 가능

이 가이드를 참고하여 **데이터 무결성과 VO 구조를 보장하는** 3계층 매핑 아키텍처를 활용하시기 바랍니다!

## 📊 DDD 매핑 아키텍처 구현 결과 보고서

### 🔄 작업 흐름 (Implementation Flow)

**Phase 1: 타입 시스템 마이그레이션**
```
FGuid SlotId → int32 SlotIndex
- 20개 이상의 파일에서 일관된 타입 변경
- SQL 호환성을 위한 데이터베이스 친화적 식별자 도입
- SkillComponent, SkillDomainService, SkillRepository 전면 개편
```

**Phase 2: 3계층 매핑 아키텍처 구축**
```
SQL Database ↔ DatabaseDTO ↔ SkillDataAsset(VO) ↔ DomainModel ↔ Entity
     ↑              ↑              ↑                 ↑              ↑
DatabaseManager   DtoMapper    AssetMapper    ModelBuilder   SkillComponent
```

**Phase 3: 인터페이스 기반 매퍼 시스템**
- `ISkillDtoMapper`: SQL ↔ DatabaseDTO 변환
- `ISkillAssetMapper`: DatabaseDTO ↔ SkillDataAsset 변환
- `ISkillModelBuilder`: DataAsset + 비즈니스 로직 → DomainModel

**Phase 4: DDD Aggregate Root 패턴 적용**
- SkillComponent를 Aggregate Root로 재설계
- 도메인 이벤트 시스템 구축
- 불변 조건 보장 메커니즘 구현

### 🏗️ 핵심 구현 결과

#### **1. 새로운 데이터베이스 DTO 구조**
```cpp
// SQL 스키마와 1:1 매핑
struct FSkillSlotDatabaseDTO {
    int32 UserId;           // user_skill_slots.user_id
    FString SlotKey;        // user_skill_slots.slot_key (Q, W, E, R)
    int32 SkillId;          // user_skill_slots.skill_id
    int32 SlotIndex;        // user_skill_slots.slot_index
    int32 SkillLevel;       // user_skills.skill_level
    FDateTime LastUsedTime; // user_skill_slots.last_used_time
};

struct FSkillMasterDatabaseDTO {
    int32 SkillId;          // skills.skill_id
    FString DisplayName;    // skills.display_name
    float BaseCooltime;     // skills.base_cooltime
    float BaseCost;         // skills.base_cost
    int32 MaxLevel;         // skills.max_level
};
```

#### **2. Aggregate Root 패턴 구현**
```cpp
// SkillComponent: DDD Aggregate Root
class USkillComponent {
    // 도메인 이벤트
    FOnSkillRegistered OnSkillRegistered;
    FOnSkillUnregistered OnSkillUnregistered;
    FOnSkillsSwapped OnSkillsSwapped;
    
    // 불변 조건 보장
    bool RegisterSkill(int32 SlotIndex, USkillDataAsset* SkillData) {
        if (!CanRegisterSkill(SlotIndex, SkillData)) return false;
        
        // 상태 변경
        TargetSlot->SetSkillData(SkillData, SkillData->SkillID);
        
        // 도메인 이벤트 발행
        OnSkillRegistered.Broadcast(SlotIndex);
        return true;
    }
};
```

#### **3. 비즈니스 로직 분리 - ModelBuilder**
```cpp
// 도메인 모델 빌딩 + 비즈니스 계산
FSkillDomainModel USkillModelBuilder::BuildDomainModel(
    const FSkillSlotDatabaseDTO& DatabaseDTO, 
    USkillDataAsset* SkillAsset)
{
    FSkillDomainModel DomainModel;
    
    // 1. Database 데이터
    DomainModel.SlotIndex = DatabaseDTO.SlotIndex;
    DomainModel.LastUsedTime = DatabaseDTO.LastUsedTime;
    
    // 2. SkillDataAsset 정보 (VO 보존)
    DomainModel.BaseCooldownTime = SkillAsset->CoolTime;
    DomainModel.ManaCost = SkillAsset->CostAmount;
    
    // 3. 비즈니스 로직 계산
    DomainModel.ActualDamage = CalculateScaledValue(
        SkillAsset->BaseDamage, DatabaseDTO.SkillLevel);
    DomainModel.RemainingCooldown = CalculateRemainingCooldown(
        DatabaseDTO.LastUsedTime, SkillAsset->CoolTime);
    DomainModel.bCanUse = CanUseSkill(
        DomainModel.LastUsedTime, DomainModel.BaseCooldownTime, 
        DomainModel.ManaCost, CurrentMana);
    
    return DomainModel;
}
```

#### **4. Value Object 무결성 보장**
```cpp
// AssetMapper: VO 구조 훼손 없이 동기화
void USkillAssetMapper::SyncDtoToDataAsset(
    const FSkillMasterDatabaseDTO& DTO, 
    USkillDataAsset* DataAsset)
{
    // 데이터베이스 필드만 업데이트
    DataAsset->SkillID = DTO.SkillId;
    DataAsset->CoolTime = DTO.BaseCooltime;
    DataAsset->CostAmount = DTO.BaseCost;
    
    // 💡 중요: UE5 전용 필드들은 보존
    // - GEClass (GameplayEffect)
    // - AbilityClass (GameplayAbility)  
    // - CastMontage (애니메이션)
    // - TargetStrategyClass 등
}
```

### 📈 실제 사용 예시

#### **스킬 로딩 전체 플로우**
```cpp
// 1. 데이터베이스에서 DTO 로드
TArray<FSkillSlotDatabaseDTO> slotDTOs = 
    await DatabaseManager->LoadUserSkillSlots(userId, "ActionBar");
TArray<FSkillMasterDatabaseDTO> masterDTOs = 
    await DatabaseManager->LoadSkillMasterData({});

// 2. DTO → DataAsset 변환 (VO 캐싱)
TArray<USkillDataAsset*> skillAssets = 
    assetMapper->MapDtosToDataAssets(masterDTOs);

// 3. 도메인 모델 구축 (비즈니스 로직 적용)
TArray<FSkillDomainModel> domainModels;
for (const auto& slotDTO : slotDTOs) {
    USkillDataAsset* asset = FindMatchingAsset(slotDTO.SkillId, skillAssets);
    if (asset) {
        auto domainModel = modelBuilder->BuildDomainModel(slotDTO, asset);
        domainModels.Add(domainModel);
    }
}

// 4. Entity 생성 및 Aggregate 업데이트
TArray<USkillSlot*> skillSlots = 
    modelBuilder->BuildSkillSlotEntities(domainModels);
skillComponent->Server_SetSkillSlots(skillSlots);
```

#### **스킬 등록 트랜잭션**
```cpp
// 도메인 서비스를 통한 원자적 트랜잭션
void USkillDomainService::RegisterSkillToPlayer(
    TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, 
    USkillDataAsset* SkillData)
{
    // 1. Aggregate에서 도메인 규칙 검증
    int32 slotIndex = skillComponent->GetAvailableSlotIndex();
    if (!skillComponent->CanRegisterSkill(slotIndex, SkillData)) {
        OnSkillOperationFailed.Broadcast(playerId, "Domain rule violation");
        return;
    }
    
    // 2. 낙관적 업데이트 (Aggregate)
    bool success = skillComponent->RegisterSkill(slotIndex, SkillData);
    
    // 3. 데이터베이스 동기화 (Repository)
    auto repositoryTask = skillRepository->RegisterSkillByPlayerId(playerId, newSlotDTO);
    
    // 4. 비동기 결과 처리 및 보상 트랜잭션
    UE::Tasks::Launch([=]() mutable {
        auto result = repositoryTask.GetResult();
        if (!result.bSuccess) {
            // 롤백: Aggregate 상태 되돌리기
            skillComponent->UnregisterSkill(slotIndex);
            OnSkillOperationFailed.Broadcast(playerId, result.ErrorMessage);
        } else {
            OnSkillOperationSucceeded.Broadcast(playerId, "Skill registered");
        }
    });
}
```

### 🎯 주요 성과 및 이점

#### **1. 데이터 무결성 보장**
- ✅ SQL 스키마와 완벽 호환되는 DatabaseDTO
- ✅ UE5 VO 구조 완전 보존 (GEClass, AbilityClass 등)
- ✅ 타입 안전성 (int32 SlotIndex로 일관성 유지)

#### **2. 확장성 및 유지보수성**
- ✅ 각 계층이 독립적으로 변경 가능
- ✅ 인터페이스 기반 의존성 주입
- ✅ 테스트 가능한 구조 (Mock 구현 용이)

#### **3. 성능 최적화**
- ✅ DataAsset 캐싱 시스템
- ✅ 배치 처리 지원 (TArray 기반 변환)
- ✅ 비동기 데이터베이스 작업

#### **4. DDD 원칙 준수**
- ✅ Aggregate Root 패턴 (SkillComponent)
- ✅ 도메인 이벤트 시스템
- ✅ 불변 조건 보장
- ✅ 비즈니스 로직과 인프라 분리

### 🔧 기술적 혁신 포인트

**매퍼 체인 패턴**
```
SQL → DtoMapper → DatabaseDTO → AssetMapper → SkillDataAsset → ModelBuilder → DomainModel → Entity
```

**이벤트 기반 아키텍처**
```cpp
// 도메인 이벤트 → UI 업데이트
skillComponent->OnSkillRegistered.AddUObject(this, &USkillUI::OnSkillAdded);
skillComponent->OnSkillsChanged.AddUObject(this, &USkillUI::RefreshUI);
```

**보상 트랜잭션 패턴**
```cpp
// 데이터베이스 실패 시 Aggregate 롤백
if (!databaseResult.bSuccess) {
    skillComponent->UnregisterSkill(slotIndex); // 보상 작업
}
```

이 구현을 통해 **완전한 엔터프라이즈급 3계층 DDD 매핑 아키텍처**를 구축하였으며, 데이터베이스 호환성, 도메인 로직 분리, UE5 에셋 시스템 보존을 모두 달성했습니다.
