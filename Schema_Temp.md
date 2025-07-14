# MyGame Database Schema Analysis & Documentation

## 1. 현재 데이터베이스 구조 분석

### 1.1 Character System
- **테이블**: `characters`
- **주요 필드**: PlayerID, CharacterName, Level, Experience, Position(X,Y,Z), Health, Mana
- **기능**: 캐릭터 기본 정보 저장 및 복원
- **트랜잭션 패턴**: 단일 플레이어 CRUD 작업

### 1.2 Inventory System
- **테이블**: `inventory`
- **주요 필드**: PlayerID, ItemID, Quantity, SlotIndex, ItemData (JSON)
- **기능**: 플레이어별 아이템 소지 및 고유 속성 관리
- **트랜잭션 패턴**: 플레이어별 배치 작업, 아이템 추가/제거 원자적 처리

### 1.3 Skill System
- **테이블**: `skills`
- **주요 필드**: PlayerID, SlotId (GUID), SkillID, SkillLevel, Cooldown, LastUsedTime, SkillData (JSON)
- **기능**: 플레이어별 스킬 등록, 쿨다운, 커스텀 데이터 관리
- **트랜잭션 패턴**: 스킬별 개별 작업, 쿨다운 업데이트

### 1.4 Equipment System
- **테이블**: `equipment`
- **주요 필드**: PlayerID, ItemID, SlotIndex, SlotType, EnhancementLevel (JSON)
- **기능**: 플레이어 장비 장착 상태 및 강화 정보 관리
- **트랜잭션 패턴**: 장비별 개별 장착/해제 작업

### 1.5 Shop System
- **테이블**: `shops`, `shop_items`
- **주요 필드**: ShopID, ShopName, ItemID, Stock, Price, Category
- **기능**: 상점 관리 및 아이템 거래
- **트랜잭션 패턴**: 상점별 아이템 관리, 재고 및 가격 업데이트

## 2. 프로토타이핑을 위한 MySQL DDL 스키마 (C++ 코드와 동기화 완료)

**⚠️ 중요: 2025.07.15 업데이트**
- DDL 스키마가 현재 C++ DatabaseManager 구현과 완전히 동기화되었습니다
- JSON 직렬화/역직렬화 헬퍼 클래스 `UDatabaseJsonHelper` 추가됨
- 인벤토리 시스템에 `slot_index` 지원 추가됨
- 모든 테이블명과 컬럼명이 snake_case로 통일됨

