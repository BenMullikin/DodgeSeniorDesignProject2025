#!/usr/bin/env python3
"""
Host-side test harness to simulate the LE250 sensor and firmware output.

Usage (PowerShell):
  python .\test\host_test.py            # runs indefinitely, 1s interval
  python .\test\host_test.py -n 5      # run 5 iterations
  python .\test\host_test.py -i 0.5    # use 0.5s interval
"""
import time
import json
import random
import argparse
import sys
import threading
from datetime import datetime


def create_sensor_json(sensor_id, measurement):
    return {
        "sensor_id": str(sensor_id),
        "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "measurement": measurement,
    }


def main(iterations=None, interval=1.0):
    stop_event = threading.Event()

    def input_watcher(stop_evt: threading.Event):
        # Wait for user input; if user types 'q', 'quit' or 'exit' then request stop
        try:
            for line in sys.stdin:
                cmd = line.strip().lower()
                if cmd in ("q", "quit", "exit"):
                    stop_evt.set()
                    break
        except Exception:
            # If stdin is closed or not a TTY, just return
            return

    # Start input watcher thread if stdin is interactive
    if sys.stdin is not None and sys.stdin.isatty():
        t = threading.Thread(target=input_watcher, args=(stop_event,), daemon=True)
        t.start()
        print("Type 'q' + Enter to stop the host test.")

    i = 0
    try:
        while (iterations is None or i < iterations) and not stop_event.is_set():
            # Simulate ADC reading (12-bit: 0 - 4095)
            measurement = random.randint(0, 4095)
            payload = create_sensor_json(1, measurement)

            # Print compact JSON (matches create_sensor_json output in firmware)
            print(json.dumps(payload, separators=(",",":"), ensure_ascii=False))
            sys.stdout.flush()

            # Wait with small sleeps so we can respond quickly to stop_event
            waited = 0.0
            while waited < interval and not stop_event.is_set():
                time.sleep(min(0.1, interval - waited))
                waited += 0.1

            i += 1
    except KeyboardInterrupt:
        print("\nHost test stopped by user (KeyboardInterrupt).")
    else:
        if stop_event.is_set():
            print("\nHost test stopped by user (command).")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Host-side sensor test harness")
    parser.add_argument('-n', '--iterations', type=int, default=None, help='Number of iterations to run')
    parser.add_argument('-i', '--interval', type=float, default=1.0, help='Seconds between messages')
    args = parser.parse_args()
    main(args.iterations, args.interval)
