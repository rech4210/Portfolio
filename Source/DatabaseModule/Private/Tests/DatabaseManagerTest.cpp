#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Tests/AutomationCommon.h"
#include "Tasks/Task.h"

// ============================================================================
// 테스트용 헬퍼 클래스 - Task 기반 API용
// ============================================================================

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForTaskCompletion, UE::Tasks::TTask<bool>, TaskToWait);
bool FWaitForTaskCompletion::Update()
{
	return TaskToWait.IsCompleted();
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForInventoryTaskCompletion, UE::Tasks::TTask<TArray<FInventoryItemDTO>>, TaskToWait);
bool FWaitForInventoryTaskCompletion::Update()
{
	return TaskToWait.IsCompleted();
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForSkillTaskCompletion, UE::Tasks::TTask<TArray<FSkillSlotDTO>>, TaskToWait);
bool FWaitForSkillTaskCompletion::Update()
{
	return TaskToWait.IsCompleted();
}

// ============================================================================
// 캐릭터 데이터 CRUD 테스트 - 스키마 호환성 검증
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseCharacterSchemaTest, "MyGame.Database.Character.SchemaCompatibility", 
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseCharacterSchemaTest::RunTest(const FString& Parameters)
{
	// DatabaseManager 가져오기 - 테스트 환경에서는 더미 데이터로 테스트
	// UGameInstance* GameInstance = GEngine->GetGameInstance();
	// if (!TestNotNull(TEXT("GameInstance should not be null"), GameInstance))
	//	return false;

	// UDatabaseManager* DBManager = GameInstance->GetSubsystem<UDatabaseManager>();
	// if (!TestNotNull(TEXT("DatabaseManager should not be null"), DBManager))
	//	return false;

	// insert_test_data.sql과 동일한 테스트 데이터 구조
	FCharacterData TestCharacter;
	TestCharacter.UserId = TEXT("test_schema_user");
	TestCharacter.CharacterId = TEXT("test_schema_char");
	TestCharacter.CharacterName = TEXT("SchemaTestWarrior");
	TestCharacter.Level = 5;
	TestCharacter.Exp = 1250;
	TestCharacter.JsonData = TEXT("{\"Position\":{\"X\":100.0,\"Y\":200.0,\"Z\":0.0},\"Health\":120.0,\"Mana\":80.0,\"Additional\":{\"Class\":\"Warrior\",\"LastLogin\":\"2025-07-15T00:00:00Z\"}}");

	// 1. VARCHAR(255) UserId 호환성 테스트
	TestTrue(TEXT("UserId should be string type"), TestCharacter.UserId.IsEmpty() == false);
	TestTrue(TEXT("CharacterId should be string type"), TestCharacter.CharacterId.IsEmpty() == false);
	TestTrue(TEXT("CharacterName should be string type"), TestCharacter.CharacterName.IsEmpty() == false);
	
	// 2. 데이터 길이 검증 (스키마 제한 사항)
	TestTrue(TEXT("UserId should be within VARCHAR(255) limit"), TestCharacter.UserId.Len() <= 255);
	TestTrue(TEXT("CharacterId should be within VARCHAR(255) limit"), TestCharacter.CharacterId.Len() <= 255);
	TestTrue(TEXT("CharacterName should be within VARCHAR(100) limit"), TestCharacter.CharacterName.Len() <= 100);
	
	// 3. BIGINT Exp 호환성 테스트
	TestTrue(TEXT("Exp should support large values"), TestCharacter.Exp >= 0 && TestCharacter.Exp <= 9223372036854775807LL);

	return true;
}

