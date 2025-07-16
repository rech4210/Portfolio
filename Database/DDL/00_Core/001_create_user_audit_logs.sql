-- user_audit_logs 테이블: 주요 이벤트 감사 로그
CREATE TABLE user_audit_logs (
  log_id     BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
  user_id    CHAR(36)           NOT NULL,
  action     VARCHAR(50)        NOT NULL COMMENT '예: registration, login_success',
  detail     JSON               NULL,
  created_at DATETIME(3)        NOT NULL DEFAULT CURRENT_TIMESTAMP(3),
  INDEX(ix_user_action) (user_id, action)
) ENGINE=InnoDB
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;
