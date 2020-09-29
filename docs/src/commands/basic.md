# Basic Commands

## `metro create [directory]`

Creates a new repository with an initial commit. The repository will be created
in a new subdirectory with the specified name, or the current directory if
none is specified.

## `metro clone <url>`

Clones a remote repository from a specified URL. It is recommended that you use
this instead of `git clone`, as it will initialize the sync cache so
that `metro sync` works correctly.

## `metro commit <message>`

Commits all changes in the working directory with the specified message.
Note that unlike Git you do not need to add changes to the index first.

## `metro patch [message]`

Adds the current changes to the previous commit. Will also change the commit 
message if one is specified.

## `metro delete commit`

Deletes the previous commit, reverting the contents of the working directory
to match the commit before the deleted commit. If `--soft` is specified the
working directory is left unchanged.

## `metro delete branch <branch>`

Deletes the specified branch. If you delete the current branch, you will be moved
to another branch.

## `metro branch <name>`

Creates a new branch with the specified name, pointing to the head of the current
branch. Automatically switches to the new branch upon creation.

## `metro info`

Prints some information about the current state of the repository, including the
current branch and uncommitted changes.

## `metro absorb <branch>`

Merges another branch into the current branch. May result in conflicts that need
to be resolved manually before running `metro resolve`.

## `metro resolve`

Marks all merge conflicts as resolved and commits the changes.

## `metro list <commits/branches>`

Lists all commits or branches in the repository.

## `metro rename <branch-1> [branch-2]`

Renames the specified branch. `branch-1` is the current branch name, and `branch-2` is the new name. If only one name is given the current branch is renamed.