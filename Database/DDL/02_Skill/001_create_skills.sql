-- 스킬 테이블 (향후 확장용 - 현재 C++ 구현에서는 미사용)
CREATE TABLE skills (
    skill_instance_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    slot_id VARCHAR(36) NOT NULL, -- FSkillSlotDTO::SlotId (GUID)
    skill_id INT NOT NULL, -- FSkillSlotDTO::SkillID
    slot_index INT NOT NULL, -- FSkillSlotDTO::SlotIndex
    last_used_time TIMESTAMP NULL, -- FSkillSlotDTO::LastUsedTime
    remaining_cooldown FLOAT DEFAULT 0.0, -- FSkillSlotDTO::RemainingCooldown
    is_active BOOLEAN DEFAULT TRUE, -- FSkillSlotDTO::bIsActive
    skill_data JSON, -- FSkillSlotDTO::SkillData (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_skill_slot (user_id, slot_id),
    INDEX idx_user_slot_index (user_id, slot_index),
    INDEX idx_skill_cooldown (skill_id, remaining_cooldown)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
