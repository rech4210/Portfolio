#pragma once

#include "CoreMinimal.h"
#include "Shared/AI/EnemySystemCore/EObservedAttribute.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"

#define HANDLE_ATTR(Field) \
case EObservedAttribute::Field: \
if constexpr (requires { Data.Field; }) { Data.Field = Value; } \
break;

#undef HANDLE_ATTR


template<typename T>
struct FObservedAttributeHelper {
public:
	void AddHandler(EObservedAttribute Attribute) {
		SetHandler(Attribute, [Attribute](T& Data, float Value)
		{
			#define HANDLE_ATTR(Field) \
			case EObservedAttribute::Field: \
			if constexpr (requires { Data.Field; }) { Data.Field = Value; } \
			break;

			switch (Attribute) {
				HANDLE_ATTR(Health)
				HANDLE_ATTR(MaxHealth)
				HANDLE_ATTR(Mana)
				HANDLE_ATTR(MaxMana)
				HANDLE_ATTR(Damage)
				HANDLE_ATTR(Defense)
				HANDLE_ATTR(Critical)
				HANDLE_ATTR(Speed)
				default: break;
			}

			#undef HANDLE_ATTR
		});
	}
	
	void HandleAttributeChange(EObservedAttribute Attribute, T& Data, float Value) const{
		if (auto Handler = AttributeHandlers.Find(Attribute)){
			(*Handler)(Data, Value);
		}
	}
private:
	void SetHandler(EObservedAttribute Attribute, const TFunction<void(T&, float)>& Handler){
		AttributeHandlers.Add(Attribute,  Handler);
	}

	// const �Լ� �� ���� ���뵵 (�Һ����� ���� ���� ĳ��/��������Ʈ �� ó����)
	mutable TMap<EObservedAttribute, TFunction<void(T&, float)>> AttributeHandlers;
};
