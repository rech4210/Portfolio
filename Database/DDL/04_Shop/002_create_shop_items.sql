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
