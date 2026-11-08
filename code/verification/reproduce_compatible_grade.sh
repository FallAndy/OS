#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CODE_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
COMPAT_ROOT=$(mktemp -d /tmp/lab3-grade-compatible.XXXXXX)

echo "isolated grading tree: $COMPAT_ROOT"

tar -C "$CODE_ROOT" \
    --exclude=.git \
    --exclude=.vscode \
    --exclude=vcs \
    --exclude=verification \
    -cf - . | tar -C "$COMPAT_ROOT" -xf -

python3 "$CODE_ROOT/tools/inject.py" "$COMPAT_ROOT"
cp "$COMPAT_ROOT/tools/grade.sh" "$COMPAT_ROOT/tools/grade.compat.sh"

# Compatibility-only change: QEMU -kernel supplies OpenSBI's next-stage entry.
# Test injection, assertions, sentinels, timeouts, and scores remain unchanged.
sed -i \
    's|-device loader,file=bin/ucore.img,addr=0x80200000|-kernel bin/ucore.img|' \
    "$COMPAT_ROOT/tools/grade.compat.sh"

cd "$COMPAT_ROOT"
sh tools/grade.compat.sh

mkdir -p "$SCRIPT_DIR/compatible"
cp .qemu.GRADE_LAB3_CONTEXT.out "$SCRIPT_DIR/compatible/"
cp .qemu.GRADE_LAB3_ILLEGAL.out "$SCRIPT_DIR/compatible/"
cp .qemu.GRADE_LAB3_TIMER.out "$SCRIPT_DIR/compatible/"

echo "raw serial logs remain in: $COMPAT_ROOT"
echo "serial logs copied to: $SCRIPT_DIR/compatible"