```sql
-- 캐릭터 기본 정보 테이블 (현재 C++ 구현에 맞춤)
CREATE TABLE characters (
    user_id VARCHAR(255) PRIMARY KEY,
    character_id VARCHAR(255) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON, -- 확장 가능한 캐릭터 데이터 (Position, Health, Mana 등)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_character_name (character_name),
    INDEX idx_level (level),
    INDEX idx_character_id (character_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 인벤토리 테이블 (현재 C++ 구현에 맞춤 + SlotIndex 지원 추가)
CREATE TABLE inventory (
    inventory_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id VARCHAR(100) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT NOT NULL, -- 인벤토리 슬롯 위치 (신규 추가)
    item_data JSON, -- FInventoryItemDTO::ItemData (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_slot (user_id, slot_index),
    INDEX idx_user_item (user_id, item_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 스킬 테이블 (향후 확장용 - 현재 C++ 구현에서는 미사용)
CREATE TABLE skills (
    skill_instance_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    slot_id VARCHAR(36) NOT NULL, -- FSkillSlotDTO::SlotId (GUID)
    skill_id INT NOT NULL, -- FSkillSlotDTO::SkillID
    slot_index INT NOT NULL, -- FSkillSlotDTO::SlotIndex
    last_used_time TIMESTAMP NULL, -- FSkillSlotDTO::LastUsedTime
    remaining_cooldown FLOAT DEFAULT 0.0, -- FSkillSlotDTO::RemainingCooldown
    is_active BOOLEAN DEFAULT TRUE, -- FSkillSlotDTO::bIsActive
    skill_data JSON, -- FSkillSlotDTO::SkillData (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_skill_slot (user_id, slot_id),
    INDEX idx_user_slot_index (user_id, slot_index),
    INDEX idx_skill_cooldown (skill_id, remaining_cooldown)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 장비 테이블 (향후 확장용 - 현재 C++ 구현에서는 미사용)
CREATE TABLE equipment (
    equipment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id INT NOT NULL, -- FEquipmentSlotState::ItemID
    slot_index INT NOT NULL, -- FEquipmentSlotState::SlotIndex
    slot_type VARCHAR(50), -- FEquipmentSlotState::SlotType
    is_equipped BOOLEAN DEFAULT FALSE, -- FEquipmentSlotState::bIsEquipped
    enhancement_level JSON, -- FEquipmentSlotState::EnhancementLevel (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_equipment_slot (user_id, slot_index),
    INDEX idx_user_equipment_type (user_id, slot_type),
    INDEX idx_equipped_items (user_id, is_equipped)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 상점 테이블 (향후 확장용)
CREATE TABLE shops (
    shop_id INT PRIMARY KEY, -- FShopDomain::ShopID
    shop_name VARCHAR(200) NOT NULL, -- FShopDomain::ShopName
    shop_description TEXT, -- FShopDomain::ShopDescription
    is_open BOOLEAN DEFAULT TRUE, -- FShopDomain::bIsOpen
    area_id INT, -- FShopDomain::AreaID
    shop_location_x FLOAT, -- FShopDomain::ShopLocation
    shop_location_y FLOAT,
    shop_location_z FLOAT,
    last_restock_time TIMESTAMP NULL, -- FShopDomain::LastRestockTime
    global_price_modifier FLOAT DEFAULT 1.0, -- FShopDomain::GlobalPriceModifier
    shop_owner_name VARCHAR(100), -- FShopDomain::ShopOwnerName
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_shop_name (shop_name),
    INDEX idx_area_id (area_id),
    INDEX idx_is_open (is_open)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 상점 아이템 테이블 (향후 확장용)
CREATE TABLE shop_items (
    shop_item_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    shop_id INT NOT NULL,
    item_id INT NOT NULL, -- FShopItemDTO::ItemID
    item_name VARCHAR(200), -- FShopItemDTO::ItemName
    item_description TEXT, -- FShopItemDTO::ItemDescription
    price FLOAT DEFAULT 0.0, -- FShopItemDTO::Price
    stock INT DEFAULT 0, -- FShopItemDTO::Stock
    is_available BOOLEAN DEFAULT TRUE, -- FShopItemDTO::bIsAvailable
    category VARCHAR(100), -- FShopItemDTO::Category
    max_stock INT, -- FShopItemDTO::MaxStock
    restock_interval_hours FLOAT, -- FShopItemDTO::RestockIntervalHours
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (shop_id) REFERENCES shops(shop_id) ON DELETE CASCADE,
    UNIQUE KEY unique_shop_item (shop_id, item_id),
    INDEX idx_item_category (category),
    INDEX idx_item_price (item_id, price),
    INDEX idx_availability (is_available, stock)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

### JSON 데이터 구조 예시

**Characters 테이블 json_data 예시:**
```json
{
  "Position": {"X": 100.0, "Y": 200.0, "Z": 0.0},
  "Health": 120.0,
  "Mana": 80.0,
  "Additional": {
    "Class": "Warrior",
    "LastLogin": "2025-07-15T00:00:00Z"
  }
}
```

**Inventory 테이블 item_data 예시:**
```json
{
  "ItemType": "Weapon",
  "Durability": 100,
  "EnhancementLevel": 2,
  "CustomProperties": {"Rarity": "Epic", "BindType": "PickupBind"}
}
```

**Skills 테이블 skill_data 예시:**
```json
{
  "SkillName": "PowerStrike",
  "Damage": 150,
  "ManaCost": 20,
  "CooldownSeconds": 5,
  "EffectType": "Instant"
}
```

**Equipment 테이블 enhancement_level 예시:**
```json
{
  "Level": 3,
  "Effects": ["Damage+15", "Crit+8", "Speed+5"],
  "EnhancementHistory": ["Basic", "Enhanced", "Rare"]
}
```

## 3. DDD 아키텍처에서의 데이터베이스 최적화 권장사항

### 3.1 트랜잭션 경계 최적화
```sql
-- 배치 인벤토리 업데이트를 위한 최적화된 프로시저
DELIMITER //
CREATE PROCEDURE UpdateInventoryBatch(
    IN p_PlayerID VARCHAR(255),
    IN p_InventoryData JSON
)
BEGIN
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        RESIGNAL;
    END;
    
    START TRANSACTION;
    
    -- 기존 인벤토리 삭제
    DELETE FROM inventory WHERE PlayerID = p_PlayerID;
    
    -- 새 인벤토리 데이터 삽입
    INSERT INTO inventory (PlayerID, ItemID, Quantity, SlotIndex)
    SELECT 
        p_PlayerID,
        JSON_UNQUOTE(JSON_EXTRACT(value, '$.ItemID')),
        JSON_EXTRACT(value, '$.Quantity'),
        JSON_EXTRACT(value, '$.SlotIndex')
    FROM JSON_TABLE(p_InventoryData, '$[*]' COLUMNS (
        value JSON PATH '$'
    )) AS jt;
    
    COMMIT;
