# MyGame 데이터베이스 관리 가이드

이 문서는 MyGame 프로젝트의 데이터베이스 스키마 관리, 초기화, 마이그레이션 방법을 설명합니다.

## 1. 폴더 구조

-   **`Database/DDL/`**: 각 도메인 모듈별 `CREATE TABLE` 스크립트를 보관합니다. 폴더와 파일 이름의 숫자 접두사는 테이블 생성 순서를 의미합니다.
-   **`Database/Migrations/`**: `ALTER TABLE`과 같이 기존 테이블 구조를 변경하는 스크립트를 버전별로 관리합니다.
-   **`Database/Scripts/`**: DB 초기화, 테스트 데이터 삽입 등 개발 편의를 위한 유틸리티 스크립트를 보관합니다.

## 2. 데이터베이스 초기화 방법

새로운 개발 환경을 설정하거나 데이터베이스를 초기 상태로 되돌리고 싶을 때 사용합니다.

### Windows (`.bat`)

1.  `Database/Scripts/` 폴더로 이동합니다.
2.  `init_db.bat` 파일을 실행합니다.

이 스크립트는 다음 작업을 자동으로 수행합니다:
-   `DDL` 폴더의 모든 `.sql` 파일을 순서대로 `master_schema.sql` 파일 하나로 병합합니다.
-   `mysql` 클라이언트를 사용하여 `master_schema.sql`을 실행, 모든 테이블을 생성합니다.
-   임시로 생성된 `master_schema.sql` 파일을 삭제합니다.

**참고**: `init_db.bat` 파일 내의 사용자 정보(`your_user`, `your_password`, `your_db`)를 자신의 로컬 DB 환경에 맞게 수정해야 합니다.

### 수동 실행

```bash
# 1. DDL 파일들을 하나의 파일로 합치기 (PowerShell 예시)
Get-Content .\DDL\**\*.sql | Set-Content master_schema.sql

# 2. MySQL 클라이언트로 실행
mysql -u your_user -p your_password your_db < master_schema.sql
```

## 3. 스키마 변경 (마이그레이션)

프로젝트가 진행됨에 따라 테이블 구조를 변경해야 할 수 있습니다.

1.  `Database/Migrations/` 폴더에 `vX.X.X_description.sql` 형식으로 `ALTER TABLE` 스크립트 파일을 생성합니다.
2.  해당 스크립트를 개발 DB에 수동으로 실행합니다.
3.  변경 사항을 팀원들에게 알리고, 모든 팀원이 동일한 마이그레이션을 적용하도록 안내합니다.

## 4. `Schema_Temp.md`의 역할

-   이 파일은 더 이상 DDL의 원본 소스가 아닙니다.
-   **역할**: 데이터베이스의 전체적인 설계, 각 테이블과 컬럼의 존재 이유, 인덱스 전략, 최적화 방안 등 **"왜 이렇게 설계했는가?"**에 대한 내용을 기록하는 **종합 설계 및 분석 문서**입니다.
-   DDL을 수정할 때는 `Database/DDL/`의 개별 파일을 수정하고, `Schema_Temp.md`에는 해당 변경에 대한 설계 노트를 업데이트하는 것을 권장합니다.

## 5. JSON 직렬화/역직렬화 개선사항 (2025.07.15 업데이트)

### 새로 추가된 JSON 헬퍼 클래스

**`UDatabaseJsonHelper`** 클래스가 `DatabaseModule`에 추가되어 JSON 처리를 개선했습니다:

```cpp
// 인벤토리 아이템 확장 데이터 직렬화
TMap<FString, FString> ItemProperties;
ItemProperties.Add(TEXT("ItemType"), TEXT("Weapon"));
ItemProperties.Add(TEXT("Durability"), TEXT("100"));
FString JsonData = UDatabaseJsonHelper::SerializeInventoryItemData(ItemProperties);

// 캐릭터 위치, 체력, 마나 데이터 직렬화
FVector Position(100.0f, 200.0f, 0.0f);
FString CharacterData = UDatabaseJsonHelper::SerializeCharacterExtendedData(Position, 120.0f, 80.0f);
```

### 주요 개선사항

1. **인벤토리 SlotIndex 지원**: `FInventoryItemDTO`에 `SlotIndex` 필드 추가
2. **통일된 테이블명**: 모든 테이블과 컬럼이 `snake_case`로 변경
3. **타입 안전성**: JSON 직렬화/역직렬화에서 타입 검증 강화
4. **확장성**: 각 도메인별 JSON 구조를 독립적으로 관리

## 6. 테스트 데이터 삽입

개발 및 테스트를 위한 샘플 데이터를 삽입하려면:

```bash
# Database/Scripts 폴더로 이동
cd Database/Scripts

# 테스트 데이터 삽입 (데이터베이스 초기화 후 실행)
mysql -u your_user -p your_password your_db < insert_test_data.sql
```

**포함된 테스트 데이터:**
- 3개의 테스트 캐릭터 (Warrior, Mage, Archer)
- 각 캐릭터별 인벤토리 아이템 (무기, 포션, 방어구 등)
- 스킬 데이터 (각 클래스별 2개씩)
- 장비 데이터 (강화 레벨 포함)
- 3개의 테스트 상점과 상품 목록

## 7. 개발 워크플로우 연계

이 데이터베이스 구조는 `Development_Guidelines.md`의 워크플로우와 완전히 연계됩니다:

1. **Step 1**: C++ 도메인 모델 정의
2. **Step 2**: `Schema_Temp.md`에서 DDL 설계 확인
3. **Step 3**: `Database/DDL/` 폴더의 해당 도메인 SQL 파일 수정
4. **Step 4**: `init_db.bat` 실행으로 변경사항 적용
5. **Step 5**: `UDatabaseJsonHelper`를 사용한 JSON 처리 구현

### 다음 단계 권장사항

1. **Skills 시스템 구현**: `Database/DDL/02_Skill/` 스키마를 활용
2. **Equipment 시스템 구현**: `Database/DDL/03_Equipment/` 스키마를 활용  
3. **Shop 시스템 구현**: `Database/DDL/04_Shop/` 스키마를 활용
4. **JSON 성능 최적화**: 대용량 데이터 처리를 위한 배치 연산 추가
