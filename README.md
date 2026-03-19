# onAir2Companion

English version: [README.en.md](README.en.md)

Интеграция **Soft-Lab FDOnAir** с **Bitfocus Companion** через **SLMessageServer** (bridge).

> По умолчанию конфигурация рассчитана на запуск всех компонентов на одном `localhost`.
> Если bridge, onAir и Companion находятся на разных машинах, нужно вручную настроить адреса и порты в конфигурации компонентов.

## Быстрый запуск

### 1) Bridge

1. Проверьте, что в `release\server\` лежат:
   - `onAir2Companion.exe`
   - `config.json`
2. Убедитесь, что в системе установлен программный пакет Forward,
   чтобы `SLMessageQueue2.dll` была установлена и доступна в системе.
3. Запустите:

```powershell
cd release\server
.\onAir2Companion.exe
```

### 2) Companion

Используйте готовый пакет модуля:

`release\companion\onair-bridge-0.1.0`

В Companion:
- Откройте `Settings`
- Укажите `Developer modules path` на папку `release\companion`
- Перезапустите Companion
- Добавьте модуль `onair-bridge`

### 3) Запуск bridge как сервиса

Bridge — console app, для сервиса используйте NSSM:

```powershell
cd "release\server\run to service"
powershell -ExecutionPolicy Bypass -File .\install-service.ps1 -StartNow
```

Удаление сервиса:

```powershell
cd "release\server\run to service"
powershell -ExecutionPolicy Bypass -File .\remove-service.ps1
```

## Передача команд из onAir в Companion

Отправка выполняется через механизм **«Команды посылки сообщений»** в onAir.
Текущая проверка выполнялась с транспортом `http` (`companion_command_transport: "http"`).
Для других вариантов транспорта может потребоваться дополнительная доработка и тестирование.

В `config.json` включите маршрутизацию:

```json
{
  "onair_to_companion_enabled": true,
  "onair_to_companion_prefix": "COMPANION:",
  "companion_command_transport": "http",
  "companion_http_base": "http://127.0.0.1:8000"
}
```

Примеры команд из onAir:

```text
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:/api/location/3/0/1/press
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:POST /api/location/3/0/1/style {"text":"LIVE"}
```

Где:
- `MACHINENAME` — имя машины, на которой работает bridge.
- `BridgeQueue` — значение `local_queue` из `config.json`.

## Структура проекта

### `release/` - готовые к запуску пакеты

- `release/companion/onair-bridge-0.1.0/` - релизный пакет модуля Companion
- `release/server/` - runtime-папка bridge (`config.json`, скрипты в `run to service`, место для `onAir2Companion.exe`)

### `dev/` - все для разработки

- `dev/server/` - исходники bridge (C++/CMake)
- `dev/companion-module/` - исходники модуля Companion (TypeScript)
- `dev/docs/` - документация и каталог команд
- `dev/server/softlab/` - локальные SDK-библиотеки SoftLab для runtime

## Для разработчика

### Сборка bridge

```powershell
cd dev\server
cmake -S . -B build -A x64
cmake --build build --config Release
```

Готовый exe после сборки:

`dev\server\build\Release\onAir2Companion.exe`

### Сборка Companion-модуля

```powershell
cd dev\companion-module
npm install
npm run build
```

### Использование dev-модуля в Companion

`Developer modules path` -> `...\onAir2Companion\dev`

Companion увидит модуль `companion-module`.

## Полезные ссылки

- Каталог шаблонов FDOnAir: `dev/docs/COMMAND_TEMPLATES.md`
- Чеклист перед публикацией: `dev/docs/GITHUB_RELEASE_CHECKLIST.md`
- README bridge: `dev/server/README.md`
- README модуля: `dev/companion-module/README.md`

## Примечание

Часть материалов проекта (документация и отдельные технические правки) была подготовлена с помощью ИИ-агентов.