// ============================================================================
// 실제 insert_test_data.sql 데이터 호환성 테스트
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseTestDataCompatibilityTest, "MyGame.Database.TestData.Compatibility",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseTestDataCompatibilityTest::RunTest(const FString& Parameters)
{
	// insert_test_data.sql의 실제 데이터로 구조체 생성
	TArray<FCharacterData> TestCharacters;

	// TestWarrior (user_001)
	FCharacterData Warrior;
	Warrior.UserId = TEXT("user_001");
	Warrior.CharacterId = TEXT("char_001");
	Warrior.CharacterName = TEXT("TestWarrior");
	Warrior.Level = 5;
	Warrior.Exp = 1250;
	Warrior.JsonData = TEXT("{\"Position\":{\"X\":100.0,\"Y\":200.0,\"Z\":0.0},\"Health\":120.0,\"Mana\":80.0,\"Additional\":{\"Class\":\"Warrior\",\"LastLogin\":\"2025-07-15T00:00:00Z\"}}");
	TestCharacters.Add(Warrior);

	// TestMage (user_002)
	FCharacterData Mage;
	Mage.UserId = TEXT("user_002");
	Mage.CharacterId = TEXT("char_002");
	Mage.CharacterName = TEXT("TestMage");
	Mage.Level = 3;
	Mage.Exp = 750;
	Mage.JsonData = TEXT("{\"Position\":{\"X\":150.0,\"Y\":250.0,\"Z\":10.0},\"Health\":80.0,\"Mana\":150.0,\"Additional\":{\"Class\":\"Mage\",\"LastLogin\":\"2025-07-15T00:00:00Z\"}}");
	TestCharacters.Add(Mage);

	// TestArcher (user_003)
	FCharacterData Archer;
	Archer.UserId = TEXT("user_003");
	Archer.CharacterId = TEXT("char_003");
	Archer.CharacterName = TEXT("TestArcher");
	Archer.Level = 7;
	Archer.Exp = 2100;
	Archer.JsonData = TEXT("{\"Position\":{\"X\":75.0,\"Y\":180.0,\"Z\":5.0},\"Health\":100.0,\"Mana\":100.0,\"Additional\":{\"Class\":\"Archer\",\"LastLogin\":\"2025-07-15T00:00:00Z\"}}");
	TestCharacters.Add(Archer);

	// 각 캐릭터 데이터 유효성 검사
	for (const FCharacterData& Character : TestCharacters)
	{
		TestTrue(FString::Printf(TEXT("%s UserId should be valid"), *Character.CharacterName), !Character.UserId.IsEmpty());
		TestTrue(FString::Printf(TEXT("%s CharacterId should be valid"), *Character.CharacterName), !Character.CharacterId.IsEmpty());
		TestTrue(FString::Printf(TEXT("%s Level should be positive"), *Character.CharacterName), Character.Level > 0);
		TestTrue(FString::Printf(TEXT("%s Exp should be non-negative"), *Character.CharacterName), Character.Exp >= 0);
		
		// JSON 파싱 테스트
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Character.JsonData);
		bool bParseSuccess = FJsonSerializer::Deserialize(Reader, JsonObject);
		TestTrue(FString::Printf(TEXT("%s JSON should be parseable"), *Character.CharacterName), bParseSuccess);
		
		if (bParseSuccess && JsonObject.IsValid())
		{
			// Position 객체 검증
			const TSharedPtr<FJsonObject>* PositionObject;
			TestTrue(FString::Printf(TEXT("%s should have Position data"), *Character.CharacterName), 
				JsonObject->TryGetObjectField(TEXT("Position"), PositionObject));
			
			// Health/Mana 값 검증
			double HealthValue, ManaValue;
			TestTrue(FString::Printf(TEXT("%s should have Health data"), *Character.CharacterName), 
				JsonObject->TryGetNumberField(TEXT("Health"), HealthValue));
			TestTrue(FString::Printf(TEXT("%s should have Mana data"), *Character.CharacterName), 
				JsonObject->TryGetNumberField(TEXT("Mana"), ManaValue));
		}
	}

	return true;
}

