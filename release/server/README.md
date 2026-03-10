# release/server

Папка для готового runtime bridge.

Ожидаемое содержимое:
- `onAir2Companion.exe`
- `config.json`
- `run to service/install-service.ps1`
- `run to service/remove-service.ps1`

Требование:
- в системе должен быть установлен программный пакет Forward(SLMessageServer),
  который устанавливает `SLMessageQueue2.dll`.

## Запуск

```powershell
cd release\server
.\onAir2Companion.exe
```

## Установка как сервис (NSSM)

```powershell
cd "release\server\run to service"
powershell -ExecutionPolicy Bypass -File .\install-service.ps1 -StartNow
```
