# Handling branches

Branches allow you to (some explanation of branches here...)

## Creating branches

Branches are created using the `metro branch` command:

```bash
metro branch "feature/documentation"
```

## Switching branches

Branches can be switched to by using the `metro switch` command. Metro automatically saves the current state of your working branch before switching and automatically restores the saved state when you return to that branch again.

```bash
metro switch "feature/documentation"
```

## Deleting branches

Branches can be deleted using the `metro delete` command:

```bash
metro delete "feature/documentation"
```

> **Warning:**
> 
> Using this command will delete **ALL** data from the provided branch. This cannot be undone.

## Branch information

Information about the current branch can be viewed by using `metro info`
