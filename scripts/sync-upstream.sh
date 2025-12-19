#!/usr/bin/env bash
set -euo pipefail

# Sync upstream triton third_party/proton commits onto the local "mirror" branch.
#
# The mirror branch is a clean 1:1 copy of upstream's third_party/proton,
# with no local modifications. This makes merging into main a normal git merge.
#
# Usage:
#   ./scripts/sync-upstream.sh [-n] [-c N]
#
#   -n       Dry run: list pending commits but don't apply anything
#   -c N     Apply at most N commits (default: all)

UPSTREAM_REPO="${UPSTREAM_REPO:-$HOME/src/triton}"
UPSTREAM_SUBDIR="third_party/proton"
FORK_POINT="d25ba85ae"  # SHA from "Initial proton copy from triton@d25ba85ae"
MIRROR_BRANCH="mirror"

DRY_RUN=0
MAX_COMMITS=0  # 0 means unlimited

while getopts "nc:" opt; do
    case "$opt" in
        n) DRY_RUN=1 ;;
        c) MAX_COMMITS="$OPTARG" ;;
        *) echo "Usage: $0 [-n] [-c N]" >&2; exit 1 ;;
    esac
done

# Validate upstream repo
if [ ! -d "$UPSTREAM_REPO/.git" ]; then
    echo "ERROR: Upstream repo not found at $UPSTREAM_REPO" >&2
    echo "Set UPSTREAM_REPO env var to the triton checkout path." >&2
    exit 1
fi

# Ensure mirror branch exists
if ! git rev-parse --verify "$MIRROR_BRANCH" >/dev/null 2>&1; then
    echo "ERROR: Branch '$MIRROR_BRANCH' does not exist." >&2
    echo "Create it from the initial commit first:" >&2
    echo "  git branch $MIRROR_BRANCH <initial-commit>" >&2
    exit 1
fi

# Find the last upstream SHA on the mirror branch by scanning commit messages
# for "upstream: <sha>" lines.
last_synced="$FORK_POINT"
while IFS= read -r msg; do
    sha=$(echo "$msg" | grep -oP '(?<=upstream: )[0-9a-f]{9,}' | head -1 || true)
    if [ -n "$sha" ]; then
        last_synced="$sha"
    fi
done < <(git log "$MIRROR_BRANCH" --format="%b" --reverse)

echo "Upstream repo:    $UPSTREAM_REPO"
echo "Mirror branch:    $MIRROR_BRANCH"
echo "Fork point:       $FORK_POINT"
echo "Last synced:      $last_synced"

# Get the list of upstream commits touching third_party/proton since last sync
mapfile -t commits < <(
    git -C "$UPSTREAM_REPO" log --format="%H" --reverse "$last_synced..HEAD" -- "$UPSTREAM_SUBDIR/"
)

# Filter: only commits that actually change files under third_party/proton/
filtered_commits=()
for sha in "${commits[@]}"; do
    proton_files=$(git -C "$UPSTREAM_REPO" diff-tree --no-commit-id -r --name-only "$sha" -- "$UPSTREAM_SUBDIR/" 2>/dev/null | head -1)
    if [ -n "$proton_files" ]; then
        filtered_commits+=("$sha")
    fi
done

total=${#filtered_commits[@]}
echo "Pending commits:  $total"
echo ""

if [ "$total" -eq 0 ]; then
    echo "Already up to date."
    exit 0
fi

# Determine how many to apply
apply_count="$total"
if [ "$MAX_COMMITS" -gt 0 ] && [ "$MAX_COMMITS" -lt "$total" ]; then
    apply_count="$MAX_COMMITS"
fi

# List them
echo "--- Pending upstream commits ---"
for i in "${!filtered_commits[@]}"; do
    sha="${filtered_commits[$i]}"
    oneline=$(git -C "$UPSTREAM_REPO" log --format="%h %s" -1 "$sha")
    marker="   "
    if [ "$i" -lt "$apply_count" ]; then
        marker=">>>"
    fi
    printf "%3d. %s %s\n" $((i+1)) "$marker" "$oneline"
done
echo ""

if [ "$DRY_RUN" -eq 1 ]; then
    echo "(dry run, exiting)"
    exit 0
fi

# Remember where we started so we can switch back
original_branch=$(git symbolic-ref --short HEAD 2>/dev/null || git rev-parse HEAD)

# Switch to mirror branch
git checkout "$MIRROR_BRANCH" --quiet

# Trap to switch back on any exit
cleanup() {
    git checkout "$original_branch" --quiet 2>/dev/null || true
}
trap cleanup EXIT

echo "Applying $apply_count of $total commits onto $MIRROR_BRANCH..."
echo ""

applied=0
for i in $(seq 0 $((apply_count - 1))); do
    sha="${filtered_commits[$i]}"
    short=$(git -C "$UPSTREAM_REPO" log --format="%h" -1 "$sha")
    subject=$(git -C "$UPSTREAM_REPO" log --format="%s" -1 "$sha")
    author=$(git -C "$UPSTREAM_REPO" log --format="%an <%ae>" -1 "$sha")
    author_date=$(git -C "$UPSTREAM_REPO" log --format="%ai" -1 "$sha")

    printf "[%d/%d] %s %s\n" $((i+1)) "$apply_count" "$short" "$subject"

    # Generate a patch for only third_party/proton/ files, strip the prefix
    patch=$(git -C "$UPSTREAM_REPO" diff-tree -p "$sha" -- "$UPSTREAM_SUBDIR/" | \
        sed "s|a/$UPSTREAM_SUBDIR/|a/|g; s|b/$UPSTREAM_SUBDIR/|b/|g")

    if [ -z "$patch" ]; then
        echo "  (no proton diff, skipping)"
        continue
    fi

    # Try to apply
    if ! echo "$patch" | git apply --check 2>/dev/null; then
        echo ""
        echo "=== CONFLICT applying $short ==="
        echo "Upstream commit: $sha"
        echo "Subject: $subject"
        echo ""
        echo "Conflicting files:"
        echo "$patch" | git apply --check 2>&1 | sed 's/^/  /' | head -20
        echo ""
        echo "Bailing out. Applied $applied of $apply_count commits before conflict."
        exit 1
    fi

    echo "$patch" | git apply

    # Commit with upstream reference
    git add -A
    commit_msg="$subject

upstream: $sha"

    GIT_AUTHOR_NAME="$(echo "$author" | sed 's/ <.*>//')" \
    GIT_AUTHOR_EMAIL="$(echo "$author" | grep -oP '(?<=<).*(?=>)')" \
    GIT_AUTHOR_DATE="$author_date" \
    git commit -m "$commit_msg" --allow-empty >/dev/null

    echo "  committed."
    applied=$((applied + 1))
done

echo ""
echo "Done. Applied $applied commits to $MIRROR_BRANCH."
echo "Last synced upstream SHA: ${filtered_commits[$((apply_count - 1))]}"
echo ""
echo "To merge into main:"
echo "  git merge $MIRROR_BRANCH"
