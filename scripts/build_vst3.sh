#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JUCE_DIR="${JUCE_DIR:-${ROOT_DIR}/JUCE}"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"

if [[ ! -f "${JUCE_DIR}/CMakeLists.txt" ]]; then
  echo "JUCE not found at ${JUCE_DIR}. Set JUCE_DIR or add JUCE as a subfolder." >&2
  exit 1
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DJUCE_DIR="${JUCE_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"

PLUGIN_SRC="${BUILD_DIR}/GrooveSeq_artefacts/${BUILD_TYPE}/VST3/GrooveSeq.vst3"
PLUGIN_DST="${HOME}/Library/Audio/Plug-Ins/VST3/GrooveSeq.vst3"

if [[ ! -d "${PLUGIN_SRC}" ]]; then
  echo "VST3 not found at ${PLUGIN_SRC}" >&2
  exit 1
fi

mkdir -p "$(dirname "${PLUGIN_DST}")"
rm -rf "${PLUGIN_DST}"
cp -R "${PLUGIN_SRC}" "${PLUGIN_DST}"

echo "Installed: ${PLUGIN_DST}"
