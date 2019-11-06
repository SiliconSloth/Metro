# Creating a repository

To create a repository with metro, you use the create command:
```
metro create
```
Or alternatively to include origin:
```
metro create git@github.com:SiliconSloth/Metro.git
```

> **Git Equivalent**
> 
> `metro create` is similar to `git init`. In particular, it performs the following git actions:
> ```bash
> git init
> git commit -m "Initial commit"
> git remote add origin git@github.com:SiliconSloth/Metro.git
> ```
