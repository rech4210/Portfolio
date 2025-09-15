// Fill out your copyright notice in the Description page of Project Settings.


#include "Provider/SkillDBProvider.h"
#include "GameSharedModule/Public/DTO/SkillDTOs.h"
#include "DatabaseManager.h"

// 3-layer architecture wrappers
UE::Tasks::TTask<TArray<FSkillSlotDatabaseDTO>> SkillDBProvider::LoadUserSkillSlots(const FString& UserId, const FString& SlotKey) {
	return DBManager->LoadUserSkillSlots(UserId, SlotKey);
}
UE::Tasks::TTask<bool> SkillDBProvider::SaveUserSkillSlots(const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs) {
	return DBManager->SaveUserSkillSlots(SkillSlotDTOs);
}
UE::Tasks::TTask<TArray<FSkillMasterDatabaseDTO>> SkillDBProvider::LoadSkillMasterData(const TArray<int32>& SkillIds) {
	return DBManager->LoadSkillMasterData(SkillIds);
}
UE::Tasks::TTask<bool> SkillDBProvider::SaveSkillMasterData(const TArray<FSkillMasterDatabaseDTO>& SkillMasterDTOs) {
	return DBManager->SaveSkillMasterData(SkillMasterDTOs);
}
UE::Tasks::TTask<bool> SkillDBProvider::UpdateSkillSlotCooldown(const FString& UserId, const FString& SlotKey, int32 SlotIndex, const FDateTime& LastUsedTime) {
	return DBManager->UpdateSkillSlotCooldown(UserId, SlotKey, SlotIndex, LastUsedTime);
}
UE::Tasks::TTask<bool> SkillDBProvider::ClearUserSkillSlots(const FString& UserId, const FString& SlotKey) {
	return DBManager->ClearUserSkillSlots(UserId, SlotKey);
}
UE::Tasks::TTask<TMap<int32, int32>> SkillDBProvider::GetSkillUsageStatistics(const FString& UserId, int32 SkillId, const FDateTime& StartDate, const FDateTime& EndDate) {
	return DBManager->GetSkillUsageStatistics(UserId, SkillId, StartDate, EndDate);
}
