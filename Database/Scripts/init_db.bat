@echo off
chcp 65001 > nul
REM MyGame 데이터베이스 초기화 스크립트 (Docker 대상)
REM 이 스크립트는 DDL 폴더의 모든 .sql 파일을 순서대로 합쳐 실행합니다.

REM --- 설정 (Docker 환경에 맞게 수정) ---
SET DB_HOST=127.0.0.1
SET DB_PORT=3307
SET DB_USER=Regacy
SET DB_PASSWORD=GGwa1234
SET DB_NAME=MyGameDB
SET MYSQL_PATH="C:\Program Files\MySQL\MySQL Server 8.4\bin\mysql.exe"
REM ---------------

ECHO ==================================================
ECHO MyGame Database Initialization Script for DOCKER
ECHO ==================================================
ECHO.
ECHO ⚠️  주의: 이 스크립트를 실행하기 전에 다음을 확인하세요:
ECHO    1. Docker의 'mygame_db' 컨테이너가 실행 중인지 확인
ECHO    2. 접속 정보: Host=%DB_HOST%, Port=%DB_PORT%
ECHO    3. 사용자 정보: User=%DB_USER%
ECHO.
PAUSE

REM 임시 마스터 스키마 파일 경로
SET MASTER_SCHEMA_FILE=master_schema.sql

REM 기존 마스터 스키마 파일 삭제
IF EXIST %MASTER_SCHEMA_FILE% (
    DEL %MASTER_SCHEMA_FILE%
    ECHO Deleted existing %MASTER_SCHEMA_FILE%.
)

ECHO Generating master schema file from DDL folder...

REM DDL 폴더의 모든 .sql 파일을 순서대로 합치기
powershell -Command "Get-ChildItem -Path ..\DDL -Recurse -Filter *.sql | Sort-Object FullName | Get-Content | Set-Content -Encoding utf8 %MASTER_SCHEMA_FILE%"

IF NOT EXIST %MASTER_SCHEMA_FILE% (
    ECHO ❌ Failed to create master schema file.
    ECHO    - DDL 폴더 경로를 확인해주세요.
    ECHO    - PowerShell 실행 권한을 확인해주세요.
    PAUSE
    GOTO :EOF
)

ECHO ✅ Master schema file created successfully: %MASTER_SCHEMA_FILE%
ECHO.

REM 스키마 파일 내용 미리보기
ECHO === DDL Preview (첫 10줄) ===
powershell -Command "Get-Content %MASTER_SCHEMA_FILE% | Select-Object -First 10"
ECHO ================================
ECHO.
ECHO Executing schema on Docker database: %DB_NAME%...

REM MySQL 클라이언트로 실행 (데이터베이스 생성 포함)
ECHO CREATE DATABASE IF NOT EXISTS %DB_NAME% CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci; | %MYSQL_PATH% -h %DB_HOST% -P %DB_PORT% -u %DB_USER% -p%DB_PASSWORD% --ssl-mode=DISABLED

%MYSQL_PATH% -h %DB_HOST% -P %DB_PORT% -u %DB_USER% -p%DB_PASSWORD%  --ssl-mode=DISABLED %DB_NAME% < %MASTER_SCHEMA_FILE%

IF %ERRORLEVEL% NEQ 0 (
    ECHO.
    ECHO *************************************************
    ECHO *  ❌ DATABASE INITIALIZATION FAILED!
    ECHO *  다음을 확인해주세요:
    ECHO *  1. Docker의 'mygame_db' 컨테이너 실행 상태
    ECHO *  2. 사용자 계정과 비밀번호
    ECHO *  3. 접속 포트(%DB_PORT%)가 올바른지 확인
    ECHO *************************************************
    DEL %MASTER_SCHEMA_FILE%
    PAUSE
    GOTO :EOF
)

ECHO.
ECHO ==================================================
ECHO  ✅ Database initialization complete!
ECHO ==================================================
ECHO.
ECHO 📋 요약:
ECHO    - 대상 데이터베이스: %DB_NAME% on Docker (%DB_HOST%:%DB_PORT%)
ECHO    - 처리된 테이블: characters, inventory, skills, equipment, shops, shop_items
ECHO.

REM 임시 파일 삭제
DEL %MASTER_SCHEMA_FILE%
ECHO 🗑️  Deleted temporary schema file.

ECHO.
ECHO 🚀 이제 다음 단계를 진행할 수 있습니다:
ECHO    1. UE5의 DatabaseSettings에서 포트를 %DB_PORT%로 변경
ECHO    2. DatabaseManager 기능 테스트
ECHO.

:EOF
PAUSE
