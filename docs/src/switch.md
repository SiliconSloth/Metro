# Switching Branches

## `metro switch <branch>`

The `switch` command allows you to move to a different branch or commit hash.
If you have uncommitted changes in your working directory, these will be stored
in a temporary `#wip` branch until you return to the previous branch.

The command will fail if there's already a WIP when it tries to switch branch,
but you can bypass this using `--force` which will discard the current changes.
