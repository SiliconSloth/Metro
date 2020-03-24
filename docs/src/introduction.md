![Banner](https://raw.githubusercontent.com/SiliconSloth/Metro/master/banner.png)
## What is Metro?
Metro is an alternative to the `git` command that lets you work with your Git repos in a more simple and flexible way,
and makes it much easier to sync unfinished work between machines. Having learned from Git's shortcomings,
Metro has two central design philosophies that differentiate it from pure Git:
1. The repository is a tool to work on and transfer code between computers, not just an archive for finished code:
   Metro stores the very latest version of your code in the repo, allowing you to easily transfer it between machines
   without making a half-finished commit every time you have to leave in a hurry.
2. When things conflict, create new branches to resolve the issue rather than showing arcane error messages:
   Git is only meant for archiving finished code, so of course we wouldn't want several ugly conflicting branches cluttering up 
   our beautiful archive; instead Git expects the issue to be fixed _before_ changing the repo, which often results in 
   users scouring Stack Overflow to find a complicated solution that involves internal parts of Git they've never heard 
   of. Metro, on the other hand, doesn't have this issue; the repository is a tool for working on the code, so why not fix 
   issues there too? When conflicts occur Metro will offer to make separate branches for each version, making it simple 
   and intuitive to fix the problem yourself. And you always are given the option to resolve quickly by absorbing if you prefer.

## Switching from Git?
See [this page](./metro-vs-git.md) explaining the differences between Metro and Git in more depth.
