#!/usr/bin/env bats

setup_file() {
  cd $BATS_TMPDIR
  SESSION_DIR="metro_test/$(date +%s)"
  mkdir -p $SESSION_DIR
  cd $SESSION_DIR
}

setup() {
  mkdir -p $BATS_TEST_NAME
  cd $BATS_TEST_NAME
}

# ~~~ Test Clone ~~~

@test "Clone empty repo" {
  mkdir -p create remote/repo
  cd remote/repo
  echo "$ git init"
  git init

  cd ../..
  mkdir local
  cd local
  echo "$ metro clone git@remote"
  metro clone ../remote/repo
  cd repo

  echo "$ git log"
  run git log
  [[ "${lines[0]}" == "fatal: your current branch 'master' does not have any commits yet" ]]
}

@test "Clone repo with initial commit only" {
  echo "$ git init remote & git commit --allow-empty -m \"Initial Commit\""
  git init remote/repo
  cd remote/repo
  git commit --allow-empty -m "Initial Commit"
  cd ../..

  mkdir local
  cd local
  echo "$ metro clone git@remote"
  metro clone ../remote/repo
  cd repo

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Initial Commit"* ]]
}

@test "Clone repo with one commit" {
  echo "$ git init remote & echo \"Remote file content\" > remote.txt & git commit -am \"Test commit message\""
  git init remote/repo
  cd remote/repo
  echo "Remote file content" > remote.txt
  git add -A
  git commit -m "Remote commit message"

  cd ../..
  mkdir local
  cd local
  echo "$ metro clone git@remote"
  metro clone ../remote/repo
  cd repo

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Remote commit message"* ]]
}

# ~~~ Test Sync ~~~

@test "Sync commit" {
  echo "$ git init remote"
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  echo "$ cd ~/local1"
  echo "$ git clone git@remote & echo \"local1 file content\" > local1.txt & git commit -am \"Local1 commit message\""
  git clone ../remote/repo
  cd repo
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "Local1 commit message"

  cd ../..
  mkdir local2
  cd local2
  echo "$ cd ~/local2"
  echo "$ git clone git@remote"
  git clone ../remote/repo

  cd ../local1/repo
  echo "$ cd ~/local1"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Local1 commit message"* ]]
}

@test "Sync WIP commit" {
  echo "$ git init remote"
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  echo "$ cd ~/local1"
  echo "$ git clone git@remote & echo \"local1 file content 1\" > local1-1.txt & git commit -am \"Local1 commit message\""
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message"
  echo "$ echo \"local1 file content 2\" > local1-2.txt"
  echo "local1 file content 2" > local1-2.txt

  cd ../..
  mkdir local2
  cd local2
  echo "$ cd ~/local2"
  echo "$ git clone git@remote"
  git clone ../remote/repo

  cd ../local1/repo
  echo "$ cd ~/local1"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  echo "$ ls"
  ls
  run ls
  [[ "${lines[1]}" == *"local1-2.txt"* ]]
}

@test "Sync reset commit" {
  echo "Mark 1"
  git init remote/repo --bare
  mkdir local1
  cd local1

  echo "Mark 2"
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"
  echo "local1 file content 2" > local1-2.txt
  git add -A
  git commit -m "Local1 commit message 2"

  echo "Mark 3"
  cd ../..
  mkdir local2
  cd local2
  git clone ../remote/repo

  echo "Mark 4"
  cd ../local1/repo
  metro sync
  echo "Mark 5"
  cd ../../local2/repo
  metro sync

  echo "Mark 6"
  cd ../../local1/repo
  git reset HEAD~ --hard
  echo "Mark 7"
  metro sync
  echo "Mark 8"
  cd ../../local2/repo
  metro sync

  echo "Mark 9"
  git log
  run git log
  [[ "${lines[3]}" != *"Local1 commit message 2"* ]]
}

@test "Sync amended commit" {
  echo "Mark 1"
  git init remote/repo --bare
  mkdir local1
  cd local1

  echo "Mark 2"
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"
  echo "local1 file content 2" > local1-2.txt

  echo "Mark 3"
  cd ../..
  mkdir local2
  cd local2
  git clone ../remote/repo

  echo "Mark 4"
  cd ../local1/repo
  metro sync
  echo "Mark 5"
  cd ../../local2/repo
  metro sync

  echo "Mark 6"
  cd ../../local1/repo
  git add -A
  git commit --amend -m "Local1 commit message 2"

  echo "Mark 7"
  metro sync
  cd ../../local2/repo
  metro sync

  echo "Mark 8"
  git log
  run git log
  [[ "${lines[3]}" == *"Local1 commit message 2"* ]]

  echo "Mark 9"
  ls
  run ls
  [[ "${lines[1]}" == *"local1-2.txt"* ]]
}

