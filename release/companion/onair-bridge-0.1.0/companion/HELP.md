# onair-bridge

Companion module for the custom onAir bridge server.

## Config

- Host: bridge server IP (for example `192.168.11.71`)
- Port: bridge TCP port (default `27015`)
- Reconnect interval: milliseconds between reconnect attempts

## Actions

- Send command
- Send template command (FDOnAir)
- Send JSON line
- Health check

## Feedbacks

- Bridge connected
- Last body contains text
- Message match (contains/equals/regex) with optional subject/from_queue filters

## Presets

- PLAY
- STOP
- NEXT
- TAKE
- PING
- HEALTH
- FDOnAir Player templates
- FDOnAir Shedule templates
- FDOnAir Folder templates
- FDOnAir Setting templates
- FDOnAir Loading templates
- FDOnAir MirrorSave templates (N+1)

## Variables

- `$(onair-bridge:bridge_state)`
- `$(onair-bridge:last_message)`
- `$(onair-bridge:last_body)`
- `$(onair-bridge:last_subject)`
- `$(onair-bridge:last_from_machine)`
- `$(onair-bridge:last_from_queue)`
- `$(onair-bridge:last_response)`
- `$(onair-bridge:last_error)`
