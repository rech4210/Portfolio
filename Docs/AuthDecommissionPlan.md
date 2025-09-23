# AuthModule Decommission Plan

Status: Draft (Phase 0)  
Target Removal Version: v0.9 (pre-production hardening)

## 1. Scope
Legacy `AuthModule` (Runtime) currently houses:
- `UAuthSubsystem`
- `UAuthRepository`, `UAuthRepositoryInterface`
- `UAuthDomainService`, `AuthComponent`
- DTOs (partially migrated already to GameSharedModule)
- Deprecated internal credential / audit / lock handling logic now superseded by:
  - `AuthServerModule` (`UAuthExternalService`, `UAuthSessionSubsystem`)
  - `DatabaseModule` providers (lock / audit persistence if retained)

## 2. Removal Goals
- Eliminate runtime exposure of server-only auth logic to clients.
- Consolidate external JWT flows exclusively in `AuthServerModule`.
- Reduce build time & binary size by removing unused repo/domain layers.
- Prevent accidental reintroduction of password / credential handling locally.

## 3. Migration Mapping
| Legacy Element | Replacement | Action |
|----------------|------------|--------|
| `UAuthSubsystem::RequestServerAuthentication` | `UAuthSessionSubsystem::LoginUser` | Updated (done) |
| `UAuthSubsystem::RequestServerRegistration` | `UAuthSessionSubsystem::RegisterUser` | Updated (done) |
| `UAuthSubsystem::VerifyTokenWithAuthServer` | `UAuthSessionSubsystem::VerifyToken` | Redirect |
| `UAuthRepository` (CRUD Users) | External Auth Service (Node.js) | DELETE |
| `UAuthRepository` (Lock/Unlock/Audit) | `IAuthDBProvider` via dedicated persistence helper (optional) | Extract minimal if needed |
| `UAuthDomainService` (password / attempts) | External Auth Service | DELETE |
| `AuthComponent` (legacy coupling) | N/A (UI or session events) | DELETE / Replace simple notifier |
| `AuthVerificationService` | `UAuthExternalService::VerifyTokenAsync` | Phase-out |

## 4. Decommission Phases
### Phase 0 (NOW)
- Deprecation warnings (done)
- New server-only path live (done)
- Documentation of plan (this file)

### Phase 1
- Replace any remaining direct includes of `AuthSubsystem.h` (verify none outside module)
- Add compile guard macro `AUTHMODULE_LEGACY_DISABLED` toggle to ensure no new code attaches.

### Phase 2
- Hard deprecate public UFUNCTIONs in `UAuthSubsystem` with UE_DEPRECATED macro.
- Migrate any residual Blueprint references (search for BP assets referencing `UAuthSubsystem`).
- Extract any still needed audit lock calls to a lightweight `AuthPersistenceService` (plain C++ or moved into `AuthServerModule`).

### Phase 3
- Remove `AuthModule` from `.uproject` module list.
- Delete source directory `Source/AuthModule/`.
- Clean Intermediate & regenerate project files.
- Update README / architecture diagrams.

### Phase 4
- CI build verify (client + dedicated server) succeeds without `AuthModule`.
- Remove migration macro and doc references.

## 5. Blueprint / Asset Impact Checklist
| Check | Tool/Action | Status |
|-------|-------------|--------|
| Search for `AuthSubsystem` in Blueprint assets | Content Browser filter, redirector fix | Pending |
| Any `AuthDomainService` references in BP | Same search | Pending |
| Any soft class references to `AuthSubsystem` in config | Grep `Default*.ini` | Pending |

## 6. Code Search Checklist
Grep terms before removal:
- `AuthSubsystem` (should only appear inside legacy module)
- `UAuthRepository` / `AuthRepositoryInterface`
- `AuthDomainService`
- `AuthComponent`
- `RequestServerAuthentication` / `RequestServerRegistration` (already isolated)

## 7. Persistence & Audit Strategy
If audit logs & lock state still required:
- Keep only provider interface calls: `LockUserAccount`, `UnlockUserAccount`, `AddAuditLog` style wrappers.
- Implement a minimal `AuthPersistenceService` in `AuthServerModule` (non-UObject) for grouping if needed.

## 8. Configuration Migration
Move any `AuthServerUrl`, `RequestTimeoutSeconds` settings to:
```
[/Script/AuthServerModule.AuthSettings]
AuthServerUrl="http://127.0.0.1:3000"
AuthRequestTimeout=15
```
Add runtime override via CVars:
```
Auth.Url
Auth.Timeout
```

#### Runtime Configuration

INI (DefaultGame.ini / Game.ini):
```
[AuthServer]
AuthServerUrl="http://127.0.0.1:3000"
AuthRequestTimeoutSeconds=10.0
AuthCacheSweepIntervalSeconds=60.0
```

Console Variables (highest precedence at runtime):
```
auth.Url=<override full base url>
auth.Timeout=<seconds or -1 to keep config>
auth.CacheSweep=<seconds or -1 to keep config>
```

Precedence: Console Var (if set & not default) > INI > Built-in defaults (http://127.0.0.1:3000, 10s, 60s sweep).

Logging Category in use: `LogServerAuth` (replaces prior `LogTemp` in new auth stack). Legacy module still uses `LogTemp` until removal.

## 9. Risk Mitigation
| Risk | Mitigation |
|------|------------|
| Hidden Blueprint dependency causes load error | Run PIE with legacy flagged disabled before deletion |
| Forgotten include introduces accidental re-coupling | Add failing static_assert in a temporary header if `UAuthSubsystem` referenced outside module |
| Token URL leakage persists | Enforce validation that `?token=` is absent in travel URL (log warning) |

## 10. Removal Readiness Gate
Removal allowed when ALL true:
- [ ] No non-legacy module includes `AuthSubsystem.h`
- [ ] Blueprint search returns zero references
- [ ] Audit/lock features either re-implemented or explicitly deferred
- [ ] README updated (Auth architecture diagram)
- [ ] CI green (Client + Server targets)

## 11. Post-Removal Verification
Smoke tests:
- Login success / failure path
- Registration path (if exposed)
- Token expiry -> re-login or rejection
- Multi-client concurrency (2+ simultaneous logins) behavior consistent with chosen policy

## 12. Follow-Up Enhancements (Not Blocking Removal)
- JWT signature verification (public key refresh)
- Refresh token flow (silent renewal)
- Rate limiting / brute-force telemetry (server side)

---
Owner: Auth / Backend Engineer  
Reviewers: Gameplay Lead, Security Reviewer

## 13. Interface Reuse Update (2025-09-23)
To avoid creating a new bridge layer, the existing `AuthRPCInterface` was extended instead of adding another server-client contract.

Changes Implemented:
- Added token & error propagation: `NotifyAuthLoginResult(bool bSuccess, const FString& UserId, const FString& Token, const FString& ErrorCode)` and `NotifyAuthRegisterResult(...)`.
- `UAuthSessionSubsystem` only requires a `UObject*` and casts to `IAuthRPCInterface`, removing compile-time dependency on `AGGwaPlayerController`.
- `AGGwaPlayerController` no longer calls legacy `UAuthSubsystem`; routes through `UAuthSessionSubsystem` under `#if WITH_SERVER_CODE`.
- Immediate failure fallback sends standardized error codes (`ServerUnavailable`, `LoginFailed`, `RegistrationFailed`).

Planned Next (Non-blocking):
- Introduce `EAuthErrorCode` enum + localization mapping.
- External service response → structured error translation table.
- Multi-session policy & refresh token scheduling.

Result: Cleaner module boundaries, no new interface proliferation, and simplified decommission of `AuthModule`.
