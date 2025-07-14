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
