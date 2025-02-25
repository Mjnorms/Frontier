import subprocess
import re
import os

# Change to the Perforce workspace (and Git repo) directory
os.chdir(r"D:\Projects\michael_msi_laptop")

def get_last_commit_message():
    """Get the latest git commit message."""
    result = subprocess.run(["git", "log", "-1", "--pretty=%B"],
                            capture_output=True, text=True)
    return result.stdout.strip()

def extract_last_cl(message):
    """
    Extract the last pushed Perforce changelist number from the commit message.
    Expected format in commit message: 'Last Perforce CL: <number>'
    """
    match = re.search(r"Last Perforce CL:\s*(\d+)", message)
    return int(match.group(1)) if match else None

def get_perforce_changes(last_cl):
    """
    Query Perforce for all submitted changelists after the given changelist.
    If no last_cl is provided, return all submitted changes.
    """
    # Build the range string if we have a last changelist number
    range_arg = f"//...@{last_cl+1},#head" if last_cl is not None else ""
    cmd = ["p4", "changes", "-s", "submitted"]
    if range_arg:
        cmd.append(range_arg)
    result = subprocess.run(cmd, capture_output=True, text=True)
    # Each line might look like:
    # "Change 12345 on 2025/02/20 by user@workspace 'Description...'"
    return result.stdout.strip().splitlines() if result.stdout else []

def compile_change_message(changes):
    """
    Build a commit message from the list of Perforce changes.
    """
    lines = ["Perforce changes since last push:\n"]
    if not changes:
        lines.append("No new changes.")
    else:
        for change in changes:
            lines.append(change)
    return "\n".join(lines)

def main():
    # Get the last git commit message and extract last pushed changelist number
    last_commit = get_last_commit_message()
    last_cl = extract_last_cl(last_commit)
    if last_cl is None:
        print("No previous Perforce changelist info found. Starting from scratch.")
        last_cl = 0

    # Query Perforce for changes after the last known changelist
    changes = get_perforce_changes(last_cl)
    message = compile_change_message(changes)

    # If there are new changes, update the marker to the highest changelist
    if changes:
        # Assuming changes are returned in descending order (newest first)
        # Extract the highest changelist number from the first line
        match = re.search(r"Change\s+(\d+)", changes[0])
        new_last_cl = match.group(1) if match else last_cl
        message += f"\n\nLast Perforce CL: {new_last_cl}"
    else:
        print("No new changes detected; nothing to commit.")
        return

    # Stage any changes in the Git repo (if your tool also updates files)
    subprocess.run(["git", "add", "."])
    # Commit with the constructed message
    commit_result = subprocess.run(["git", "commit", "-m", message],
                                   capture_output=True, text=True)
    print(commit_result.stdout, commit_result.stderr)
    
    # Push to GitHub
    push_result = subprocess.run(["git", "push"], capture_output=True, text=True)
    print(push_result.stdout, push_result.stderr)

if __name__ == "__main__":
    main()