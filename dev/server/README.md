# onAir Bridge Server

Bridge между `FDOnAir` (через `SLMessageServer`) и внешними клиентами (Companion/TCP).

Полная документация и сценарии интеграции: `../../README.md`.

## Build (Windows)

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Исполняемый файл: `build/Release/onAir2Companion.exe`.
После сборки CMake автоматически копирует рядом с exe:
- `SLMessageQueue2.dll` (и `SLMessageRemote2.dll`, если есть для этой архитектуры)
- `config.json` (из `config.example.json`)

## Конфиг

Bridge загружает `config.json` из директории, где лежит `onAir2Companion.exe`.

Ключевые параметры:
- `listen_ip`, `listen_port`
- `local_queue`
- `onair_machine`, `onair_queue`, `onair_subject`
- `companion_transport` (`tcp` или `ws`)
- `onair_to_companion_enabled`
- `onair_to_companion_prefix`
- `companion_command_transport` (`http` или `ws`)

## Протокол входящих команд

JSONL (`\n` в конце каждой строки):

```json
{"id":"1","action":"send","command":"Shedule.Start"}
{"id":"h1","action":"health"}
{"id":"c1","action":"companion_send","payload":"/api/location/3/0/1/press"}
```

Plain text тоже поддерживается:

```text
Shedule.Start
```

## События от сервера

- `hello`
- `bridge_state`
- `onair_message`
- `response`
- `health`

## Установка как сервис

`onAir2Companion.exe` не является native Windows Service binary.
Для установки используйте NSSM (service-wrapper).

Скрипты в `run to service/`:
- `install-service.ps1`
- `remove-service.ps1`

Пример:

```powershell
cd "run to service"
powershell -ExecutionPolicy Bypass -File .\install-service.ps1 -StartNow
```

