#!/usr/bin/env bash
# Fetches the third-party sources the dashboard needs into external/.
# Versions are pinned so builds are reproducible; override with
#   IMGUI_TAG=... JSON_TAG=... scripts/setup_dependencies.sh
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMGUI_TAG="${IMGUI_TAG:-v1.91.9}"
JSON_TAG="${JSON_TAG:-v3.11.3}"

mkdir -p "$ROOT/external"

fetch() {
    local url="$1" tag="$2" dir="$3" marker="$4"
    if [ -f "$dir/$marker" ]; then
        echo "$(basename "$dir") already present"
        return
    fi
    rm -rf "$dir"
    git clone --quiet --depth 1 --branch "$tag" "$url" "$dir"
    echo "fetched $(basename "$dir") $tag"
}

fetch https://github.com/ocornut/imgui.git "$IMGUI_TAG" "$ROOT/external/imgui" imgui.cpp
fetch https://github.com/nlohmann/json.git "$JSON_TAG" "$ROOT/external/json" include/nlohmann/json.hpp

echo "Dependencies are ready."
