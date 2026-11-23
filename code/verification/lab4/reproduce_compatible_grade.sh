#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CODE_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)
COMPAT_ROOT=$(mktemp -d /tmp/lab4-grade-compatible.XXXXXX)

echo "isolated grading tree: $COMPAT_ROOT"

tar -C "$CODE_ROOT" \
    --exclude=.git \
    --exclude=.vscode \
    --exclude=vcs \
    --exclude=bin \
    --exclude=obj \
    --exclude=verification \
    -cf - . | tar -C "$COMPAT_ROOT" -xf -

python3 "$CODE_ROOT/tools/inject.py" "$COMPAT_ROOT"
cp "$COMPAT_ROOT/tools/grade.sh" "$COMPAT_ROOT/tools/grade.compat.sh"

# QEMU 7's generic loader copies the raw image but does not populate the
# OpenSBI FW_DYNAMIC next-stage address.  This temporary copy changes only
# the launch parameter; test injection, assertions, scores, and timeouts stay
# identical to the teacher-provided grade.sh.
sed -i \
    's|-device loader,file=bin/ucore.img,addr=0x80200000|-kernel bin/ucore.img|' \
    "$COMPAT_ROOT/tools/grade.compat.sh"

cd "$COMPAT_ROOT"
sh tools/grade.compat.sh

cp .qemu.out "$SCRIPT_DIR/compatible_qemu_serial.log"

echo "raw serial log copied to: $SCRIPT_DIR/compatible_qemu_serial.log"
echo "isolated grading tree retained at: $COMPAT_ROOT"
