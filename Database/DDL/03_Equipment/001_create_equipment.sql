-- 장비 테이블 (향후 확장용 - 현재 C++ 구현에서는 미사용)
CREATE TABLE equipment (
    equipment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id INT NOT NULL, -- FEquipmentSlotState::ItemID
    slot_index INT NOT NULL, -- FEquipmentSlotState::SlotIndex
    slot_type VARCHAR(50), -- FEquipmentSlotState::SlotType
    is_equipped BOOLEAN DEFAULT FALSE, -- FEquipmentSlotState::bIsEquipped
    enhancement_level JSON, -- FEquipmentSlotState::EnhancementLevel (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_equipment_slot (user_id, slot_index),
    INDEX idx_user_equipment_type (user_id, slot_type),
    INDEX idx_equipped_items (user_id, is_equipped)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