@test "Sync committed WIP" {
  echo "$ git init remote"
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  echo "$ cd ~/local1"
  echo "$ git clone git@remote & echo \"local1 file content 1\" > local1-1.txt & git commit -am \"Local1 commit message\""
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"
  echo "$ echo \"local1 file content 2\" > local1-2.txt"
  echo "local1 file content 2" > local1-2.txt

  cd ../..
  mkdir local2
  cd local2
  echo "$ cd ~/local2"
  echo "$ git clone git@remote"
  git clone ../remote/repo

  echo "$ cd ~/local1"
  cd ../local1/repo
  echo "$ metro sync"
  metro sync
  echo "$ cd ~/local2"
  cd ../../local2/repo
  echo "$ metro sync"
  metro sync

  cd ../../local1/repo
  echo "$ cd ~/local1"
  echo "$ git commit -am \"Local1 commit message 2\""
  git add -A
  git commit -m "Local1 commit message 2"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "$output" == "* master" ]]

  echo "$ git log master"
  git log master
  run git log master
  [[ "${lines[3]}" == *"Local1 commit message 2"* ]]
  [[ "${lines[7]}" == *"Local1 commit message 1"* ]]
}

@test "Sync different merge-able commits" {
  echo "$ git init remote"
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  echo "$ cd ~/local1"
  echo "$ git clone git@remote & echo \"local1 file content 1\" > local1-1.txt & git commit -am \"Local1 commit message\""
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"

  cd ../..
  mkdir local2
  cd local2
  echo "$ cd ~/local2"
  echo "$ git clone git@remote"
  git clone ../remote/repo

  echo "$ cd ~/local1"
  cd ../local1/repo
  echo "$ metro sync"
  metro sync
  echo "$ cd ~/local2"
  cd ../../local2/repo
  echo "$ metro sync"
  metro sync

  cd ../../local1/repo
  echo "$ cd ~/local1"
  echo "$ echo \"local1 file content 2\" > local1-2.txt & git commit -am \"Local1 commit message 2\""
  echo "local1 file content 2" > local1-2.txt
  git add -A
  git commit -m "Local1 commit message 2"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ echo \"local2 file content\" > local2.txt & git commit -am \"Local2 commit message\""
  echo "local2 file content" > local2.txt
  git add -A
  git commit -m "Local2 commit message"
  echo "$ metro sync"
  metro sync

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* master#1" ]]

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Local2 commit message"* ]]
  echo "$ git log master"
  git log master
  run git log master
  [[ "${lines[3]}" == *"Local1 commit message 2"* ]]
}

@test "Sync updated WIP" {
  echo "$ git init remote"
  git init remote/repo --bare

  mkdir local1
  cd local1
  echo "$ cd ~/local1"
  echo "$ git clone git@remote & echo \"local1 file content 1\" > local1-1.txt & git commit -am \"Local1 commit message\""
  git clone ../remote/repo
  cd repo
  echo "local1 file content 1" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"

  cd ../..
  mkdir local2
  cd local2
  echo "$ cd ~/local2"
  echo "$ git clone git@remote"
  git clone ../remote/repo

  echo "$ cd ~/local1"
  cd ../local1/repo
  echo "$ metro sync"
  metro sync
  echo "$ cd ~/local2"
  cd ../../local2/repo
  echo "$ metro sync"
  metro sync

  cd ../../local1/repo
  echo "$ cd ~/local1"
  echo "$ echo \"local1 file content 2\" > local1-2.txt"
  echo "local1 file content 2" > local1-2.txt
  git add -A
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ echo \"local2 file content\" > local2.txt"
  echo "local2 file content" > local2.txt
  git add -A
  echo "$ metro sync"
  metro sync

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* master#1" ]]
  [[ "${lines[2]}" == "  master#wip" ]]

  [[ -f "local2.txt" ]]

  metro switch master

  [[ -f "local1-2.txt" ]]
}

