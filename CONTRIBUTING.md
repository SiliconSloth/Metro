# Contributing to Metro
Thanks for taking the time to improve Metro!
This doc should act as a basic guideline for how you can help out

## Table of Contents
[The Team](#the-team)<br><br>
[Contact Us](#contact-us)<br><br>
[Problems and Suggestions:](#problems-and-suggestions)
 - [Reporting Bugs](#reporting-bugs)
 - [Adding Commands](#adding-commands)
 - [Adding/Changing Features](#adding-changing-features)<br>
 
[Editing the Code](#editing-the-code):
 - [Getting Started with Metro](#getting-started-with-metro)
 - [Creating a Good Pull Request](#creating-a-good-pull-request)
 - [Regarding Documentation](#regarding-documentation)
 - [Regarding the Build Process](#regarding-the-build-process)
 
## The Team
We're all more than glad to have you help on this open source project.

Currently there are two of us actively working on the project:
 - SiliconSloth, or Michael Tyler
 - Black-Photon, or Joseph Keane

## Contact Us
If you can't find what you're looking for on here, or need to talk to us beyond messages in Issues and Pull Requests, you can contact us [here](https://blackphoton.wufoo.com/forms/qb9o8291vuc25y/)

## Problems and Suggestions
Have you found a problem in Metro, but can't fix it? Or had an idea you don't know how to program?

You can submit suggestions for enhancements or bugs using the Issue system.
### Reporting Bugs
If you find a bug, you should make a new issue. Here the template will give you a selection of fields to fill out, as well as suggested values.

It's important to fill out all of the fields to the best of your ability, otherwise it will be difficult for us to find a fix. You can see an example of a good issue [here](https://github.com/SiliconSloth/Metro/issues/21), which explains the issue, what was expected, how to reproduce it, what version of Metro the issue occurred on, and what Operating System the issue occurred on.

A good way to go about issues is that more information is better. The more detail is there, the more likely we can find a fix.

Be careful to make sure the issue you are reporting hasn't already been reported. You can find current issues using the `bug` tag in issue search.

### Adding Commands
If you have an idea for a command, you should make a new issue. If the command is entirely new, for example `commit` or `list branches`, you should make the title `Command: Command Name`, otherwise if something like `--pull` or a similar option, the title should best describe the command.

You should replace the default fields with `Command Description` and optionally `Expected Behaviour`. Command Description should explain what the command should do, and why it is useful. Expected Behaviour should give an example of what a common use of this command might look at. [Here](https://github.com/SiliconSloth/Metro/issues/24), you can see an example of a good command, which explains what the command should do, why it should exist and how it might be used.

Be careful to make sure the command you are suggesting doesn't already exist, and isn't already suggested. You can find suggested commands using the `command` tag in issue search.

### Adding/Changing Features
If you have an idea for a feature that isn't directly a command, you should make a new issue. The issue should have a title which describes the feature.

You should replace the default fields with `Feature Description` and optionally `Expected Behaviour`. Feature Description should explain what the feature should do, and why it is useful. Expected Behaviour should give an example of how this feature would help use of Metro. [Here](https://github.com/SiliconSloth/Metro/issues/31), you can see an example of a good feature request, which explains what the feature should do, an example of its use, and detail of how the feature should work within what already exists in Metro.

Be careful to make sure the feature you are suggesting doesn't already exist, and isn't already suggested. You can find suggested features using the `enhancement` tag in issue search.


## Editing the Code
Did you find a problem, or have an idea and know how to fix it? You can edit the code yourself and submit a Pull Request for us to review.

Be aware that in Github, you can make pull requests for other projects by forking the project, making your changes as normal to the forked version, and submitting a pull request from the fork to the initial project.

In addition, the team will have the final say of when and if the pull request will be merged in, depending on its usefulness, implementation and code quality. Pull requests need not be perfect however - we'll look over the code and comment on anything we think is lacking as long as the idea behind the pull request is useful.

### Getting Started with Metro
To get started using Metro, you're first going to want to get the project set up and build on your machine. You can follow the build process [here](https://siliconsloth.github.io/Metro/building/building.html) to do this. If it build successfully, you're all set.

Metro primarily uses the LibGit2 library for most of its functions, being a Git wrapper. Thus, it might be useful to use the documentation [here](https://libgit2.org/) during development. There may be many subtleties of Metro to get used to. If you have questions, you can always contact us on the form [here](https://blackphoton.wufoo.com/forms/qb9o8291vuc25y/), or create a draft pull request, asking questions in the comments if you feel that is more appropriate.

### Creating a Good Pull Request
Once you feel your code is ready to be merged into `master`, you can submit a pull request. This should explain everything that is changed, as well as a description of why the pull request should exist if not obvious, and notes of things that may affect developers if the request is merged.

You can find a good example of a pull request [here](https://github.com/SiliconSloth/Metro/pull/28), which describes what the request does, as well as listing the specific improvements made by that request.

Be sure the pull request doesn't already exist before submitting, and also check the issues - you might be able to close one or two of them with your pull request. You can link them in by putting `closes #12` in the description, where `#12` is the number for the issue being closed.

### Regarding Documentation
The documentation is built using [mdbook](https://github.com/rust-lang/mdBook), and must be built before it will be updated. You can do this by first making the changes in the `.md` files in the `docs/src` directory. Once done, run `mdbook build`, which should compile into a `book` directory. Move the files up to `docs` and delete the `book` directory once done.

You should ideally add documentation for any pull requested features or commands. In addition, you can make a request for purely documentation if you feel like it is lacking.

### Regarding the Build Process
Finally, it's possible you will want to try and improve the build process. It currently uses CMake to build, with two git submodules for two main dependancies. Take the time to understand why it is the way it is before trying to improve it.

In addition, we will NOT be changing the build process to use Shared Libraries. The reason for this is that we would like Metro to be portable, and having a static single file is the most convenient for the most people. Ideally, and build improvement should not add additional dependancies or use any build process other than CMake. However, you may add dependancies for changes to the main codebase if they are needed to implement the feature of bug fix.
