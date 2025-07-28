// @Needmodifi
#pragma once

UENUM(BlueprintType)
enum class EBossState :uint8{
	Idle,
	Patrol,
	Chase,
	Stun,
	Dead
};