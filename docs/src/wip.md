# Work In Progress Branches

Git can only sync data stored as a commit, so Metro automatically commits uncommitted changes to a temporary WIP branch when `switch` and `sync` are used.
There can be a WIP branch for each base branch in the repository; the name of the
WIP branch is the name of the base branch with the suffix `#wip` appended.
When you switch or sync a branch, Metro will automatically restore the changes in the WIP branch to your working directory.

If Metro is used correctly, these WIP branches are mostly invisible to the user.
However if regular Git commands are used on a repository, the WIP branches may be
left in an invalid state. The `wip` command can be used to resolve such issues.

## `metro wip save`

Saves the contents of the working directory to the current branch's WIP branch,
in similar manner to `switch` and `sync`. Changes should not be made to the base branch until the WIP branch is restored. Syncing will still work in this state.

The command will fail if the WIP branch already exists. The existing WIP branch 
can be deleted with the standard `metro delete branch` command.

## `metro wip restore`

Restores the contents of the WIP branch back to the working directory, overriding 
any existing uncommitted changes. The WIP branch is deleted by this operation.

## `metro wip squash`

Squashes an invalid WIP branch into a single valid commit past master while keeping the contents
of the repository at that commit the same.