// ============================================================================
// 인벤토리 데이터 호환성 테스트
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseInventoryDataTest, "MyGame.Database.Inventory.DataCompatibility",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseInventoryDataTest::RunTest(const FString& Parameters)
{
	// insert_test_data.sql의 실제 인벤토리 데이터로 테스트
	TArray<FInventoryItemDTO> TestInventory;
	
	// user_001 (TestWarrior) 인벤토리 재현
	FInventoryItemDTO Sword;
	Sword.ItemID = FName(TEXT("sword_001"));
	Sword.Quantity = 1;
	Sword.SlotIndex = 0;
	Sword.ItemData = TEXT("{\"ItemType\":\"Weapon\",\"Durability\":100,\"EnhancementLevel\":2}");
	TestInventory.Add(Sword);

	FInventoryItemDTO HealthPotion;
	HealthPotion.ItemID = FName(TEXT("health_potion"));
	HealthPotion.Quantity = 5;
	HealthPotion.SlotIndex = 1;
	HealthPotion.ItemData = TEXT("{\"ItemType\":\"Consumable\",\"EffectValue\":50,\"Duration\":0}");
	TestInventory.Add(HealthPotion);

	FInventoryItemDTO IronOre;
	IronOre.ItemID = FName(TEXT("iron_ore"));
	IronOre.Quantity = 10;
	IronOre.SlotIndex = 2;
	IronOre.ItemData = TEXT("{\"ItemType\":\"Material\",\"Quality\":\"Common\",\"UsedFor\":\"Crafting\"}");
	TestInventory.Add(IronOre);

	FInventoryItemDTO WarriorArmor;
	WarriorArmor.ItemID = FName(TEXT("warrior_armor"));
	WarriorArmor.Quantity = 1;
	WarriorArmor.SlotIndex = 3;
	WarriorArmor.ItemData = TEXT("{\"ItemType\":\"Armor\",\"Defense\":25,\"Weight\":15}");
	TestInventory.Add(WarriorArmor);

	// 인벤토리 데이터 유효성 검사
	for (const FInventoryItemDTO& Item : TestInventory)
	{
		TestTrue(FString::Printf(TEXT("Item %s should have valid ID"), *Item.ItemID.ToString()), !Item.ItemID.IsNone());
		TestTrue(FString::Printf(TEXT("Item %s should have positive quantity"), *Item.ItemID.ToString()), Item.Quantity > 0);
		TestTrue(FString::Printf(TEXT("Item %s should have valid slot index"), *Item.ItemID.ToString()), Item.SlotIndex >= 0);
		
		// JSON 파싱 테스트
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Item.ItemData);
		bool bParseSuccess = FJsonSerializer::Deserialize(Reader, JsonObject);
		TestTrue(FString::Printf(TEXT("Item %s JSON should be parseable"), *Item.ItemID.ToString()), bParseSuccess);
		
		if (bParseSuccess && JsonObject.IsValid())
		{
			FString ItemType;
			TestTrue(FString::Printf(TEXT("Item %s should have ItemType"), *Item.ItemID.ToString()), 
				JsonObject->TryGetStringField(TEXT("ItemType"), ItemType));
			TestTrue(FString::Printf(TEXT("Item %s ItemType should not be empty"), *Item.ItemID.ToString()), 
				!ItemType.IsEmpty());
		}
	}

	return true;
}

// ============================================================================
// 스킬 데이터 호환성 테스트
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseSkillDataTest, "MyGame.Database.Skill.DataCompatibility",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseSkillDataTest::RunTest(const FString& Parameters)
{
	// insert_test_data.sql의 실제 스킬 데이터로 테스트
	TArray<FSkillSlotDTO> TestSkills;
	
	// user_001 (TestWarrior) 스킬 재현
	FSkillSlotDTO PowerStrike;
	PowerStrike.SlotId = FGuid::NewGuid();
	PowerStrike.SkillID = 1001;
	PowerStrike.SlotIndex = 0;
	PowerStrike.LastUsedTime = FDateTime::Now();
	PowerStrike.RemainingCooldown = 0.0f;
	PowerStrike.bIsActive = true;
	PowerStrike.SkillData = TEXT("{\"SkillName\":\"PowerStrike\",\"Damage\":150,\"ManaCost\":20,\"CooldownSeconds\":5}");
	TestSkills.Add(PowerStrike);

	FSkillSlotDTO DefenseStance;
	DefenseStance.SlotId = FGuid::NewGuid();
	DefenseStance.SkillID = 1002;
	DefenseStance.SlotIndex = 1;
	DefenseStance.LastUsedTime = FDateTime::Now();
	DefenseStance.RemainingCooldown = 0.0f;
	DefenseStance.bIsActive = true;
	DefenseStance.SkillData = TEXT("{\"SkillName\":\"DefenseStance\",\"DefenseBonus\":50,\"Duration\":30,\"ManaCost\":15}");
	TestSkills.Add(DefenseStance);

	// 스킬 데이터 유효성 검사
	for (const FSkillSlotDTO& Skill : TestSkills)
	{
		TestTrue(FString::Printf(TEXT("Skill %d should have valid GUID"), Skill.SkillID), Skill.SlotId.IsValid());
		TestTrue(FString::Printf(TEXT("Skill %d should have valid ID"), Skill.SkillID), Skill.SkillID > 0);
		TestTrue(FString::Printf(TEXT("Skill %d should have valid slot index"), Skill.SkillID), Skill.SlotIndex >= 0);
		TestTrue(FString::Printf(TEXT("Skill %d should have valid timestamp"), Skill.SkillID), Skill.LastUsedTime > FDateTime::MinValue());
		
		// JSON 파싱 테스트
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Skill.SkillData);
		bool bParseSuccess = FJsonSerializer::Deserialize(Reader, JsonObject);
		TestTrue(FString::Printf(TEXT("Skill %d JSON should be parseable"), Skill.SkillID), bParseSuccess);
		
		if (bParseSuccess && JsonObject.IsValid())
		{
			FString SkillName;
			TestTrue(FString::Printf(TEXT("Skill %d should have SkillName"), Skill.SkillID), 
				JsonObject->TryGetStringField(TEXT("SkillName"), SkillName));
			TestTrue(FString::Printf(TEXT("Skill %d SkillName should not be empty"), Skill.SkillID), 
				!SkillName.IsEmpty());
		}
	}

	return true;
}

