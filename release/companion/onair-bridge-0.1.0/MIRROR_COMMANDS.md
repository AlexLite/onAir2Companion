# FDOnAir and Mirror Command Catalog

This module includes template commands from:
- `docs/FDOnAirCommands.rtf`
- `docs/OnAir mirror messages.rtf`
- `docs/OnAir mirror messages N+1.rtf`

## FDOnAir templates

Templates are available in Companion action `Send template command (FDOnAir)` and auto-presets grouped by:
- FDOnAir Player
- FDOnAir Shedule
- FDOnAir Folder
- FDOnAir Setting
- FDOnAir Loading
- FDOnAir MirrorSave

## N+1 MirrorSave flow (from OnAir mirror messages N+1)

Request commands (Subject):
- `OnAir1.MirrorSave.GetInfoAll` - request full snapshot
- `OnAir1.MirrorSave.GetInfoChanges` - request changes since previous request
- `OnAir1.MirrorSave.SetIdle` - switch reserve OnAir to idle
- `OnAir1.MirrorSave.Restore` - restore reserve OnAir from mirrored data

Response commands (Subject):
- `OnAir1.MirrorSave.InfoState` - current state payload
- `OnAir1.MirrorSave.InfoSettings` - settings payload
- `OnAir1.MirrorSave.InfoSchedule1` - schedule 1 payload
- `OnAir1.MirrorSave.InfoSchedule2` - schedule 2 payload
- `OnAir1.MirrorSave.RestoreFinished` - restore completed marker

## Notes

- In current bridge module, these commands are sent as command body through `send`.
- `to_machine`/`to_queue`/`subject` are configured per action or from default config.
- For OnAir3 use queue `FDOnAir3` and the same command syntax.
