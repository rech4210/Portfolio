-- ===============================================================================
-- Skill Cooldown Optimization Queries
-- 목적: DB에는 last_used_time만 저장하고, 쿨타임은 서버/클라에서 실시간 계산
-- 기반 테이블: skills, user_skills, user_skill_slots (최적화 버전)
-- ===============================================================================

-- ===============================================================================
-- 1. 스킬 사용 시점 UPDATE 쿼리
-- 목적: 스킬 사용 즉시 last_used_time을 현재 시간으로 갱신
-- 파라미터: :userId, :slotKey
-- ===============================================================================

UPDATE user_skill_slots 
SET 
    last_used_time = NOW(3),
    updated_at = NOW(3)
WHERE 
    user_id = :userId 
    AND slot_key = :slotKey 
    AND skill_id IS NOT NULL;

-- ===============================================================================
-- 2. 실시간 쿨타임 계산 조회 쿼리 (UI 갱신용)
-- 목적: 슬롯별 스킬 쿨타임 상태를 실시간으로 계산하여 조회
-- 출력: 남은 쿨타임, 사용 가능 여부, 쿨타임 완료 예상 시간
-- ===============================================================================

SELECT 
    uss.slot_key,
    uss.slot_index,
    uss.skill_id,
    s.display_name AS skill_name,
    s.base_cooltime,
    uss.last_used_time,
    
    -- 경과 시간 계산 (초 단위, 마이크로초 정밀도)
    CASE 
        WHEN uss.last_used_time IS NULL THEN NULL
        ELSE TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0
    END AS elapsed_seconds,
    
    -- 남은 쿨타임 계산 (초 단위, 0 이하는 0으로)
    CASE 
        WHEN uss.last_used_time IS NULL THEN 0
        ELSE GREATEST(
            s.base_cooltime - TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0, 
            0
        )
    END AS remaining_cooldown,
    
    -- 사용 가능 여부 (boolean)
    CASE 
        WHEN uss.skill_id IS NULL THEN FALSE
        WHEN uss.last_used_time IS NULL THEN TRUE
        WHEN TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0 >= s.base_cooltime THEN TRUE
        ELSE FALSE
    END AS is_ready,
    
    -- 쿨타임 완료 예상 시간
    CASE 
        WHEN uss.last_used_time IS NULL THEN NULL
        ELSE DATE_ADD(uss.last_used_time, INTERVAL s.base_cooltime SECOND)
    END AS cooldown_end_at

FROM user_skill_slots uss
LEFT JOIN skills s ON uss.skill_id = s.skill_id
WHERE uss.user_id = :userId
ORDER BY uss.slot_index ASC, uss.slot_key ASC;

-- ===============================================================================
-- 3. 특정 슬롯 쿨타임 검증 쿼리 (스킬 사용 전 검증용)
-- 목적: 스킬 사용 가능 여부를 빠르게 검증
-- 파라미터: :userId, :slotKey
-- ===============================================================================

SELECT 
    uss.skill_id,
    s.base_cooltime,
    uss.last_used_time,
    CASE 
        WHEN uss.skill_id IS NULL THEN FALSE
        WHEN uss.last_used_time IS NULL THEN TRUE
        WHEN TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0 >= s.base_cooltime THEN TRUE
        ELSE FALSE
    END AS can_use,
    CASE 
        WHEN uss.last_used_time IS NULL THEN 0
        ELSE GREATEST(
            s.base_cooltime - TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0, 
            0
        )
    END AS remaining_seconds
FROM user_skill_slots uss
LEFT JOIN skills s ON uss.skill_id = s.skill_id
WHERE uss.user_id = :userId AND uss.slot_key = :slotKey;

-- ===============================================================================
-- 4. 로그인/재접속 시 전체 쿨타임 상태 조회
-- 목적: 클라이언트 초기화용 전체 스킬 상태 조회
-- 파라미터: :userId
-- ===============================================================================

