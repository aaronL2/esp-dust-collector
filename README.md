# ESP Dust Collector

This repository contains two PlatformIO-based projects:

- `base/` – Controls the central collector and tracks stations
- `station/` – Runs on each ESP-based station, controls servo gates and reports current

Open the included `.code-workspace` file in VS Code for full multi-project development.

# Configuration

The base exposes a small HTTP API that allows runtime tuning of the current
threshold used to determine when the dust collector should run. Post a JSON
payload to `/set-tool-on-threshold` to adjust this value:

```bash
curl -X POST http://<base-ip>/set-tool-on-threshold \
     -H "Content-Type: application/json" \
     -d '{"tool_on_threshold":5.0}'
```

The relay uses a debounce period (default `750ms`) to avoid rapid toggling when
the measured current fluctuates around the threshold. To change this duration,
edit the `kDebounceMs` constant in `base/src/comms.cpp` and rebuild.