END //
DELIMITER ;
```

### 3.2 인덱스 최적화 전략
```sql
-- 복합 인덱스를 통한 쿼리 성능 향상
CREATE INDEX idx_player_level_exp ON characters(PlayerID, Level, Experience);
CREATE INDEX idx_shop_item_stock ON shop_items(ShopID, ItemID, Stock, bIsAvailable);
CREATE INDEX idx_skill_cooldown ON skills(PlayerID, SkillID, RemainingCooldown, bIsActive);
CREATE INDEX idx_player_equipment ON equipment(PlayerID, SlotType, bIsEquipped);

-- 부분 인덱스를 통한 스토리지 최적화
CREATE INDEX idx_active_shop_items ON shop_items(ShopID, ItemID) WHERE bIsAvailable = TRUE;
CREATE INDEX idx_active_skills ON skills(PlayerID, SkillID) WHERE bIsActive = TRUE;
```

### 3.3 파티셔닝 고려사항
```sql
-- 플레이어 기반 파티셔닝 (향후 확장성 고려)
ALTER TABLE inventory PARTITION BY HASH(CRC32(PlayerID)) PARTITIONS 8;
ALTER TABLE skills PARTITION BY HASH(CRC32(PlayerID)) PARTITIONS 8;
ALTER TABLE equipment PARTITION BY HASH(CRC32(PlayerID)) PARTITIONS 8;
```

## 4. 데이터베이스 연결 및 설정 최적화

### 4.1 연결 풀 설정
```cpp
// DatabaseManager.cpp 최적화 권장사항
class FDatabaseConnectionPool {
private:
    static constexpr int32 MIN_CONNECTIONS = 5;
    static constexpr int32 MAX_CONNECTIONS = 20;
    static constexpr int32 CONNECTION_TIMEOUT = 30; // seconds
    
public:
    void InitializePool() {
        // Connection Pool 초기화
        Properties.setProperty("minPoolSize", std::to_string(MIN_CONNECTIONS));
        Properties.setProperty("maxPoolSize", std::to_string(MAX_CONNECTIONS));
        Properties.setProperty("connectionTimeoutInMs", std::to_string(CONNECTION_TIMEOUT * 1000));
        Properties.setProperty("autoReconnect", "true");
        Properties.setProperty("useUnicode", "true");
        Properties.setProperty("characterEncoding", "UTF-8");
    }
};
```

### 4.2 트랜잭션 타임아웃 설정
```cpp
// 트랜잭션 타임아웃 관리
class FTransactionGuard {
private:
    static constexpr int32 TRANSACTION_TIMEOUT = 10; // seconds
    
public:
    FTransactionGuard(sql::Connection* Con) : Connection(Con) {
        if (Connection) {
            Connection->setAutoCommit(false);
            // 트랜잭션 타임아웃 설정
            auto Stmt = Connection->createStatement();
            Stmt->execute("SET SESSION innodb_lock_wait_timeout = " + std::to_string(TRANSACTION_TIMEOUT));
            delete Stmt;
        }
    }
};
```

## 5. 보안 및 성능 모니터링

### 5.1 SQL 인젝션 방지
- 모든 쿼리에 Prepared Statement 사용 (현재 구현됨)
- 입력 값 검증 강화
- 권한 분리 (읽기/쓰기 사용자 계정 분리)

### 5.2 성능 모니터링 쿼리
```sql
-- 느린 쿼리 모니터링
SET GLOBAL slow_query_log = 'ON';
SET GLOBAL long_query_time = 2; -- 2초 이상 쿼리 로깅

-- 인덱스 사용률 확인
SELECT 
    OBJECT_SCHEMA,
    OBJECT_NAME,
    INDEX_NAME,
    COUNT_FETCH,
    COUNT_INSERT,
    COUNT_UPDATE,
    COUNT_DELETE
FROM performance_schema.table_io_waits_summary_by_index_usage
WHERE OBJECT_SCHEMA = 'mygame_db'
ORDER BY COUNT_FETCH DESC;

-- 테이블 잠금 모니터링
SELECT 
    r.trx_id waiting_trx_id,
    r.trx_mysql_thread_id waiting_thread,
    r.trx_query waiting_query,
    b.trx_id blocking_trx_id,
    b.trx_mysql_thread_id blocking_thread,
    b.trx_query blocking_query
