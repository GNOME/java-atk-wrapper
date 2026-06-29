#!/usr/bin/env bash

set -euo pipefail

STYLE_FILE="${CI_PROJECT_DIR:-$(pwd)}/jni/src/.clang-format"
UPSTREAM_URL="https://gitlab.gnome.org/GNOME/java-atk-wrapper.git"
TARGET_BRANCH="${CI_MERGE_REQUEST_TARGET_BRANCH_NAME:-${CI_DEFAULT_BRANCH:-master}}"

if [ ! -f "$STYLE_FILE" ]; then
    echo "Style file not found: $STYLE_FILE"
    exit 1
fi

UPSTREAM_REMOTE=$(git remote -v | awk -v url="$UPSTREAM_URL" '$2 == url { print $1; exit }')

if [ -n "$UPSTREAM_REMOTE" ]; then
    echo "Reusing existing remote: ${UPSTREAM_REMOTE}"
    ORIGIN="$UPSTREAM_REMOTE"
else
    echo "Adding upstream remote: ${UPSTREAM_URL}"
    git remote add upstream "$UPSTREAM_URL"
    ORIGIN="upstream"
fi

git fetch "$ORIGIN"

CLANG_FORMAT_DIFF="${CLANG_FORMAT_DIFF:-clang-format-diff}"

if ! command -v "$CLANG_FORMAT_DIFF" >/dev/null 2>&1; then
    for version in 21 20 19 18 17 16 15; do
        if command -v "clang-format-diff-$version" >/dev/null 2>&1; then
            CLANG_FORMAT_DIFF="clang-format-diff-$version"
            break
        fi
    done
fi

if ! command -v "$CLANG_FORMAT_DIFF" >/dev/null 2>&1; then
    echo "clang-format-diff was not found"
    exit 1
fi

target_ref="${ORIGIN}/${TARGET_BRANCH}"

if ! git rev-parse --verify "$target_ref" >/dev/null 2>&1; then
    echo "Target branch not found: $target_ref"
    exit 1
fi

newest_common_ancestor_sha=$(
    set +o pipefail
    diff --old-line-format='' --new-line-format='' \
        <(git rev-list --first-parent "$target_ref") \
        <(git rev-list --first-parent HEAD) \
        | head -1
)

if [ -z "$newest_common_ancestor_sha" ]; then
    echo "Could not determine common ancestor with $target_ref"
    exit 1
fi

if ! git diff -U0 --no-color "$newest_common_ancestor_sha" -- jni/src \
    | "$CLANG_FORMAT_DIFF" \
        -p1 \
        -regex '^jni/src/.*\.(c|h)$' \
        -style "file:${STYLE_FILE}" \
    > format-diff.log; then
    cat format-diff.log || true
    echo
    echo "clang-format-diff failed"
    exit 1
fi

if [ -s format-diff.log ]; then
    cat format-diff.log
    echo
    echo "PLEASE FIX THE FORMATTING OF THE SOURCE CODE ABOVE"
    exit 1
fi