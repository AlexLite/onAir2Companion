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

## Команды из onAir в Companion

Отправка выполняется через **«Команды посылки сообщений»** в onAir.
Работа проверялась с транспортом `http` (`companion_command_transport: "http"`).
Для других вариантов транспорта может потребоваться дополнительная доработка и тестирование.

```text
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:/api/location/3/0/1/press
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:POST /api/location/3/0/1/style {"text":"LIVE"}
```
