# Metro vs Git

## What's wrong with Git?
### Gets used for file syncing when it probably shouldn't be
Git repositories are meant to be a place to store finished parts of your code, in a manner that makes it easy for multiple people to work on different features in parallel. While Git is an excellent choice for this purpose, we feel that its approach is outdated and fails to effectively fulfil the needs of the modern programmer.

Git was designed for the days when programmers would write all their code at one computer, and only commit and push it to the repo once they had finished a feature completely. However, nowadays it is common for people to use many different computers (laptop, work computer, home desktop...) and want to switch machines partway through writing a feature. The correct way to solve this problem is probably to use a separate directory syncing tool to transfer the work-in-progress code, using the Git repo only for completed commits, however in practice most people just end up committing random half-finished changes when they need to leave in a hurry. This can lead to Git repositories filled with lots of poor-quality commits, with actual completed feature commits few and far between.

In contrast, Metro is _made_ for syncing unfinished code between computers. Rather than having to commit halfway through a semicolon every time you leave the room, Metro syncs the current contents of the working directory into the repo so that you can easily transfer your work to another machine without leaving loads of broken commits in the repo. Metro treats the repo as a living, changing thing that includes the very latest versions of your code, rather than just the finished archived stuff.

### When it breaks, it **BREAKS**
When you know the basics of Git (and remember to add your files before committing, and to push after committing, and to pull before working on the code) it is a simple and easy system to use. Unfortunately, when something goes wrong, it really does go wrong. 

Here is common scene when trying to commit a file:
```bash
> git add -A

> git commit -m "Changed file"
[master 6c3a3de] Changed file
 1 file changed, 1 insertion(+), 1 deletion(-)

> git push
To github.com:Black-Photon/Git-Testing.git
 ! [rejected]        master -> master (fetch first)
error: failed to push some refs to 'git@github.com:Black-Photon/Git-Testing.git'
hint: Updates were rejected because the remote contains work that you do
hint: not have locally. This is usually caused by another repository pushing
hint: to the same ref. You may want to first integrate the remote changes
hint: (e.g., 'git pull ...') before pushing again.
hint: See the 'Note about fast-forwards' in 'git push --help' for details.

> git pull
<Goes to vim to edit commit message>
remote: Enumerating objects: 5, done.
remote: Counting objects: 100% (5/5), done.
remote: Compressing objects: 100% (2/2), done.
remote: Total 3 (delta 0), reused 0 (delta 0), pack-reused 0
Unpacking objects: 100% (3/3), done.
From github.com:Black-Photon/Git-Testing
   4436cfd..f88f912  master     -> origin/master
Merge made by the 'recursive' strategy.
 Readme.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)

> git push
Enumerating objects: 9, done.
Counting objects: 100% (8/8), done.
Delta compression using up to 4 threads
Compressing objects: 100% (4/4), done.
Writing objects: 100% (5/5), 588 bytes | 588.00 KiB/s, done.
Total 5 (delta 0), reused 1 (delta 0)
To github.com:Black-Photon/Git-Testing.git
   f88f912..4129e5e  master -> master
```
Having to do so much is difficult to learn and not at all intuitive. Why do you need to add files? What does -m do? In what order do I pull and push? It also takes up time working out all the commands each time. Now what if that was:
```bash
> metro commit "Changed file"
Committed Successfully

> metro sync
Conflict Found:
[0] Absorb local master line into remote master line
[1] Move local changes to new line master-local
> 0
Successfully Absorbed Changes
```

## Features
We've tried to reduce the number of inconveniences you encounter with git as much as possible while still keeping the functionality you need:
* As a Git wrapper, Metro is fully compatible with all Git tools
* No staging area or adding files - all files are automatically added to commits
* Push and pull combined into one command to sync the current line with the remote
* Simple method to delete or return to past commits
* Patching last commit with current work to fix mistakes or reduce small commits
* Ignores files recently added to .gitignore rather than deleting them from other machines that then sync
