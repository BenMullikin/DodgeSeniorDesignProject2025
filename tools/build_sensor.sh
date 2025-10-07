#!/usr/bin/env bash
set -euo pipefail
source /opt/esp-idf/export.sh
cd "$(dirname "$0")/../software/firmware/sensor"
idf.py set-target esp32s3 build