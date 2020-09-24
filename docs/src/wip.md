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

Replaces the contents of the working directory with the contents of the WIP commit,
overriding any existing uncommitted changes. This also deletes the WIP branch.

## `metro wip squash`

Commits the content of the head of the WIP branch to a single commit that is a child
of the base branch's head. This is useful if commits have been made on top of the WIP
branch, rendering it invalid; `restore` allows all these commits to be converted into
uncommitted changes in a single valid WIP commit.
