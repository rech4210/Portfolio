CREATE TABLE users (
  user_id         CHAR(36)       PRIMARY KEY,
  username        VARCHAR(30)    NOT NULL UNIQUE,
  password_hash   VARCHAR(255)   NOT NULL,
  created_at      DATETIME(3)    NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  last_login_at   DATETIME(3)    NULL,
  is_locked       TINYINT(1)     NOT NULL DEFAULT 0    COMMENT '0=정상, 1=잠금',
  lock_expires_at DATETIME(3)    NULL                    COMMENT '잠금 해제 시각',
  is_deleted      TINYINT(1)     NOT NULL DEFAULT 0    COMMENT '0=활성, 1=탈퇴(소프트 삭제)',
  deleted_at      DATETIME(3)    NULL                    COMMENT '탈퇴(소프트 삭제) 일시'
) ENGINE=InnoDB
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
CREATE TABLE characters (
    user_id CHAR(36) PRIMARY KEY,
    character_id VARCHAR(255) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_character_name (character_name),
    INDEX idx_level (level),
    INDEX idx_character_id (character_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE user_audit_logs (
  log_id     BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  user_id    CHAR(36)           NOT NULL,
  action     VARCHAR(50)        NOT NULL COMMENT '예: registration, login_success',
  detail     JSON               NULL,
  created_at DATETIME(3)        NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  INDEX ix_user_action (user_id, action)
) ENGINE=InnoDB
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
CREATE TABLE inventory (
    inventory_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id CHAR(36) NOT NULL,
    item_id VARCHAR(100) NOT NULL,
    quantity INT DEFAULT 1,
    slot_index INT NOT NULL,
    item_data JSON,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES characters(user_id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_slot (user_id, slot_index),
    INDEX idx_user_item (user_id, item_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

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

CREATE TABLE user_skill_slots (
    user_id        CHAR(36) NOT NULL,
    slot_key       VARCHAR(10) NOT NULL,
    skill_id       INT NULL,
    slot_index     INT NULL,
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
CREATE TABLE equipment (
    equipment_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id CHAR(36) NOT NULL,
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

-- 상점 테이블 (향후 확장용)
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
-- 상점 아이템 테이블 (향후 확장용)
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