SELECT 
    uss.slot_key,
    uss.slot_index,
    uss.skill_id,
    s.display_name,
    s.base_cooltime,
    CASE 
        WHEN uss.last_used_time IS NULL THEN 0
        ELSE GREATEST(
            s.base_cooltime - TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0, 
            0
        )
    END AS remaining_cooldown,
    CASE 
        WHEN uss.last_used_time IS NULL THEN NULL
        ELSE DATE_ADD(uss.last_used_time, INTERVAL s.base_cooltime SECOND)
    END AS cooldown_end_at,
    CASE 
        WHEN uss.skill_id IS NULL THEN FALSE
        WHEN uss.last_used_time IS NULL THEN TRUE
        WHEN TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0 >= s.base_cooltime THEN TRUE
        ELSE FALSE
    END AS is_ready
FROM user_skill_slots uss
LEFT JOIN skills s ON uss.skill_id = s.skill_id
WHERE uss.user_id = :userId AND uss.skill_id IS NOT NULL
ORDER BY uss.slot_index ASC;

-- ===============================================================================
-- 5. 배치용 만료된 쿨타임 정리 쿼리 (선택적 사용)
-- 목적: 오래된 last_used_time 데이터 정리 (성능 최적화)
-- 파라미터: :cleanupDays (기본 30일)
-- ===============================================================================

-- 30일 이상 된 쿨타임 완료 기록 정리
UPDATE user_skill_slots uss
INNER JOIN skills s ON uss.skill_id = s.skill_id
SET last_used_time = NULL, updated_at = NOW(3)
WHERE 
    uss.last_used_time IS NOT NULL
    AND uss.last_used_time < DATE_SUB(NOW(3), INTERVAL :cleanupDays DAY)
    AND TIMESTAMPDIFF(SECOND, uss.last_used_time, NOW(3)) > s.base_cooltime;

-- ===============================================================================
-- 6. 트랜잭션을 활용한 스킬 사용 처리 (예시)
-- 목적: 동시성 제어를 포함한 안전한 스킬 사용 처리
-- ===============================================================================

-- START TRANSACTION;

-- -- 1. 쿨타임 검증 (FOR UPDATE로 동시성 제어)
-- SELECT 
--     uss.skill_id,
--     CASE 
--         WHEN uss.skill_id IS NULL THEN FALSE
--         WHEN uss.last_used_time IS NULL THEN TRUE
--         WHEN TIMESTAMPDIFF(MICROSECOND, uss.last_used_time, NOW(3)) / 1000000.0 >= s.base_cooltime THEN TRUE
--         ELSE FALSE
--     END AS can_use
-- FROM user_skill_slots uss
-- LEFT JOIN skills s ON uss.skill_id = s.skill_id
-- WHERE uss.user_id = :userId AND uss.slot_key = :slotKey
-- FOR UPDATE;

-- -- 2. 조건 만족 시 사용 시간 갱신
-- UPDATE user_skill_slots 
-- SET last_used_time = NOW(3), updated_at = NOW(3)
-- WHERE user_id = :userId AND slot_key = :slotKey AND skill_id IS NOT NULL;

-- -- 3. 추가 로직 (경험치, 통계 등)
-- UPDATE user_skills 
-- SET experience = experience + :expGain, last_used_at = NOW(3), updated_at = NOW(3)
-- WHERE user_id = :userId AND skill_id = :skillId;

-- COMMIT;

-- ===============================================================================
-- 성능 최적화 및 인덱스 활용 팁
-- ===============================================================================

/*
1. 인덱스 활용:
   - idx_user_last_used (user_id, last_used_time): 사용자별 쿨타임 조회 최적화
   - idx_skill_binding (skill_id): JOIN 성능 향상

2. 쿼리 최적화:
   - TIMESTAMPDIFF(MICROSECOND, ...) / 1000000.0: 소수점 정밀도 확보
   - GREATEST() 함수: 음수 방지
   - LEFT JOIN vs INNER JOIN: 빈 슬롯 포함 여부에 따라 선택

3. 캐싱 전략:
   - 자주 조회되는 사용자의 쿨타임 정보는 Redis 캐싱 고려
   - TTL을 쿨타임보다 짧게 설정하여 정확성 보장

4. 모니터링 포인트:
   - TIMESTAMPDIFF 함수 호출 빈도
   - user_skill_slots 테이블 스캔 빈도
   - 트랜잭션 대기 시간
*/
