# Making commits

## Creating a Commit
Commits are created using the `metro commit` command. This creates a new commit for the current working branch which includes the contents of any changed files automatically.

```bash
metro commit "Message"
```

> **Git Equivalent**
> 
> `metro commit` performs a combination of adding files, as well as creating a commit.
> ```
> git add -A 
> git commit -m "Message"
> ```

## Patching a Commit
Commit messages can be patched in order to update the latest commit with local changes before it has been synced. This is performed using the `metro patch` command. This rewrites the previous commit with the contents of the local branch.

```bash
metro patch
```

## Deleting a Commit
You can also delete a commit by using delete:
```bash
metro delete commit
```

Which is the alternative to `reset` in Git:
```bash
git reset --hard HEAD~1
```