@test "Sync WIP with no branches" {
  git init remote/repo --bare

  echo "Mark 1"
  mkdir local1
  cd local1
  echo "Mark 2"
  git clone ../remote/repo
  cd repo
  echo "Mark 3"
  echo "local1 file content" > local1.txt
  metro sync

  echo "Mark 4"
  cd ../..
  mkdir local2
  cd local2
  echo "Mark 5"
  metro clone ../remote/repo
  cd repo

  echo "Mark 6"
  run git branch --list
  [[ "${#lines[@]}" == 0 ]]

  echo "Mark 7"
  run git status
  [[ "${lines[2]}" == "Changes to be committed:" ]]
  [[ "${lines[4]}" == *"new file:   local1.txt" ]]
}

@test "Sync down changes with unchanged local WIP" {
  git init remote/repo --bare

  echo "Mark 1"
  mkdir local1
  cd local1
  echo "Mark 2"
  git clone ../remote/repo
  cd repo
  echo "Mark 3"
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "local1 commit"
  echo "local1 file content 2" > local1-2.txt
  metro sync

  echo "Mark 4"
  cd ../..
  mkdir local2
  cd local2
  echo "Mark 5"
  metro clone ../remote/repo
  cd repo
  echo "Mark 6"
  git add -A
  git commit -m "local2 commit"
  echo "Mark 7"
  metro sync

  echo "Mark 8"
  cd ../../local1/repo
  metro sync
  echo "Mark 9"

  run git branch --list
  [[ "$output" == "* master" ]]

  run git log master
  [[ "${lines[3]}" == *"local2 commit"* ]]
}

@test "Sync down changes with unchanged local WIP but changed base" {
  git init remote/repo --bare

  echo "Mark 1"
  mkdir local1
  cd local1
  echo "Mark 2"
  git clone ../remote/repo
  cd repo
  echo "Mark 3"
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "local1 commit"
  echo "local1 file content 2" > local1-2.txt
  echo "local1 file content 3" > local1-3.txt
  metro sync

  echo "Mark 4"
  cd ../..
  mkdir local2
  cd local2
  echo "Mark 5"
  metro clone ../remote/repo
  cd repo
  echo "Mark 6"
  git add -A
  git commit -m "local2 commit"
  echo "Mark 7"
  metro sync

  echo "Mark 8"
  cd ../../local1/repo
  rm local1-3.txt
  git add -A
  git commit -m "local1 commit 2"
  echo "Mark 9"
  echo "local1 file content 3" > local1-3.txt
  metro sync
  echo "Mark 10"

  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* master#1" ]]
  [[ "${#lines[@]}" == 2 ]]

  run git log master
  [[ "${lines[3]}" == *"local2 commit"* ]]

  run git log master#1
  [[ "${lines[3]}" == *"local1 commit 2"* ]]
}

# ~~~ Test Switch ~~~

@test "Create WIP branch after switch branch" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\" & echo \"Test file content\" > test.txt"
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  echo "$ git branch other & metro switch other"
  git branch other
  metro switch other

  [[ ! -f "test.txt" ]]

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[1]}" == "  master#wip" ]]
  [[ "${lines[2]}" == "* other" ]]


  git log master#wip
  run git log master#wip
  [[ "${lines[3]}" == *"WIP" ]]
}

@test "Switch branch while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch other

  echo "Mark 2"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 3"
  metro switch other

  echo "Mark 4"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* other" ]]

  echo "Mark 5"
  git checkout master

  echo "Mark 6"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* master" ]]
  [[ "${lines[1]}" == "  other" ]]
}

@test "Switch to non-head revision" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test content" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  metro switch HEAD~

  echo "Mark 4"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
}

@test "Switch to tag" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git tag test-tag -m "Test tag"
  echo "Test content" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 2"
  metro switch test-tag

  echo "Mark 3"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
}

@test "Switch to annotated tag" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git tag test-tag -a -m "Test tag"
  echo "Test content" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 2"
  metro switch test-tag

  echo "Mark 3"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
}

@test "Switch branch while detached with uncommitted changes" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch other

  echo "Mark 2"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 3"
  echo "test content" > test.txt

  echo "Mark 4"
  run metro switch other

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
  [[ "${lines[2]}" == "  other" ]]
  [[ "${#lines[@]}" == 3 ]]
}

