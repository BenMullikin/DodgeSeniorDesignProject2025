#!/usr/bin/env bash
set -euo pipefail
source /opt/esp-idf/export.sh

declare -A TARGETS=(
  ["software/firmware/master"]="esp32s3"
  ["software/firmware/sensor"]="esp32s3"  # change to esp32c3 if/when needed
)

for proj in "${!TARGETS[@]}"; do
  if [[ -f "$proj/CMakeLists.txt" ]]; then
    pushd "$proj" >/dev/null
      tgt="${TARGETS[$proj]}"
      echo "[bootstrap] $proj -> $tgt"
      idf.py --no-ccache set-target "$tgt"
      idf.py -D CMAKE_EXPORT_COMPILE_COMMANDS=ON reconfigure
      ln -sf build/compile_commands.json compile_commands.json
    popd >/dev/null
  else
    echo "[bootstrap] skip $proj (no CMakeLists.txt yet)"
  fi
done
echo "[bootstrap] done."