FROM performance_schema.data_lock_waits w
JOIN information_schema.innodb_trx r ON r.trx_id = w.requesting_trx_id
JOIN information_schema.innodb_trx b ON b.trx_id = w.blocking_trx_id;
```

## 6. 테스팅 전략

### 6.1 단위 테스트 (Unit Tests)
```cpp
// DatabaseManager 단위 테스트 예시
TEST_F(DatabaseManagerTest, SaveAndLoadCharacterInfo) {
    // Given
    FCharacterDTO TestCharacter;
    TestCharacter.PlayerID = TEXT("TestPlayer123");
    TestCharacter.CharacterName = TEXT("TestCharacter");
    TestCharacter.Level = 10;
    TestCharacter.Experience = 5000;
    
    // When
    bool bSaveResult = DatabaseManager->SaveCharacterInfo(TestCharacter);
    FCharacterDTO LoadedCharacter = DatabaseManager->LoadCharacterInfo(TestCharacter.PlayerID);
    
    // Then
    EXPECT_TRUE(bSaveResult);
    EXPECT_EQ(LoadedCharacter.PlayerID, TestCharacter.PlayerID);
    EXPECT_EQ(LoadedCharacter.Level, TestCharacter.Level);
}

TEST_F(DatabaseManagerTest, InventoryTransactionRollback) {
    // Given
    TArray<FInventoryDTO> InvalidInventory; // 잘못된 데이터
    
    // When & Then
    EXPECT_THROW(
        DatabaseManager->SaveInventoryForPlayer(TEXT("TestPlayer"), InvalidInventory),
        sql::SQLException
    );
    
    // 롤백 확인
    auto LoadedInventory = DatabaseManager->LoadInventoryForPlayer(TEXT("TestPlayer"));
    EXPECT_TRUE(LoadedInventory.IsEmpty()); // 기존 데이터 유지 확인
}
```

### 6.2 통합 테스트 (Integration Tests)
```cpp
// DDD 시스템 통합 테스트
TEST_F(ShopSystemIntegrationTest, CompleteShopTransaction) {
    // Given
    auto PlayerState = CreateTestPlayerState();
    auto ShopSubsystem = GetShopSubsystem();
    
    // When - Use Case Orchestration 테스트
    auto BuyTask = ShopSubsystem->BuyItemUseCase(PlayerState, TEXT("Shop001"), TEXT("Item001"), 1);
    auto Result = BuyTask.GetResult();
    
    // Then
    EXPECT_TRUE(Result.bSuccess);
    
    // Domain 상태 검증
    auto PlayerInventory = PlayerState->GetInventoryComponent()->GetInventoryItems();
    EXPECT_TRUE(PlayerInventory.Contains(TEXT("Item001")));
    
    // Database 상태 검증
    auto DbInventory = DatabaseManager->LoadInventoryForPlayer(PlayerState->GetPlayerID());
    EXPECT_TRUE(DbInventory.ContainsByPredicate([](const FInventoryDTO& Item) {
        return Item.ItemID == TEXT("Item001");
    }));
}
```

### 6.3 성능 테스트
```cpp
// 대용량 데이터 처리 성능 테스트
TEST_F(DatabasePerformanceTest, BatchInventoryUpdate) {
    // Given
    TArray<FInventoryDTO> LargeInventory;
    for (int32 i = 0; i < 1000; ++i) {
        LargeInventory.Add(CreateTestInventoryItem(i));
    }
    
    // When
    auto StartTime = FDateTime::Now();
    bool bResult = DatabaseManager->SaveInventoryForPlayer(TEXT("TestPlayer"), LargeInventory);
    auto EndTime = FDateTime::Now();
    
    // Then
    EXPECT_TRUE(bResult);
    auto Duration = (EndTime - StartTime).GetTotalMilliseconds();
    EXPECT_LT(Duration, 1000.0); // 1초 이내 완료
}
```

## 7. 네트워크 최적화 고려사항

### 7.1 배치 처리 최적화
```cpp
// 네트워크 대역폭 최적화를 위한 배치 처리
class FDatabaseBatchProcessor {
public:
    void QueueInventoryUpdate(const FString& PlayerID, const TArray<FInventoryDTO>& Items) {
        InventoryUpdateQueue.Add(PlayerID, Items);
    }
    
