# MyGame Database Schema Analysis & Documentation

## 1. 현재 데이터베이스 구조 분석

### 1.1 Character System
- **테이블**: `characters`
- **주요 필드**: PlayerID, CharacterName, Level, Experience, Position(X,Y,Z), Health, Mana
- **기능**: 캐릭터 기본 정보 저장 및 복원
- **트랜잭션 패턴**: 단일 플레이어 CRUD 작업

### 1.2 Inventory System  
- **테이블**: `inventory`
- **주요 필드**: PlayerID, ItemID, Quantity, SlotIndex
- **기능**: 플레이어별 아이템 소지 관리
- **트랜잭션 패턴**: 플레이어별 배치 작업, 아이템 추가/제거 원자적 처리

### 1.3 Skill System
- **테이블**: `skills`
- **주요 필드**: PlayerID, SkillID, SkillLevel, Cooldown, IsRegistered
- **기능**: 플레이어별 스킬 등록 및 쿨다운 관리
- **트랜잭션 패턴**: 스킬별 개별 작업, 쿨다운 업데이트

### 1.4 Shop System
- **테이블**: `shops`, `shop_items`
- **주요 필드**: ShopID, ShopName, ItemID, Stock, Price
- **기능**: 상점 관리 및 아이템 거래
- **트랜잭션 패턴**: 상점별 아이템 관리, 재고 및 가격 업데이트

## 2. 프로토타이핑을 위한 MySQL DDL 스키마

```sql
-- 캐릭터 기본 정보 테이블
CREATE TABLE characters (
    PlayerID VARCHAR(255) PRIMARY KEY,
    CharacterName VARCHAR(100) NOT NULL,
    Level INT DEFAULT 1,
    Experience BIGINT DEFAULT 0,
    PositionX FLOAT DEFAULT 0.0,
    PositionY FLOAT DEFAULT 0.0,
    PositionZ FLOAT DEFAULT 0.0,
    Health FLOAT DEFAULT 100.0,
    Mana FLOAT DEFAULT 100.0,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_character_name (CharacterName),
    INDEX idx_level (Level)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 인벤토리 테이블
CREATE TABLE inventory (
    InventoryID BIGINT AUTO_INCREMENT PRIMARY KEY,
    PlayerID VARCHAR(255) NOT NULL,
    ItemID VARCHAR(100) NOT NULL,
    Quantity INT DEFAULT 1,
    SlotIndex INT NOT NULL,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (PlayerID) REFERENCES characters(PlayerID) ON DELETE CASCADE,
    UNIQUE KEY unique_player_slot (PlayerID, SlotIndex),
    INDEX idx_player_item (PlayerID, ItemID),
    INDEX idx_item_id (ItemID)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 스킬 테이블
CREATE TABLE skills (
    SkillInstanceID BIGINT AUTO_INCREMENT PRIMARY KEY,
    PlayerID VARCHAR(255) NOT NULL,
    SkillID VARCHAR(100) NOT NULL,
    SkillLevel INT DEFAULT 1,
    Cooldown FLOAT DEFAULT 0.0,
    IsRegistered BOOLEAN DEFAULT FALSE,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (PlayerID) REFERENCES characters(PlayerID) ON DELETE CASCADE,
    UNIQUE KEY unique_player_skill (PlayerID, SkillID),
    INDEX idx_player_skills (PlayerID),
    INDEX idx_skill_id (SkillID),
    INDEX idx_registered_skills (PlayerID, IsRegistered)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 상점 테이블
CREATE TABLE shops (
    ShopID VARCHAR(255) PRIMARY KEY,
    ShopName VARCHAR(200) NOT NULL,
    ShopType VARCHAR(50) DEFAULT 'General',
    IsActive BOOLEAN DEFAULT TRUE,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_shop_name (ShopName),
    INDEX idx_shop_type (ShopType),
    INDEX idx_active_shops (IsActive)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 상점 아이템 테이블
CREATE TABLE shop_items (
    ShopItemID BIGINT AUTO_INCREMENT PRIMARY KEY,
    ShopID VARCHAR(255) NOT NULL,
    ItemID VARCHAR(100) NOT NULL,
    Stock INT DEFAULT 0,
    Price DECIMAL(15,2) DEFAULT 0.00,
    IsAvailable BOOLEAN DEFAULT TRUE,
    CreatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UpdatedAt TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (ShopID) REFERENCES shops(ShopID) ON DELETE CASCADE,
    UNIQUE KEY unique_shop_item (ShopID, ItemID),
    INDEX idx_shop_items (ShopID),
    INDEX idx_item_availability (ItemID, IsAvailable),
    INDEX idx_item_price (ItemID, Price)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
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
CREATE INDEX idx_shop_item_stock ON shop_items(ShopID, ItemID, Stock, IsAvailable);
CREATE INDEX idx_skill_cooldown ON skills(PlayerID, SkillID, Cooldown, IsRegistered);

-- 부분 인덱스를 통한 스토리지 최적화
CREATE INDEX idx_active_shop_items ON shop_items(ShopID, ItemID) WHERE IsAvailable = TRUE;
CREATE INDEX idx_registered_skills ON skills(PlayerID, SkillID) WHERE IsRegistered = TRUE;
```

### 3.3 파티셔닝 고려사항
```sql
-- 플레이어 기반 파티셔닝 (향후 확장성 고려)
ALTER TABLE inventory PARTITION BY HASH(CRC32(PlayerID)) PARTITIONS 8;
ALTER TABLE skills PARTITION BY HASH(CRC32(PlayerID)) PARTITIONS 8;
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

현재 MyGame의 데이터베이스 구조는 DDD 아키텍처에 적합한 형태로 구성되어 있으며, 각 도메인별로 명확한 책임 분리가 이루어져 있습니다. 제안된 최적화 방안들을 단계적으로 적용하면 성능과 확장성을 크게 향상시킬 수 있을 것입니다.

**핵심 권장사항:**
1. 연결 풀링 및 트랜잭션 최적화 우선 적용
2. 복합 인덱스를 통한 쿼리 성능 개선
3. 배치 처리를 통한 네트워크 최적화
4. 포괄적인 테스트 커버리지 확보
5. 모니터링 및 알람 시스템 구축

이러한 개선사항들을 통해 안정적이고 확장 가능한 게임 데이터베이스 시스템을 구축할 수 있습니다.
