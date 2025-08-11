\
# Option B migration to unified root PlatformIO
# Run from repo root in PowerShell
$ErrorActionPreference = "Stop"

# Optional: create a branch
git checkout -b refactor/unified-platformio

# Remove vendored libs (if present)
if (Test-Path "station\lib\AsyncTCP") { git rm -r "station\lib\AsyncTCP" }
if (Test-Path "station\lib\ESPAsyncWebServer") { git rm -r "station\lib\ESPAsyncWebServer" }

# Remove per-project platformio.ini and duplicate scripts
if (Test-Path "base\platformio.ini") { git rm "base\platformio.ini" }
if (Test-Path "station\platformio.ini") { git rm "station\platformio.ini" }
if (Test-Path "base\scripts\version_stamp.py") { git rm "base\scripts\version_stamp.py" }
if (Test-Path "station\scripts\version_stamp.py") { git rm "station\scripts\version_stamp.py" }

# Ensure scripts directory
if (!(Test-Path "scripts")) { New-Item -ItemType Directory "scripts" | Out-Null }

git add -A
git commit -m "Unify PlatformIO at root with base & station envs; centralize version script; remove vendored libs"