    void ProcessBatch() {
        if (InventoryUpdateQueue.Num() > 0) {
            // 배치 단위로 데이터베이스 업데이트 실행
            for (auto& Entry : InventoryUpdateQueue) {
                DatabaseManager->SaveInventoryForPlayer(Entry.Key, Entry.Value);
            }
            InventoryUpdateQueue.Empty();
        }
    }
    
private:
    TMap<FString, TArray<FInventoryDTO>> InventoryUpdateQueue;
    static constexpr float BATCH_INTERVAL = 5.0f; // 5초마다 배치 처리
};
```

### 7.2 압축 및 직렬화
```cpp
// JSON 직렬화를 통한 네트워크 최적화
class FDataSerializer {
public:
    static FString SerializeInventory(const TArray<FInventoryDTO>& Inventory) {
        TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> JsonArray;
        
        for (const auto& Item : Inventory) {
            auto ItemJson = MakeShareable(new FJsonObject);
            ItemJson->SetStringField(TEXT("ItemID"), Item.ItemID);
            ItemJson->SetNumberField(TEXT("Quantity"), Item.Quantity);
            ItemJson->SetNumberField(TEXT("SlotIndex"), Item.SlotIndex);
            JsonArray.Add(MakeShareable(new FJsonValueObject(ItemJson)));
        }
        
        JsonObject->SetArrayField(TEXT("items"), JsonArray);
        
        FString OutputString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
        FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
        
        return OutputString;
    }
};
```

## 8. 확장성 및 유지보수성

### 8.1 데이터베이스 마이그레이션 전략
```sql
-- 버전 관리 테이블
CREATE TABLE schema_migrations (
    version VARCHAR(50) PRIMARY KEY,
    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    description TEXT
);

-- 마이그레이션 예시 (v1.1.0)
INSERT INTO schema_migrations (version, description) 
VALUES ('v1.1.0', 'Add shop system with items table');

-- 인덱스 추가 마이그레이션
ALTER TABLE inventory ADD INDEX idx_item_quantity (ItemID, Quantity);
ALTER TABLE skills ADD INDEX idx_skill_level (SkillID, SkillLevel);
```

### 8.2 모니터링 및 알람 설정
```sql
-- 데이터베이스 헬스 체크 뷰
CREATE VIEW db_health_check AS
SELECT 
    'connection_count' as metric,
    COUNT(*) as value
FROM information_schema.processlist
UNION ALL
SELECT 
    'avg_query_time' as metric,
    AVG(QUERY_TIME) as value
FROM performance_schema.events_statements_summary_by_digest
WHERE LAST_SEEN > DATE_SUB(NOW(), INTERVAL 1 HOUR);

-- 자동 정리 작업
CREATE EVENT cleanup_old_logs
ON SCHEDULE EVERY 1 DAY
DO
    DELETE FROM error_logs WHERE created_at < DATE_SUB(NOW(), INTERVAL 30 DAY);
```

### 8.3 백업 및 복구 전략
```bash
#!/bin/bash
# 자동 백업 스크립트
DB_NAME="mygame_db"
BACKUP_DIR="/backup/mysql"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# 전체 백업
mysqldump --single-transaction --routines --triggers $DB_NAME > $BACKUP_DIR/full_backup_$TIMESTAMP.sql

# 증분 백업 (바이너리 로그 기반)
mysqlbinlog --start-datetime="$(date -d '1 hour ago' '+%Y-%m-%d %H:%M:%S')" /var/log/mysql/mysql-bin.* > $BACKUP_DIR/incremental_backup_$TIMESTAMP.sql

# 오래된 백업 파일 정리 (7일 이상)
find $BACKUP_DIR -name "*.sql" -mtime +7 -delete
```

---

## 결론

현재 MyGame의 데이터베이스 구조는 DDD 아키텍처에 적합한 형태로 구성되어 있으며, 각 도메인별로 명확한 책임 분리가 이루어져 있습니다. **새롭게 추가된 장비 시스템을 포함하여 모든 DDL이 최신 C++ 코드와 동기화되었습니다.** 제안된 최적화 방안들을 단계적으로 적용하면 성능과 확장성을 크게 향상시킬 수 있을 것입니다.

**핵심 권장사항:**
1.  연결 풀링 및 트랜잭션 최적화 우선 적용
2.  **검증된 DDL 스키마를 기반으로 데이터베이스 마이그레이션 수행**
3.  복합 인덱스를 통한 쿼리 성능 개선
4.  배치 처리를 통한 네트워크 최적화
5.  포괄적인 테스트 커버리지 확보
6.  모니터링 및 알람 시스템 구축

이러한 개선사항들을 통해 안정적이고 확장 가능한 게임 데이터베이스 시스템을 구축할 수 있습니다.
