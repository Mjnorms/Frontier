#!/bin/bash

# Determine the script directory dynamically
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Workspace root is assumed to be the parent of the script directory
WORKSPACE="$(dirname "$SCRIPT_DIR")"

LOG_FILE="/path/to/workspace/p4v_scripts/sync_log.txt"

cd "$WORKSPACE" || exit 1

# Ensure the Perforce workspace is up-to-date
p4 sync >> "$LOG_FILE" 2>&1

# Add all changes to Git
git add . >> "$LOG_FILE" 2>&1

# Retrieve changelist description
CHANGE_DESCRIPTION=$(p4 describe -s "$1" | sed -n '2,$p')  # Exclude the first line (Changelist info)

# Commit the changes with the Perforce changelist number
COMMIT_MESSAGE="Sync from Perforce: Changelist $1

$CHANGE_DESCRIPTION"
git commit -m "$COMMIT_MESSAGE" >> "$LOG_FILE" 2>&1

# Push to GitHub
git push origin main >> "$LOG_FILE" 2>&1

# Log the action (optional)
echo "$(date) - Pushed Perforce changelist $1 to GitHub" >> "$LOG_FILE" 2>&1

exit 0
