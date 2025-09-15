#pragma once

class IInventoryDBProvider;
class ISkillDBProvider;
class IShopDBProvider;
class IEquipDBProvider;
class IAuthDBProvider;

class IDBProviderInfra {
public:
	virtual ~IDBProviderInfra() = default;
	virtual TSharedPtr<IEquipDBProvider>     GetEquipDbProvider() = 0;
	virtual TSharedPtr<IShopDBProvider>      GetShopDbProvider() = 0;
	virtual TSharedPtr<ISkillDBProvider>     GetSkillDbProvider() = 0;
	virtual TSharedPtr<IInventoryDBProvider> GetInventoryDbProvider() = 0;
    virtual TSharedPtr<IAuthDBProvider>     GetAuthDbProvider() = 0;
};