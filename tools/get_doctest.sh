#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="$ROOT_DIR/third_party/doctest"
HEADER_URL="https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h"
LICENSE_URL="https://raw.githubusercontent.com/doctest/doctest/master/LICENSE.txt"

mkdir -p "$DEST_DIR"
echo "Downloading doctest.h ..."
curl -fsSL "$HEADER_URL" -o "$DEST_DIR/doctest.h"
echo "Downloading LICENSE ..."
curl -fsSL "$LICENSE_URL" -o "$DEST_DIR/LICENSE.txt"
echo "doctest ready at third_party/doctest/doctest.h"
