# Тест onAir -> Bridge -> Companion

## 1. Подготовка в Companion

1. Открой страницу `3`.
2. Перетащи пресет `HEALTH` из модуля `onair-bridge` в кнопку `row=0, col=1` (локация `3/0/1`).
3. Убедись, что модуль `onair-bridge` подключен к `192.168.11.71:27015`.

## 2. Команды для onAir (копипаста)

Отправляй в поле команды onAir:

```text
SERVER1/BridgeQueue OnAir1.Mirror COMPANION:POST /api/location/3/0/1/style {"text":"RX_OK","size":14}
```

Потом:

```text
SERVER1/BridgeQueue OnAir1.Mirror COMPANION:POST /api/location/3/0/1/press
```

## 3. Что должно произойти

1. Кнопка `3/0/1` меняет текст на `RX_OK` (проверка HTTP-команды в Companion).
2. Кнопка `3/0/1` нажимается удаленно (через `press`).
3. В логе bridge появляются строки:

```text
onair_message ... body=COMPANION:POST /api/location/3/0/1/press
onair->companion forwarded transport=http payload=POST /api/location/3/0/1/press
tcp action=health
```

`tcp action=health` появится, если на кнопке действительно стоит пресет `HEALTH`.

## 4. Быстрая диагностика

- Есть `onair_message`, но нет `onair->companion forwarded`:
  префикс не `COMPANION:` или `onair_to_companion_enabled=false`.
- Есть `onair->companion forwarded`, но нет эффекта в UI:
  проверь `companion_http_base` и координаты кнопки.
- Нет `onair_message` вообще:
  команда ушла не в `BridgeQueue` или не с `subject=OnAir1.Mirror`.
