# Work In Progress Branches

## What is a WIP Branch?

Git can only sync data stored as a commit, so Metro automatically commits the changes to a
branch `<branch-name>#wip`. When retrieving such a commit, it will automatically detect the WIP
branch and apply the changes to your repository. Different branches can have different WIP's,
and they'll automatically be checked out when using `metro switch`. However they will *not* be
checked out when using Git.

The WIP's can become invalid if it is not applied and more changes or commits are made.

## Controlling the WIP

Oftentimes you'll need to directly control and fix the WIP for a branch. This can be done using
the `metro wip` command.

### `metro wip save`

Saves the contents of the working directory to a WIP commit in the appropriate branch. Changes
should NOT be made to the base branch until the WIP is restored. Syncing will still work in
this state.

The command will fail if the WIP already exists. The existing WIP can be deleted with the
standard `metro delete branch` command.

### `metro wip restore`

Restores the WIP back to the working directory, overriding any changes since the last commit with
those stored on the WIP.

### `metro wip squash`

Squashes an invalid WIP branch into a single valid commit past master while keeping the contents
of the repository at that commit the same.
