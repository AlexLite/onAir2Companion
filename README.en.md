# onAir2Companion

Integration of **Soft-Lab FDOnAir** with **Bitfocus Companion** via **SLMessageServer** (bridge).

> Default configuration assumes all components run on the same `localhost`.
> If bridge, onAir, and Companion run on different machines, configure component addresses and ports manually.

## Quick Start

### 1) Bridge

1. Make sure `release\server\` contains:
   - `onAir2Companion.exe`
   - `config.json`
2. Make sure the Forward software package (SLMessageServer) is installed on the system so `SLMessageQueue2.dll` is available.
3. Run:

```powershell
cd release\server
.\onAir2Companion.exe
```

### 2) Companion

Use the ready module package:

`release\companion\onair-bridge-0.1.0`

In Companion:
- Open `Settings`
- Set `Developer modules path` to `release\companion`
- Restart Companion
- Add module `onair-bridge`

### 3) Run bridge as a service

Bridge is a console app, so use NSSM for service mode:

```powershell
cd "release\server\run to service"
powershell -ExecutionPolicy Bypass -File .\install-service.ps1 -StartNow
```

Remove service:

```powershell
cd "release\server\run to service"
powershell -ExecutionPolicy Bypass -File .\remove-service.ps1
```

## Sending Commands from onAir to Companion

Sending is performed through **"Message Sending Commands"** in onAir.
Current validation was performed with `http` transport (`companion_command_transport: "http"`).
Other transport modes may require additional implementation and testing.

Enable routing in `config.json`:

```json
{
  "onair_to_companion_enabled": true,
  "onair_to_companion_prefix": "COMPANION:",
  "companion_command_transport": "http",
  "companion_http_base": "http://127.0.0.1:8000"
}
```

Examples:

```text
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:/api/location/3/0/1/press
MACHINENAME/BridgeQueue OnAir1.Mirror COMPANION:POST /api/location/3/0/1/style {"text":"LIVE"}
```

Where:
- `MACHINENAME` is the machine where bridge is running.
- `BridgeQueue` is `local_queue` from `config.json`.

## Project Structure

### `release/` - runtime-ready packages

- `release/companion/onair-bridge-0.1.0/` - Companion module runtime package
- `release/server/` - bridge runtime folder (`config.json`, scripts in `run to service`, location for `onAir2Companion.exe`)

### `dev/` - development sources

- `dev/server/` - bridge sources (C++/CMake)
- `dev/companion-module/` - Companion module sources (TypeScript)
- `dev/docs/` - documentation and command catalog
- `dev/server/softlab/` - local SoftLab SDK runtime libraries

## For Developers

### Build bridge

```powershell
cd dev\server
cmake -S . -B build -A x64
cmake --build build --config Release
```

Output:

`dev\server\build\Release\onAir2Companion.exe`

### Build Companion module

```powershell
cd dev\companion-module
npm install
npm run build
```

### Use dev module in Companion

`Developer modules path` -> `...\onAir2Companion\dev`

Companion will detect module `companion-module`.

## Useful Links

- FDOnAir command templates: `dev/docs/COMMAND_TEMPLATES.md`
- Pre-release checklist: `dev/docs/GITHUB_RELEASE_CHECKLIST.md`
- Bridge README: `dev/server/README.md`
- Module README: `dev/companion-module/README.md`

## Note

Some project materials (documentation and selected technical edits) were prepared with AI assistance and then validated manually.
