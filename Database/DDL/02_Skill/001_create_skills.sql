-- 스킬 정의 테이블 (마스터 데이터)
CREATE TABLE skills (
    skill_id      INT PRIMARY KEY,
    display_name  VARCHAR(100) NOT NULL,
    description   TEXT NULL,
    base_cooltime FLOAT NOT NULL DEFAULT 0,
    base_cost     FLOAT NOT NULL DEFAULT 0,
    enabled       TINYINT(1) NOT NULL DEFAULT 1,
    created_at    DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at    DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 유저가 보유한 스킬 상태 테이블
-- skill_data   JSON NULL, -- 확장 데이터 (기존 FSkillSlotDTO::SkillData 호환) -> AssetManager에서 관리

CREATE TABLE user_skills (
    user_id      CHAR(36) NOT NULL,
    skill_id     INT NOT NULL,
    unlocked     TINYINT(1) NOT NULL DEFAULT 0,
    experience   INT NOT NULL DEFAULT 0,
    last_used_at DATETIME(3) NULL,
    created_at   DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at   DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    PRIMARY KEY (user_id, skill_id),
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    FOREIGN KEY (skill_id) REFERENCES skills(skill_id) ON DELETE CASCADE,
    INDEX idx_user_unlocked (user_id, unlocked),
    INDEX idx_skill_experience (skill_id, experience)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 스킬 슬롯 바인딩 테이블 (UI 키 매핑) - 쿨타임 최적화 적용
CREATE TABLE user_skill_slots (
    user_id        CHAR(36) NOT NULL,
    slot_key       VARCHAR(10) NOT NULL,
    skill_id       INT NULL,
    slot_index     INT NULL, -- UI에서의 슬롯 위치 (기존 FSkillSlotDTO::SlotIndex 호환)
    last_used_time DATETIME(3) NULL COMMENT '마지막 사용 시간 (쿨타임 계산 기준점)',
    created_at     DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
    updated_at     DATETIME(3) NOT NULL DEFAULT CURRENT_TIMESTAMP(3) ON UPDATE CURRENT_TIMESTAMP(3),
    PRIMARY KEY (user_id, slot_key),
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    FOREIGN KEY (skill_id) REFERENCES skills(skill_id) ON DELETE SET NULL,
    UNIQUE KEY unique_user_slot_index (user_id, slot_index),
    INDEX idx_skill_binding (skill_id),
    INDEX idx_user_last_used (user_id, last_used_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
