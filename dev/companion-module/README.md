# companion-module

Companion v4 модуль для управления `FDOnAir` через bridge-сервер.

Полная документация по установке и маршрутизации onAir -> Companion: `../../README.md`.

## Сборка

```powershell
npm install
npm run build
```

## Подключение как Developer Module

1. В Companion откройте `Settings`.
2. Укажите `Developer modules path` на директорию, содержащую `companion-module`.
3. Перезапустите Companion.
4. Добавьте инстанс `onair-bridge`.

## Настройки инстанса

- `Bridge Host`
- `Bridge Port`
- `Reconnect (ms)`
- `Default machine`
- `Default queue`
- `Default subject`
- `Preset PLAY/STOP/NEXT/TAKE command`

## Actions

- `Send command`
- `Send template command (FDOnAir)`
- `Send JSON line`
- `Health check`

## Feedbacks

- `Bridge connected`
- `Last body contains`
- `Message match (contains|equals|regex)`

## Variables

- `bridge_state`
- `last_message`
- `last_body`
- `last_subject`
- `last_from_machine`
- `last_from_queue`
- `last_response`
- `last_error`

## Templates / Presets

Полный каталог встроенных шаблонов команд: `../docs/COMMAND_TEMPLATES.md`.