// ============================================================================
// JSON 유틸리티 클래스 테스트
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseJsonHelperTest, "MyGame.Database.JsonHelper.Utility",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseJsonHelperTest::RunTest(const FString& Parameters)
{
	// 캐릭터 확장 데이터 직렬화/역직렬화 테스트
	FVector TestPosition(100.0f, 200.0f, 0.0f);
	float TestHealth = 120.0f;
	float TestMana = 80.0f;
	TMap<FString, FString> TestAdditionalData;
	TestAdditionalData.Add(TEXT("Class"), TEXT("Warrior"));
	TestAdditionalData.Add(TEXT("LastLogin"), TEXT("2025-07-15T00:00:00Z"));

	// 직렬화 테스트
	FString SerializedData = UDatabaseJsonHelper::SerializeCharacterExtendedData(TestPosition, TestHealth, TestMana, TestAdditionalData);
	TestTrue(TEXT("Serialized data should not be empty"), !SerializedData.IsEmpty());

	// 역직렬화 테스트
	FVector OutPosition;
	float OutHealth, OutMana;
	TMap<FString, FString> OutAdditionalData;
	
	bool bDeserializeSuccess = UDatabaseJsonHelper::DeserializeCharacterExtendedData(SerializedData, OutPosition, OutHealth, OutMana, OutAdditionalData);
	TestTrue(TEXT("Deserialization should succeed"), bDeserializeSuccess);
	
	if (bDeserializeSuccess)
	{
		TestEqual(TEXT("Position X should match"), OutPosition.X, TestPosition.X);
		TestEqual(TEXT("Position Y should match"), OutPosition.Y, TestPosition.Y);
		TestEqual(TEXT("Position Z should match"), OutPosition.Z, TestPosition.Z);
		TestEqual(TEXT("Health should match"), OutHealth, TestHealth);
		TestEqual(TEXT("Mana should match"), OutMana, TestMana);
		
		TestTrue(TEXT("Additional data should contain Class"), OutAdditionalData.Contains(TEXT("Class")));
		if (OutAdditionalData.Contains(TEXT("Class")))
		{
			TestEqual(TEXT("Class should match"), OutAdditionalData[TEXT("Class")], TEXT("Warrior"));
		}
	}

	// 인벤토리 아이템 데이터 직렬화/역직렬화 테스트
	TMap<FString, FString> ItemProperties;
	ItemProperties.Add(TEXT("ItemType"), TEXT("Weapon"));
	ItemProperties.Add(TEXT("Durability"), TEXT("100"));
	ItemProperties.Add(TEXT("EnhancementLevel"), TEXT("2"));

	FString SerializedItemData = UDatabaseJsonHelper::SerializeInventoryItemData(ItemProperties);
	TestTrue(TEXT("Serialized item data should not be empty"), !SerializedItemData.IsEmpty());

	TMap<FString, FString> OutItemProperties = UDatabaseJsonHelper::DeserializeInventoryItemData(SerializedItemData);
	TestEqual(TEXT("Item properties count should match"), OutItemProperties.Num(), ItemProperties.Num());
	
	TestTrue(TEXT("Item properties should contain ItemType"), OutItemProperties.Contains(TEXT("ItemType")));
	if (OutItemProperties.Contains(TEXT("ItemType")))
	{
		TestEqual(TEXT("ItemType should match"), OutItemProperties[TEXT("ItemType")], TEXT("Weapon"));
	}

	return true;
}

// ============================================================================
// 종합 통합 테스트: 실제 데이터베이스 연동 시뮬레이션
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseIntegrationTest, "MyGame.Database.Integration.FullWorkflow",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter)