@test "Switch branch while detached with uncommitted changes with --force" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch other

  echo "Mark 2"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 3"
  echo "test content" > test.txt

  echo "Mark 4"
  metro switch other --force

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* other" ]]
  [[ "${#lines[@]}" == 2 ]]

  echo "Mark 6"
  git status
  run git status
  [[ "$output" == *"nothing to commit"* ]]
}

@test "Switch branch with children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"
  git branch other

  echo "Mark 4"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 5"
  git checkout other

  echo "Mark 6"
  metro switch master

  echo "Mark 7"
  run git log
  [[ "${lines[3]}" == *"Test commit 2"* ]]
  [[ "${lines[7]}" == *"Test commit 1"* ]]

  echo "Mark 8"
  run git branch --list
  [[ "${lines[0]}" == "* master" ]]
  [[ "${lines[1]}" == "  other" ]]
}

# ~~~ Test Delete Branch ~~~

@test "Delete only branch" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ metro delete master"
  run metro delete master
  [[ "$status" != 0 ]]
}

@test "Delete other branch" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ git branch other"
  git branch other

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[1]}" == "  other" ]]

  echo "$ metro delete other"
  metro delete other

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "$output" != *"other"* ]]
}

@test "Delete branch while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"

  echo "Mark 2"
  git branch other

  echo "Mark 3"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 4"
  metro delete other

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]

  echo "Mark 6"
  git checkout master

  echo "Mark 7"
  git branch --list
  run git branch --list
  [[ "$output" == "* master" ]]
}

@test "Delete current WIP" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch master#wip

  echo "Mark 2"
  git branch --list
  run git branch --list
  [[ "${lines[1]}" == "  master#wip" ]]

  echo "Mark 3"
  git checkout master#wip
  git commit --allow-empty -m "WIP"

  echo "Mark 4"
  run metro delete master#wip
  echo "$output"
  [[ "${lines[0]}" == "Deleted branch master#wip." ]]

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* master" ]]
}

@test "Delete current invalid WIP" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch master#wip

  echo "Mark 2"
  git branch --list
  run git branch --list
  [[ "${lines[1]}" == "  master#wip" ]]

  echo "Mark 3"
  git checkout master#wip
  run metro delete master#wip
  echo "$output"
  [[ "${lines[0]}" == "Deleted branch master#wip." ]]

  echo "Mark 4"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* master" ]]
}

# ~~~ Test Rename ~~~

@test "Rename branch (1 argument)" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"

  echo "Mark 2"
  metro rename master-1

  echo "Mark 3"
  run git branch
  [[ "${lines[0]}" == "* master-1" ]]
}

@test "Rename branch (2 arguments)" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch x

  echo "Mark 2"
  metro rename x y

  echo "Mark 3"
  run git branch
  [[ "${lines[0]}" == "* master" ]]
  [[ "${lines[1]}" == "  y" ]]
}

@test "Rename on empty repo" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  metro rename master-1

  echo "Mark 3"
  git branch --list
  run git branch --list
  [[ "${#lines[@]}" == 0 ]]

  echo "Mark 4"
  cat .git/HEAD
  run cat .git/HEAD
  [[ "$output" == "ref: refs/heads/master-1"* ]]
}

@test "Rename while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 2"
  run metro rename master-1

  echo "Mark 3"
  run git branch --list
  [[ "${lines[0]}" == *"* (HEAD detached at"* ]]
  [[ "${lines[1]}" ==  "  master" ]]
}

@test "Rename branch while detached (2 arguments)" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch x
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 2"
  metro rename x y

  echo "Mark 3"
  run git branch
  [[ "${lines[0]}" == *"* (HEAD detached at"* ]]
  [[ "${lines[1]}" ==  "  master" ]]
  [[ "${lines[2]}" ==  "  y" ]]
}

@test "Rename non-existent branch" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch x

  echo "Mark 2"
  run metro rename z y

  echo "Mark 3"
  echo "$output"
  [[ "$output" == "Branch 'z' not found." ]]

  echo "Mark 4"
  run git branch
  echo "$output"
  [[ "${lines[0]}" ==  "* master" ]]
  [[ "${lines[1]}" ==  "  x" ]]
  [[ "${#lines[@]}" ==  2 ]]
}

# ~~~ Test Wip ~~~

@test "Save to WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    echo "Test file content 1" > test.txt

    metro wip save

    run ls
    [[ "${#lines[@]}" ==  0 ]]

    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]
}

