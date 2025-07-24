-- 인벤토리 테이블(현재 C++ 구현에 맞춤)
CREATE TABLE inventory (
    inventory_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id CHAR(36) NOT NULL,
    item_id VARCHAR(100) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT NOT NULL, -- 인벤토리 슬롯 위치
    item_data JSON, -- FInventoryItemDTO::ItemData (확장 데이터)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_slot (user_id, slot_index),
    INDEX idx_user_item (user_id, item_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
