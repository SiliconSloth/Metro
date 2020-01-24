# Absorbing code

The contents of a branch can be absorbed into another branch by using the `metro absorb` command. This combines the contents of a branch into another branch if possible. If a conflict arises, Metro will put the branch into "merged" mode, where it can be resolved using `metro resolve`.

### Example:

Say we have a branch _feature/documentation_ which includes some changes about documentation. We want to include the contents of this branch into our _master_ branch. Firstly, we make sure we are in the _master_ branch by using `metro switch`:

```bash
metro switch "master"
```

We can then absorb the contents of _feature/documentation_ into the current branch:
```bash
metro absorb "feature/documentation"
```
