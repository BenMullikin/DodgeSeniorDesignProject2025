# Host test harness

This folder contains a simple host-side test harness that simulates the LE250 sensor and prints JSON payloads similar to the firmware's output. It lets you test parsing/consuming code without the ESP32 or LE250 connected.

Requirements
- Python 3.x

Quick run (PowerShell)
```powershell
python .\test\host_test.py
```

Run a fixed number of messages
```powershell
python .\test\host_test.py -n 10
```

Change interval between messages
```powershell
python .\test\host_test.py -i 0.5
```

Notes
- Output format: compact JSON like:
  `{"sensor_id":"1","timestamp":"2025-11-18 14:32:01","measurement":123}`
- You can pipe output to files or other local programs for integration testing.
