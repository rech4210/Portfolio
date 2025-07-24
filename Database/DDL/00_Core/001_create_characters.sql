-- 캐릭터 기본 정보 테이블(현재 C++ 구현에 맞춤)
CREATE TABLE characters (
    user_id CHAR(36) PRIMARY KEY,
    character_id VARCHAR(255) NOT NULL,
    character_name VARCHAR(100) NOT NULL,
    level INT DEFAULT 1,
    exp BIGINT DEFAULT 0,
    json_data JSON, -- 확장 가능한 캐릭터 데이터(Position, Health, Mana 등)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    INDEX idx_character_name (character_name),
    INDEX idx_level (level),
    INDEX idx_character_id (character_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;이블 (현재 C++ 구현에 맞춤)
