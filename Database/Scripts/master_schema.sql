-- 캐릭터 테이블 (C++ 구조체 매핑)
CREATE TABLE characters (
    user_id VARCHAR(255) PRIMARY KEY,
    character_id VARCHAR(255) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON, -- 캐릭터 상태 정보 (Position, Health, Mana 등)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_character_name (character_name),
    INDEX idx_level (level),
    INDEX idx_character_id (character_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 인벤토리 테이블 (C++ 구조체 매핑)
CREATE TABLE inventory (
    inventory_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id VARCHAR(100) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT NOT NULL, -- 슬롯 인덱스
    item_data JSON, -- FInventoryItemDTO::ItemData (아이템 상태 정보)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_slot (user_id, slot_index),
    INDEX idx_user_item (user_id, item_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 스킬 테이블 (C++ 구조체 매핑)
CREATE TABLE skills (
    skill_instance_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    slot_id VARCHAR(36) NOT NULL, -- FSkillSlotDTO::SlotId (GUID)
    skill_id INT NOT NULL, -- FSkillSlotDTO::SkillID
    slot_index INT NOT NULL, -- FSkillSlotDTO::SlotIndex
    last_used_time TIMESTAMP NULL, -- FSkillSlotDTO::LastUsedTime
    remaining_cooldown FLOAT DEFAULT 0.0, -- FSkillSlotDTO::RemainingCooldown
    is_active BOOLEAN DEFAULT TRUE, -- FSkillSlotDTO::bIsActive
    skill_data JSON, -- FSkillSlotDTO::SkillData (스킬 상태 정보)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_skill_slot (user_id, slot_id),
    INDEX idx_user_slot_index (user_id, slot_index),
    INDEX idx_skill_cooldown (skill_id, remaining_cooldown)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 장비 테이블 (C++ 구조체 매핑)
CREATE TABLE equipment (
    equipment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id INT NOT NULL, -- FEquipmentSlotState::ItemID
    slot_index INT NOT NULL, -- FEquipmentSlotState::SlotIndex
    slot_type VARCHAR(50), -- FEquipmentSlotState::SlotType
    is_equipped BOOLEAN DEFAULT FALSE, -- FEquipmentSlotState::bIsEquipped
    enhancement_level JSON, -- FEquipmentSlotState::EnhancementLevel (강화 상태 정보)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_equipment_slot (user_id, slot_index),
    INDEX idx_user_equipment_type (user_id, slot_type),
    INDEX idx_equipped_items (user_id, is_equipped)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 상점 테이블 (C++ 구조체 매핑)
CREATE TABLE shops (
    shop_id INT PRIMARY KEY, -- FShopDomain::ShopID
    shop_name VARCHAR(200) NOT NULL, -- FShopDomain::ShopName
    shop_description TEXT, -- FShopDomain::ShopDescription
    is_open BOOLEAN DEFAULT TRUE, -- FShopDomain::bIsOpen
    area_id INT, -- FShopDomain::AreaID
    shop_location_x FLOAT, -- FShopDomain::ShopLocation
    shop_location_y FLOAT,
    shop_location_z FLOAT,
    last_restock_time TIMESTAMP NULL, -- FShopDomain::LastRestockTime
    global_price_modifier FLOAT DEFAULT 1.0, -- FShopDomain::GlobalPriceModifier
    shop_owner_name VARCHAR(100), -- FShopDomain::ShopOwnerName
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_shop_name (shop_name),
    INDEX idx_area_id (area_id),
    INDEX idx_is_open (is_open)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- 상점 아이템 테이블 (C++ 구조체 매핑)
CREATE TABLE shop_items (
    shop_item_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    shop_id INT NOT NULL,
    item_id INT NOT NULL, -- FShopItemDTO::ItemID
    item_name VARCHAR(200), -- FShopItemDTO::ItemName
    item_description TEXT, -- FShopItemDTO::ItemDescription
    price FLOAT DEFAULT 0.0, -- FShopItemDTO::Price
    stock INT DEFAULT 0, -- FShopItemDTO::Stock
    is_available BOOLEAN DEFAULT TRUE, -- FShopItemDTO::bIsAvailable
    category VARCHAR(100), -- FShopItemDTO::Category
    max_stock INT, -- FShopItemDTO::MaxStock
    restock_interval_hours FLOAT, -- FShopItemDTO::RestockIntervalHours
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (shop_id) REFERENCES shops(shop_id) ON DELETE CASCADE,
    UNIQUE KEY unique_shop_item (shop_id, item_id),
    INDEX idx_item_category (category),
    INDEX idx_item_price (item_id, price),
    INDEX idx_availability (is_available, stock)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
