#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/FCharacterData.h"
#include "DatabaseManager.generated.h"

// Delegate for async operations
DECLARE_DELEGATE_OneParam(FCharacterDataLoadDelegate, const TOptional<FCharacterData>& /* CharacterData */);
DECLARE_DELEGATE_OneParam(FCharacterDataSaveDelegate, bool /* bSuccess */);

// Forward declaration for the implementation class (PIMPL pattern)
struct FDatabaseManagerImpl;// 불완전한 형식

UCLASS()
class DATABASEMODULE_API UDatabaseManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDatabaseManager();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual ~UDatabaseManager();

	/**
	 * Asynchronously loads character information from the database.
	 * @param UserId The ID of the user whose character to load.
	 * @param Delegate The delegate to call upon completion.
	 */
	void LoadCharacterInfo(int32 UserId, FCharacterDataLoadDelegate Delegate);

	/**
	 * Asynchronously saves character information to the database.
	 * @param CharacterData The character data to save.
	 * @param Delegate The delegate to call upon completion.
	 */
	void SaveCharacterInfo(const FCharacterData& CharacterData, FCharacterDataSaveDelegate Delegate);

private:
	// Pointer to the implementation
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
}; 