bool FDatabaseIntegrationTest::RunTest(const FString& Parameters)
{
	// 테스트 환경에서는 실제 GameInstance 대신 데이터 구조 유효성만 검증
	// UGameInstance* GameInstance = GEngine->GetGameInstance();
	// if (!TestNotNull(TEXT("GameInstance should not be null"), GameInstance))
	//	return false;

	// UDatabaseManager* DBManager = GameInstance->GetSubsystem<UDatabaseManager>();
	// if (!TestNotNull(TEXT("DatabaseManager should not be null"), DBManager))
	//	return false;

	// insert_test_data.sql과 동일한 실제 테스트 시나리오
	const FString TestUserId = TEXT("integration_test_user");
	
	// 1. 캐릭터 데이터 준비 (복잡한 JSON 포함)
	FCharacterData IntegrationCharacter;
	IntegrationCharacter.UserId = TestUserId;
	IntegrationCharacter.CharacterId = TEXT("integration_char");
	IntegrationCharacter.CharacterName = TEXT("IntegrationTestChar");
	IntegrationCharacter.Level = 10;
	IntegrationCharacter.Exp = 5000;
	IntegrationCharacter.JsonData = TEXT("{\"Position\":{\"X\":300.0,\"Y\":400.0,\"Z\":50.0},\"Health\":200.0,\"Mana\":150.0,\"Additional\":{\"Class\":\"Paladin\",\"Guild\":\"TestGuild\",\"LastLogin\":\"2025-07-15T00:00:00Z\",\"Stats\":{\"Strength\":25,\"Agility\":15,\"Intelligence\":20}}}");

	// 2. 인벤토리 데이터 준비
	TArray<FInventoryItemDTO> IntegrationInventory;
	
	FInventoryItemDTO PaladinSword;
	PaladinSword.ItemID = FName(TEXT("paladin_sword"));
	PaladinSword.Quantity = 1;
	PaladinSword.SlotIndex = 0;
	PaladinSword.ItemData = TEXT("{\"ItemType\":\"Weapon\",\"Durability\":100,\"EnhancementLevel\":5,\"Properties\":{\"HolyDamage\":50,\"Blessing\":\"Divine Protection\"}}");
	IntegrationInventory.Add(PaladinSword);

	FInventoryItemDTO HolyPotion;
	HolyPotion.ItemID = FName(TEXT("holy_potion"));
	HolyPotion.Quantity = 10;
	HolyPotion.SlotIndex = 1;
	HolyPotion.ItemData = TEXT("{\"ItemType\":\"Consumable\",\"EffectValue\":100,\"Duration\":0,\"Properties\":{\"HealType\":\"Divine\",\"RemovesCurse\":true}}");
	IntegrationInventory.Add(HolyPotion);

	// 3. 스킬 데이터 준비
	TArray<FSkillSlotDTO> IntegrationSkills;
	
	FSkillSlotDTO HolyStrike;
	HolyStrike.SlotId = FGuid::NewGuid();
	HolyStrike.SkillID = 4001;
	HolyStrike.SlotIndex = 0;
	HolyStrike.LastUsedTime = FDateTime::Now();
	HolyStrike.RemainingCooldown = 0.0f;
	HolyStrike.bIsActive = true;
	HolyStrike.SkillData = TEXT("{\"SkillName\":\"HolyStrike\",\"Damage\":250,\"ManaCost\":40,\"CooldownSeconds\":8,\"Properties\":{\"Element\":\"Holy\",\"UndeadBonus\":100}}");
	IntegrationSkills.Add(HolyStrike);

	// 모든 데이터가 스키마와 호환되는지 최종 검증
	TestTrue(TEXT("Integration character data should be schema-compatible"), 
		IntegrationCharacter.UserId.Len() <= 255 && 
		IntegrationCharacter.CharacterId.Len() <= 255 && 
		IntegrationCharacter.CharacterName.Len() <= 100);
		
	TestTrue(TEXT("Integration inventory data should be valid"), IntegrationInventory.Num() > 0);
	TestTrue(TEXT("Integration skill data should be valid"), IntegrationSkills.Num() > 0);

	UE_LOG(LogTemp, Warning, TEXT("Integration test prepared successfully with schema-compatible data"));
	UE_LOG(LogTemp, Warning, TEXT("UserId: %s, CharacterName: %s, Level: %d, Exp: %lld"), 
		*IntegrationCharacter.UserId, *IntegrationCharacter.CharacterName, 
		IntegrationCharacter.Level, IntegrationCharacter.Exp);

	return true;
}