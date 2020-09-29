# Syncing with a Remote Repository

## `metro sync`

The `sync` command synchronizes all your local branches with the remote repository,
by either pushing or pulling each branch so that all changes since the last sync are
retained on both sides. If a branch has been changed both locally and remotely since
the last sync, Metro automatically creates a new branch to store one of these
versions, so that you can merge or discard the changes at your leisure. Newly created
or deleted branches will also be synced too, as will uncommitted changes (including
ongoing merges) in the working directory.

Specify `--pull` to only pull branches, without pushing any local changes to the 
remote repository. Specify `--push` to only push branches to the remote, without
pulling any changes. `sync --push` will fail if there are branch conflicts.

The `sync` command may fail if the repository has invalid WIP branches. If you
experience issues pertaining to WIP branches, try using the `wip` command to resolve 
them.