-- users 테이블: 계정 관리
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
