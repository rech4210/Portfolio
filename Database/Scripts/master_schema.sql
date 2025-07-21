
-- users ?뚯씠釉? 怨꾩젙 愿由?
CREATE TABLE users (
  user_id         CHAR(36)       PRIMARY KEY,
  username        VARCHAR(30)    NOT NULL UNIQUE,
  password_hash   VARCHAR(255)   NOT NULL,
  created_at      DATETIME(3)    NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  last_login_at   DATETIME(3)    NULL,
  is_locked       TINYINT(1)     NOT NULL DEFAULT 0    COMMENT '0=?뺤긽, 1=?좉툑',
  lock_expires_at DATETIME(3)    NULL                    COMMENT '?좉툑 ?댁젣 ?쒓컖',
  is_deleted      TINYINT(1)     NOT NULL DEFAULT 0    COMMENT '0=?쒖꽦, 1=?덊눜(?뚰봽????젣)',
  deleted_at      DATETIME(3)    NULL                    COMMENT '?덊눜(?뚰봽????젣) ?쇱떆'
) ENGINE=InnoDB
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
-- 罹먮┃??湲곕낯 ?뺣낫 ?뚯씠釉?(?꾩옱 C++ 援ы쁽??留욎땄)
CREATE TABLE characters (
    user_id VARCHAR(255) PRIMARY KEY,
    character_id VARCHAR(255) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON, -- ?뺤옣 媛?ν븳 罹먮┃???곗씠??(Position, Health, Mana ??
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_character_name (character_name),
    INDEX idx_level (level),
    INDEX idx_character_id (character_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- user_audit_logs ?뚯씠釉? 二쇱슂 ?대깽??媛먯궗 濡쒓렇
CREATE TABLE user_audit_logs (
  log_id     BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  user_id    CHAR(36)           NOT NULL,
  action     VARCHAR(50)        NOT NULL COMMENT 'registration, login_success',
  detail     JSON               NULL,
  created_at DATETIME(3)        NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  INDEX ix_user_action (user_id, action)
) ENGINE=InnoDB
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
-- ?몃깽?좊━ ?뚯씠釉?(?꾩옱 C++ 援ы쁽??留욎땄)
CREATE TABLE inventory (
    inventory_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id VARCHAR(100) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT NOT NULL, -- ?몃깽?좊━ ?щ’ ?꾩튂
    item_data JSON, -- FInventoryItemDTO::ItemData (?뺤옣 ?곗씠??
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_slot (user_id, slot_index),
    INDEX idx_user_item (user_id, item_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- ?ㅽ궗 ?뚯씠釉?(?ν썑 ?뺤옣??- ?꾩옱 C++ 援ы쁽?먯꽌??誘몄궗??
CREATE TABLE skills (
    skill_instance_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    slot_id VARCHAR(36) NOT NULL, -- FSkillSlotDTO::SlotId (GUID)
    skill_id INT NOT NULL, -- FSkillSlotDTO::SkillID
    slot_index INT NOT NULL, -- FSkillSlotDTO::SlotIndex
    last_used_time TIMESTAMP NULL, -- FSkillSlotDTO::LastUsedTime
    remaining_cooldown FLOAT DEFAULT 0.0, -- FSkillSlotDTO::RemainingCooldown
    is_active BOOLEAN DEFAULT TRUE, -- FSkillSlotDTO::bIsActive
    skill_data JSON, -- FSkillSlotDTO::SkillData (?뺤옣 ?곗씠??
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_skill_slot (user_id, slot_id),
    INDEX idx_user_slot_index (user_id, slot_index),
    INDEX idx_skill_cooldown (skill_id, remaining_cooldown)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- ?λ퉬 ?뚯씠釉?(?ν썑 ?뺤옣??- ?꾩옱 C++ 援ы쁽?먯꽌??誘몄궗??
CREATE TABLE equipment (
    equipment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(255) NOT NULL,
    item_id INT NOT NULL, -- FEquipmentSlotState::ItemID
    slot_index INT NOT NULL, -- FEquipmentSlotState::SlotIndex
    slot_type VARCHAR(50), -- FEquipmentSlotState::SlotType
    is_equipped BOOLEAN DEFAULT FALSE, -- FEquipmentSlotState::bIsEquipped
    enhancement_level JSON, -- FEquipmentSlotState::EnhancementLevel (?뺤옣 ?곗씠??
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_equipment_slot (user_id, slot_index),
    INDEX idx_user_equipment_type (user_id, slot_type),
    INDEX idx_equipped_items (user_id, is_equipped)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
-- ?곸젏 ?뚯씠釉?(?ν썑 ?뺤옣??
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
-- ?곸젏 ?꾩씠???뚯씠釉?(?ν썑 ?뺤옣??
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
