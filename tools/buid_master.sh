#!/usr/bin/env bash 
set -euo pipefall
source /opt/esp-idf/export.sh
cd "$(dirname "$0")/../software/firmware/master"
idf.py set-target esp32s3 build