# 적/아군 구분 시스템 설계 전략

## 핵심 방식
- **GameplayTag 기반**으로 적/아군 구분

## 기본 태그 구조 예시
- `Faction.Player`
- `Faction.Enemy`
- `Faction.Boss`
- `Faction.Neutral`

## 주요 전략
1. 모든 캐릭터에 Faction Tag 부여
2. `UGwaFactionLibrary::IsEnemy(Self, Target)` 함수 하나로 통일된 판별 처리
3. 게임 모드(PvE, PvP, Coop 등)에 따라 판별 방식 분기
4. AI Perception 용도에는 필요 시 `TeamID` 병행 사용

## 실전 적용 예시

```cpp
bool UGwaFactionLibrary::IsEnemy(AActor* Self, AActor* Other, EGameModeType Mode)
{
	FGameplayTag SelfFaction = GetFactionTag(Self);
	FGameplayTag OtherFaction = GetFactionTag(Other);

	switch (Mode)
	{
	case EGameModeType::PVE:
		return SelfFaction == "Faction.Player" && OtherFaction == "Faction.Enemy";
	case EGameModeType::PVP:
		return SelfFaction != OtherFaction;
	default:
		return false;
	}
}
```

## 확장 방향

- `Faction.Elite`, `Faction.BossPhase2` 등으로 **세분화된 Faction 분류 가능**
- `Status.Enraged`, `State.Downed` 등과 **상태 조건 조합 가능**
- **데이터 기반 설정**을 통해 디자이너와의 연동 및 밸런싱 용이

---

## 요약 정리

| 항목               | 방식                                         |
|--------------------|----------------------------------------------|
| 표현력 / 확장성     | `GameplayTag` 기반 사용                       |
| AI 인식 및 감지     | `TeamID` 병행 사용                           |
| 유지보수 / 테스트   | `UGwaFactionLibrary` 내부 함수로 집중 관리   |
| 다양한 모드 대응    | `GameModeType` 기준으로 동적 분기 처리       |


// 6월 9일
대상 타격 GA 생성 시도중...
현재 GA 발동 문제로 인해 처리중임. (actor 생성 문제.)