@test "Fail to save to WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    git checkout master
    echo "Test file content 2" > test.txt

    run metro wip save
    [[ $status != 0 ]]
}

@test "Restore WIP branch fail" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    git checkout master
    echo "Test file content 2" > test.txt

    run metro wip restore
    [[ $status != 0 ]]
}

@test "Force restore WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    git checkout master
    echo "Test file content 2" > test.txt

    metro wip restore --force

    run cat test.txt
    [[ "${lines[0]}" == "Test file content 1" ]]

    run git branch
    [[ "${lines[0]}" == "* master" ]]
}

@test "Restore invalid WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    echo "Test file content 2" >> test.txt
    git add -A
    git commit -m "Error"
    git checkout master

    run metro wip restore
    [[ $status != 0 ]]

    run ls
    [[ "${#lines[@]}" ==  0 ]]
}

@test "Squash WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    echo "Test file content 2" >> test.txt
    git add -A
    git commit -m "Error"
    git checkout master

    metro wip squash

    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]

    git checkout master#wip
    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 1"* ]]
    [[ "${lines[1]}" == "Test file content 2"* ]]
}

@test "Squash WIP branch with non-empty working dir" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    echo "Test file content 2" >> test.txt
    git add -A
    git commit -m "Error"
    git checkout master
    echo "Test file content 3" > test.txt

    metro wip squash

    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 3"* ]]

    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]

    git checkout master#wip -f
    run git log
    [[ "${lines[3]}" == *"WIP"* ]]
}

@test "Squash WIP branch with extra commit" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    echo "Test file content 2" >> test.txt
    git add -A
    git commit -m "Error"
    git checkout master
    echo "Test file content 3" > test.txt
    git add -A
    git commit -m "Test commit"

    run metro wip squash
    [[ $status != 0 ]]

    metro wip squash --force

    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 3"* ]]

    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]

    git checkout master#wip
    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 1"* ]]
    [[ "${lines[1]}" == "Test file content 2"* ]]

    run git log
    [[ "${lines[3]}" == *"WIP"* ]]
}

@test "Squash valid WIP branch" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout -b master#wip
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "WIP"
    git checkout master

    metro wip squash

    git branch
    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]

    git checkout master#wip
    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 1" ]]
}

@test "Squash WIP branch with merge" {
    git init
    git commit --allow-empty -m "Initial Commit"
    git checkout --orphan other
    echo "Test file content 1" > test-1.txt
    git add -A
    git commit -m "Test Commit"

    git checkout master
    git checkout -b master#wip
    echo "Test file content 2" > test-2.txt
    git add -A
    git commit -m "WIP"
    git merge --allow-unrelated-histories other
    git checkout master

    metro wip squash

    run git branch
    [[ "${lines[0]}" == "* master" ]]
    [[ "${lines[1]}" == "  master#wip" ]]

    git checkout master#wip
    git log
    run git log
    [[ "${lines[1]}" == *"Merge"* ]]
    [[ "${lines[4]}" == *"Absorbed"* ]]
    [[ "$output" == *"Test Commit"* ]]
    [[ "$output" == *"Initial Commit"* ]]
}

@test "Wip commands don't work in detached" {
    git init
    git commit --allow-empty -m "Initial Commit"
    echo "Test file content 1" > test.txt
    git add -A
    git commit -m "Test commit 1"
    echo "\nTest file content 2" >> test.txt
    git add -A
    git commit -m "Test commit 2"

    git checkout HEAD~1
    echo "\nTest file content 3" >> test.txt

    run metro wip save
    [[ $status != 0 ]]
    run metro wip restore
    [[ $status != 0 ]]
    run metro wip squash
    [[ $status != 0 ]]
}

@test "Wip commands work without initial commit" {
    git init
    echo "Test file content 1" > test.txt

    metro wip save

    run git branch
    [[ "${lines[0]}" == "  master#wip" ]]

    git checkout master#wip
    echo "Test file content 2" >> test.txt
    git add -A
    git commit -m "Test commit"

    git checkout --orphan master

    metro wip squash

    git checkout master#wip
    run git log
    [[ "${lines[3]}" == *"WIP"* ]]

    git checkout --orphan master
    rm test.txt

    metro wip restore

    cat test.txt
    run cat test.txt
    [[ "${lines[0]}" == "Test file content 1"* ]]
    [[ "${lines[1]}" == "Test file content 2"* ]]
}