# Switching Branches

## `metro switch <branch>`

The `switch` command allows you to move to a different branch or commit hash.
If you have uncommitted changes in your working directory, these will be stored
in a temporary WIP branch until you return to that branch. Each branch can have
its own WIP branch. You can even switch branches during a merge; the merge will
be saved to the WIP branch and continued when you switch back.

Note that uncommitted changes cannot be saved when switching away from a detached
head (e.g. when you have a commit hash checked out rather than a branch name).
`switch` will not normally allow you to switch away from a detached head with
uncommitted changes; to override this behaviour and switch anyway, use `--force`.
This will cause the uncommitted changes to be lost.

There are also some other conditions under which `switch` will fail, such as
if a WIP branch already exists. These cases should never occur during regular use,
but if you do experience such issues `--force` can be used to switch anyway,
with potential loss of data. For more advanced ways to resolve issues with WIP
branches, see the `wip` command.
