# GitHub Release Checklist

Перед публикацией проверьте:

1. Создан репозиторий на GitHub и известен URL.
2. В `release/companion/onair-bridge-0.1.0/companion/manifest.json` обновлены поля:
   - `repository`
   - `bugs`
   - `maintainers`
3. Bridge запускается с `config.json` и успешно отвечает на `{"action":"health"}`.
4. Модуль Companion подключается и отправляет `Shedule.Start`/`Player.Video 1`.
5. Если используется сценарий onAir -> Companion, включен `onair_to_companion_enabled` и проверен префикс `COMPANION:`.
6. В репозиторий не попали временные файлы (`node_modules`, `build`, `dist`, `*.sqlite`).

Полные инструкции: `../../README.md`.

