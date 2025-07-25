# MySQL 백업 및 복원 스크립트

# 1. 현재 데이터 백업
Write-Host "=== 데이터 백업 중 ==="
$backupFile = "mysql_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss').tar.gz"
docker run --rm -v mygame_db_data:/data -v ${PWD}:/backup alpine tar czf /backup/$backupFile -C /data .
Write-Host "백업 완료: $backupFile"

# 2. 컨테이너 중지 및 볼륨 삭제
Write-Host "=== 컨테이너 중지 및 볼륨 삭제 ==="
docker-compose down
docker volume rm mygame_db_data

# 3. 새로운 스키마로 컨테이너 시작
Write-Host "=== 새로운 스키마로 컨테이너 시작 ==="
docker-compose up -d
Start-Sleep -Seconds 15

# 4. 백업 데이터 복원 (필요한 경우)
Write-Host "=== 백업 데이터 복원 (옵션) ==="
Write-Host "복원하려면: docker run --rm -v mygame_db_data:/data -v ${PWD}:/backup alpine tar xzf /backup/$backupFile -C /data"
