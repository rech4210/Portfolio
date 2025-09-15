# Database Provider Infra Refactor Plan

## Current State (Post-Subsystem Migration)
- `UDBProviderInfra` promoted to `UGameInstanceSubsystem` for stable lifecycle.
- Providers (`Equip/Shop/Skill/Inventory`) constructed in `Initialize` using `UDatabaseManager` subsystem.
- Runtime modules now guarded (`#if WITH_SERVER_CODE`) where they still include `DatabaseManager.h`.
- `SkillSubsystem` uses subsystem instead of constructing `UDBProviderInfra` manually.

## Remaining Issues
- DTO types (e.g., `FInventoryItemDTO`, `FSkillSlotDatabaseDTO`, `FShopRepositoryResult`) still defined inside `DatabaseManager.h` (ServerOnly) causing leakage.
- Multiple repositories still directly depend on server DTOs via forward declarations only; actual logic may require full definition.
- Legacy skill APIs co-exist with new 3-layer APIs; interface surface area is large.

## Target Architecture
```
[ Game Logic Modules ] --> [ GameShared (Interfaces + Data DTO Light) ] --> [ DatabaseModule (ServerOnly Impl) ]
```
Optionally introduce a new `DatabaseData` runtime module holding only pure POD DTO structs used broadly.

## Phased Plan
### Phase 1 (Done)
- Promote `UDBProviderInfra` to subsystem.
- Guard server-only includes.

### Phase 2 (In Progress)
- Replace direct `UDatabaseManager` usages with provider interface retrieval (module by module).
  - SkillModule: repository init migrated. (Next: Inventory, Shop, Auth, Equipment.)

### Phase 3 (DTO Extraction)
1. Enumerate DTO families:
   - Character: `FCharacterData`
   - Inventory: `FInventoryItemDTO`
   - Skill: `FSkillSlotDTO`, `FSkillSlotDatabaseDTO`, `FSkillMasterDatabaseDTO`
   - Shop: `FShopItemDTO`, `FShopDomain`, `FShopRepositoryResult`
   - User: `FDatabaseUserData`, `FDatabaseAuditLogData`
2. Split into layers:
   - Core Gameplay DTO (needed client + server): Character basic, Inventory items (without DB-only metadata), Skill runtime slot snapshot, Shop presentation data (without internal pricing rules), Audit/User excluded (server).
   - Server-only Extended DTO: Audit logs, user account, internal shop restock fields, skill master tuning, etc.
3. Create new module `GameDataShared` (Runtime) or reuse `GameSharedModule` if acceptable.
4. Move core gameplay DTO headers there; adjust includes.
5. Provide mapping functions in DatabaseModule to convert DB-layer DTO -> Core DTO.

### Phase 4 (Legacy Skill API Removal)
- Collect all callsites of deprecated methods.
- Provide migration table (Old -> New API).
- Remove deprecated methods from interfaces and providers after migration.

### Phase 5 (Error Handling & Result Normalization)
- Introduce `FDbOpResult` (bool Success, FString Error, optional payload via template or variant) to standardize returns.
- Retrofit providers gradually.

### Phase 6 (Testing & Validation)
- Add unit-style tests (where possible) for mapping functions.
- Add integration smoke command (console) to validate provider pipeline.

## Short-Term Action Items
| Priority | Task | Module | Notes |
|----------|------|--------|-------|
| High | Inventory repository provider injection | InventoryModule | Mirror Skill pattern |
| High | Shop repository provider injection | ShopModule | Replace direct DBManager calls |
| Medium | DTO classification spreadsheet | Docs | Aid extraction |
| Medium | Add Facade helper (optional) | GameSharedModule | Static helper to fetch providers |
| Low | Legacy skill API usage audit | SkillModule | Prepare removal |

## Subsystem Usage Guidelines
- Access providers via:
  ```cpp
  if (auto Infra = GetGameInstance()->GetSubsystem<UDBProviderInfra>()) {
      auto SkillProvider = Infra->GetSkillDbProvider();
  }
  ```
- Do NOT cache raw subsystem pointer in long-lived static contexts; use on-demand lookup or weak/shared pointer to provider only.
- Wrap provider calls with `WITH_SERVER_CODE` when compiled into client targets.

## Risks & Mitigations
- Risk: Moving DTOs breaks serialization assumptions.
  - Mitigation: Provide transitional typedefs or include wrappers.
- Risk: Client build accidentally references server-only audit/user DTOs.
  - Mitigation: Keep them in DatabaseModule and add explicit `#if WITH_SERVER_CODE` guards.

## Open Questions
- Should audit/user account data be extracted at all? Probably no (belongs to auth service boundary).
- Do we need async abstraction beyond `UE::Tasks::TTask`? Consider if future backend swap (e.g., HTTP) requires interface independence.

---
(End of Plan)
