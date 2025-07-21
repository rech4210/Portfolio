-- JWT 서버에 맞는 users 테이블 수정 스크립트

-- 기존 users 테이블이 있다면 삭제
DROP TABLE IF EXISTS user_audit_logs;
DROP TABLE IF EXISTS users;

-- JWT 서버 app.js에 맞는 users 테이블 생성
CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(30) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(255) NULL,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_login DATETIME NULL,
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    failed_login_attempts INT NOT NULL DEFAULT 0,
    account_locked_until DATETIME NULL,
    INDEX idx_username (username),
    INDEX idx_active (is_active),
    INDEX idx_lock_status (account_locked_until)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- user_audit_logs 테이블도 JWT 서버에 맞게 수정
CREATE TABLE user_audit_logs (
    log_id BIGINT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NULL, -- NULL 허용 (존재하지 않는 사용자 시도 기록용)
    action VARCHAR(50) NOT NULL,
    details JSON NULL,
    ip_address VARCHAR(45) NOT NULL, -- IPv6 지원
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user_action (user_id, action),
    INDEX idx_created_at (created_at),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 테스트용 사용자 추가 (비밀번호: "password")
-- bcrypt hash for "password" with salt rounds 10: $2b$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi
INSERT INTO users (username, password_hash, email) VALUES 
('testuser', '$2b$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', 'test@example.com'),
('123', '$2b$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi', null);

-- 테이블 생성 확인
SELECT 'Users table created successfully' as result;
SHOW TABLES;
DESCRIBE users;
