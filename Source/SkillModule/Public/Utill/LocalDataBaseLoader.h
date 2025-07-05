#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Data/SkillDataAsset.h"
#include "LocalDataBaseLoader.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillDataLoadedSignature, class UBaseDataAsset*, LoadedAsset);

/**
 * Static helper class for managing skill data assets globally
 */
UCLASS()
class SKILLMODULE_API ULocalDataBaseLoader : public UObject
{
    GENERATED_BODY()
public:
    static void Initialize();
    static bool IsInitialized() { return bIsInitialized; }
    static void CheckRegistedAsset();
    static bool CheckPrimaryAssetId(int32 SkillID, FPrimaryAssetId& OutId);
    
    /** Bind to this delegate to receive notifications when skill data is loaded */
    // UPROPERTY(BlueprintAssignable, Category = "Skill Data")
    static FOnSkillDataLoadedSignature OnSkillDataLoaded;
    /** Get data asset from AssetId. If bSync is true, blocks until loaded */
    template<typename T>
    static T* GetDataFromAssetId(const FPrimaryAssetId& AssetId, bool bSync = true, const FStreamableDelegate& OnLoaded = FStreamableDelegate())
    {
        if (!bIsInitialized)
        {
            UE_LOG(LogTemp, Warning, TEXT("LocalDataBaseLoader not initialized! Call Initialize() first."));
            return nullptr;
        }

        UAssetManager& Manager = UAssetManager::Get();

        // Try to get already loaded asset first
        if (UObject* Obj = Manager.GetPrimaryAssetObject(AssetId))
        {
            T* TypedObj = Cast<T>(Obj);
            if (TypedObj)
            {
                OnSkillDataLoaded.Broadcast(Cast<UBaseDataAsset>(TypedObj));
                return TypedObj;
            }
        }

        // Load the asset
        if (bSync)
        {
            TSharedPtr<FStreamableHandle> Handle = Manager.LoadPrimaryAsset(AssetId, {}, FStreamableDelegate(), 0);
            if (Handle.IsValid())
            {
                Handle->WaitUntilComplete();
                T* LoadedObj = Cast<T>(Manager.GetPrimaryAssetObject(AssetId));
                if (LoadedObj)
                {
                    OnSkillDataLoaded.Broadcast(Cast<UBaseDataAsset>(LoadedObj));
                }
                return LoadedObj;
            }
        }
        else
        {
            Manager.LoadPrimaryAsset(AssetId, {}, 
                FStreamableDelegate::CreateLambda([AssetId, OnLoaded]()
                {
                    if (T* LoadedObj = Cast<T>(UAssetManager::Get().GetPrimaryAssetObject(AssetId)))
                    {
                        OnSkillDataLoaded.Broadcast(Cast<UBaseDataAsset>(LoadedObj));
                        OnLoaded.ExecuteIfBound();
                    }
                }), 0);
        }

        return nullptr;
    }

private:
    static bool bIsInitialized;
	static TMap<int32, FPrimaryAssetId> SkillIdToAssetId;
};