#!/bin/sh
set -u

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CODE_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)
ORIGINAL_ROOT=$(mktemp -d /tmp/lab4-grade-original.XXXXXX)

echo "isolated grading tree: $ORIGINAL_ROOT"

tar -C "$CODE_ROOT" \
    --exclude=.git \
    --exclude=.vscode \
    --exclude=vcs \
    --exclude=bin \
    --exclude=obj \
    --exclude=verification \
    -cf - . | tar -C "$ORIGINAL_ROOT" -xf -

cd "$ORIGINAL_ROOT"
if make grade; then
    status=0
else
    status=$?
fi

echo "isolated grading tree retained at: $ORIGINAL_ROOT"
exit "$status"
