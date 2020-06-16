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

# ~~~ Test Create ~~~

@test "Create repo in current dir" {
  echo "$ metro create"
  metro create
  cd .git

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Create repo in subdir" {
  echo "$ metro create repo/test"
  metro create repo/test
  cd repo/test/.git

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

# ~~~ Test Commit ~~~

@test "Commit after initial commit" {
  echo "Mark 1"
  metro create
  echo "Mark 2"
  echo "Test file" > test.txt
  metro commit "Test commit message"
  echo "Mark 3"

  git log
  run git log
  [[ "${lines[3]}" == *"Test commit message"* ]]
  [[ "${lines[7]}" == *"Create repository"* ]]
}

@test "Commit file in empty repo" {
  echo "$ git init & echo \"Test file\" > test.txt & metro commit \"Test commit message\""
  git init
  echo "Test file" > test.txt
  metro commit "Test commit message"

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Test commit message"* ]]
}

@test "Commit file with detached head" {
  echo "Mark 1"
  git init

  echo "Test file 1" > test.txt
  metro commit "Test commit message 1"
  echo "Mark 2"

  echo "Test file 2" > test.txt
  metro commit "Test commit message 2"
  echo "Mark 3"

  git checkout HEAD~
  echo "Mark 4"

  echo "Test file 3" > test.txt
  run metro commit "Test commit message 3"
  [[ "$status" != 0 ]]
  echo "Mark 5"

  git log
  run git log
  [[ "${lines[3]}" == *"Test commit message 1"* ]]
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

@test "Sync deleted commit" {
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
  echo "local1 file content 2" > local1-2.txt
  git add -A
  echo "$ echo \"local1 file content 2\" > local1-2.txt & git commit -m \"Local1 commit message 2\""
  git commit -m "Local1 commit message 2"

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
  echo "$ metro delete commit"
  metro delete commit
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" != *"Local1 commit message 2"* ]]
}

@test "Sync patched commit" {
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

  cd ../local1/repo
  echo "$ cd ~/local1"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  cd ../../local1/repo
  echo "$ cd ~/local1"
  echo "$ metro patch \"Local1 commit message 2\""
  metro patch "Local1 commit message 2"
  echo "$ metro sync"
  metro sync
  cd ../../local2/repo
  echo "$ cd ~/local2"
  echo "$ metro sync"
  metro sync

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Local1 commit message 2"* ]]

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
  metro commit "local1 commit"
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
  metro commit "local2 commit"
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
  metro commit "local1 commit"
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
  metro commit "local2 commit"
  echo "Mark 7"
  metro sync

  echo "Mark 8"
  cd ../../local1/repo
  rm local1-3.txt
  metro commit "local1 commit 2"
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

# ~~~ Test Branch ~~~

@test "Create branch" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ metro branch other"
  metro branch other

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[1]}" == "* other" ]]
}

@test "Create branch with uncommitted changes" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"

  echo "test content" > test.txt
  echo "$ metro branch other"
  metro branch other

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "  master#wip" ]]
  [[ "${lines[2]}" == "* other" ]]
}

@test "Create branch with no commits" {
  echo "$ git init"
  git init
  echo "$ metro branch other"
  run metro branch other

  echo "$ git branch --list"
  git branch --list
  run git branch --list
  [[ "$output" == "" ]]
}

@test "Create branch while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"

  echo "Mark 2"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 3"
  metro branch other

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

@test "Create branch while detached with uncommitted changes" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"

  echo "Mark 2"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 3"
  echo "test content" > test.txt

  echo "Mark 4"
  metro branch other

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
  [[ "${lines[2]}" == "  other" ]]
  [[ "${#lines[@]}" == 3 ]]
}

@test "Create branch with children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 4"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 5"
  git checkout HEAD~

  echo "Mark 6"
  metro branch other

  echo "Mark 7"
  run git log
  [[ "${lines[3]}" == *"Test commit 1"* ]]

  echo "Mark 8"
  run git branch --list
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* other" ]]
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
  metro switch other

  echo "Mark 5"
  git branch --list
  run git branch --list
  [[ "${lines[0]}" == "* (HEAD detached at "* ]]
  [[ "${lines[1]}" == "  master" ]]
  [[ "${lines[2]}" == "  other" ]]
  [[ "${#lines[@]}" == 3 ]]
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
  echo "$ metro delete branch master"
  run metro delete branch master
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

  echo "$ metro delete branch other"
  metro delete branch other

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
  metro delete branch other

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

# ~~~ Test Delete Commit ~~~

@test "Delete last commit" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  echo "$ metro delete commit"
  metro delete commit

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  echo "$ ls"
  ls
  run ls
  [[ -z "$output" ]]
}

@test "Delete last commit with uncommitted changes" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"
  echo "Test content 2" > test2.txt

  echo "$ metro delete commit"
  metro delete commit

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  echo "$ ls"
  ls
  run ls
  [[ -z "$output" ]]
}

@test "Delete last commit soft" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  echo "$ metro delete commit --soft"
  metro delete commit --soft

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  echo "$ ls"
  ls
  run ls
  [[ "$output" == "test.txt" ]]
}

@test "Delete last commit soft with uncommitted changes" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"
  echo "Test content 2" > test2.txt

  echo "$ metro delete commit --soft"
  metro delete commit --soft

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  echo "$ ls"
  ls
  run ls
  [[ "${lines[0]}" == "test.txt" ]]
  [[ "${lines[1]}" == "test2.txt" ]]
  [[ "${#lines[@]}" == 2 ]]
}

@test "Delete with children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  git branch other

  echo "Mark 4"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 5"
  git checkout other

  echo "Mark 6"
  run metro delete commit
  [[ "$output" == "Cannot delete because commit has children" ]]

  echo "Mark 7"
  run git log
  [[ "${lines[3]}" == *"Test commit 1"* ]]
}

@test "Delete initial commit" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  run metro delete commit
  [[ "$output" == "Can't delete initial commit." ]]

  echo "Mark 4"
  run git log
  [[ "${lines[3]}" == *"Test commit 1"* ]]
}

@test "Delete with no commits" {
  echo "Mark 1"
  git init
  echo "Mark 2"

  run metro delete commit
  echo "Mark 3"
  [[ "$output" == "No commit to delete." ]]
}

@test "Delete with detached head" {
  echo "Mark 1"
  metro create

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 4"
  metro delete commit

  echo "Mark 5"
  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]

  echo "Mark 6"
  git checkout master

  echo "Mark 7"
  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Delete with detached head and children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 4"
  git checkout HEAD~

  echo "Mark 5"
  run metro delete commit
  [[ "$output" == "Cannot delete because commit has children" ]]

  echo "Mark 6"
  run git log
  [[ "${lines[3]}" == *"Test commit 1"* ]]
}

# ~~~ Test Patch ~~~

@test "Patch commit contents" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test-1.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test-1.txt
  git add -A
  git commit -m "Test Commit"
  echo "$ echo \"Test file content\" > test-2.txt"
  echo "Test file content" > test-2.txt
  git add -A

  echo "$ metro patch"
  metro patch

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Test Commit"* ]]

  echo "$ git status"
  git status
  run git status
  [[ "${lines[3]}" != *"modified"* ]]
}

@test "Patch commit message" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  echo "$ metro patch \"Test Commit 1\""
  metro patch "Test Commit 1"

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Test Commit 1"* ]]
}

@test "Patch with children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  git branch other

  echo "Mark 4"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 5"
  git checkout other

  echo "Mark 6"
  run metro patch "Patched commit"
  [[ "$output" == "Cannot patch because commit has children" ]]
}

@test "Patch with no commits" {
  echo "Mark 1"
  git init
  echo "Mark 2"

  run metro patch "Test Commit 1"
  echo "Mark 3"
  [[ "$output" == "No commit to patch." ]]
}

@test "Patch with detached head" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 4"
  metro patch "Patch test commit"

  echo "Mark 5"
  run git log
  [[ "${lines[3]}" == *"Patch test commit"* ]]
  [[ "${#lines[@]}" == 4 ]]

  echo "Mark 6"
  git checkout master

  echo "Mark 7"
  run git log
  [[ "${lines[3]}" == *"Patch test commit"* ]]
  [[ "${#lines[@]}" == 4 ]]
}

@test "Patch with detached head and children" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  echo "Test content 1" > test.txt
  git add -A
  git commit -m "Test commit 1"

  echo "Mark 3"
  echo "Test content 2" > test.txt
  git add -A
  git commit -m "Test commit 2"

  echo "Mark 4"
  git checkout HEAD~

  echo "Mark 5"
  run metro patch "Patched commit"
  [[ "$output" == "Cannot patch because commit has children" ]]
}

# ~~~ Test Absorb ~~~

@test "Absorb branch with commit" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"

  echo "$ git checkout -b other"
  git checkout -b other
  echo "$ echo \"Test file content\" > test.txt & git commit -am \"Test Commit\""
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"
  echo "$ git checkout master"
  git checkout master

  echo "$ metro absorb other"
  metro absorb other

  echo "$ git log"
  git log
  run git log
  [[ "${lines[4]}" == *"Absorbed other"* ]]
  [[ "${lines[8]}" == *"Test Commit"* || "${lines[12]}" == *"Test Commit"* ]]
}

@test "Absorb branch with conflict" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"

  echo "$ git checkout -b other"
  git checkout -b other
  echo "$ echo \"Test file content\" > test-1.txt & git commit -am \"Test Commit 1\""
  echo "Test file content" > test-1.txt
  git add -A
  git commit -m "Test Commit 1"

  echo "$ git checkout master"
  git checkout master
  echo "$ echo \"Test file content\" > test-2.txt & git commit -am \"Test Commit 2\""
  echo "Test file content" > test-2.txt
  git add -A
  git commit -m "Test Commit 2"

  echo "$ metro absorb other"
  metro absorb other

  echo "$ git log"
  git log
  run git log
  [[ "${lines[4]}" == *"Absorbed other"* ]]
  [[ "${lines[8]}" == *"Test Commit 2"* ]]
  [[ "${lines[12]}" == *"Test Commit 1"* ]]
}

@test "Absorb branch with content conflict and resolve" {
  echo "$ git init"
  git init
  echo "$ git commit --allow-empty -m \"Initial Commit\""
  git commit --allow-empty -m "Initial Commit"

  echo "$ git checkout -b other"
  git checkout -b other
  echo "$ echo \"Test file content 1\" > test.txt & git commit -am \"Test Commit 1\""
  echo "Test file content 1" > test.txt
  git add -A
  git commit -m "Test Commit 1"

  echo "$ git checkout master"
  git checkout master
  echo "$ echo \"Test file content 2\" > test.txt & git commit -am \"Test Commit 2\""
  echo "Test file content 2" > test.txt
  git add -A
  git commit -m "Test Commit 2"

  echo "$ metro absorb other"
  metro absorb other

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Test Commit 2"* ]]

  echo "$ metro resolve"
  metro resolve

  echo "$ git log"
  git log
  run git log
  [[ "${lines[4]}" == *"Absorbed other"* ]]
  [[ "${lines[8]}" == *"Test Commit 2"* ]]
  [[ "${lines[12]}" == *"Test Commit 1"* ]]
}

@test "Absorb branch with content conflict and switch before resolve" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch separate

  echo "Mark 2"
  git checkout -b other
  echo "Test file content 1" > test.txt
  git add -A
  git commit -m "Test Commit 1"

  echo "Mark 3"
  git checkout master
  echo "Test file content 2" > test.txt
  git add -A
  git commit -m "Test Commit 2"

  echo "Mark 4"
  metro absorb other

  echo "Mark 5"
  git log
  run git log
  [[ "${lines[3]}" == *"Test Commit 2"* ]]

  echo "Mark 6"
  metro switch separate
  metro switch master
  metro resolve

  echo "Mark 7"
  git log
  run git log
  [[ "${lines[4]}" == *"Absorbed other"* ]]
  [[ "${lines[8]}" == *"Test Commit 2"* ]]
  [[ "${lines[12]}" == *"Test Commit 1"* ]]
}

@test "Absorb branch while detached" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  git commit --allow-empty -m "Initial Commit"
  git branch branch-2
  git checkout branch-2
  touch "Test"
  git add -A
  git commit -m "Test commit"

  echo "Mark 3"
  git checkout master
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 4"
  run metro absorb branch-2
  [[ "$output" == "You must be on a branch to absorb." ]]
}

# ~~~ Test Info ~~~

@test "Info with no commits" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  metro info
  run metro info
  [[ "${lines[0]}" == "Current branch is master"* ]]
  [[ "${lines[1]}" == "Not merging"* ]]
  [[ "${lines[2]}" == "Nothing to commit"* ]]
}

@test "Info with no commits and some changes" {
  echo "Mark 1"
  git init
  echo "Test content" > test.txt
  git add test.txt
  echo "Test content 2" > test2.txt

  echo "Mark 2"
  git status
  run git status
  [[ "${lines[2]}" == "Changes to be committed:" ]]
  [[ "${lines[4]}" == *"new file:   test.txt" ]]
  [[ "${lines[5]}" == "Untracked files:" ]]
  [[ "${lines[7]}" == *"test2.txt" ]]

  echo "Mark 3"
  metro info
  run metro info
  [[ "${lines[0]}" == "Current branch is master"* ]]
  [[ "${lines[1]}" == "Not merging"* ]]
  [[ "${lines[2]}" == "2 files to add"* ]]

  # Mustn't change index state
  echo "Mark 4"
  git status
  run git status
  [[ "${lines[2]}" == "Changes to be committed:" ]]
  [[ "${lines[4]}" == *"new file:   test.txt" ]]
  [[ "${lines[5]}" == "Untracked files:" ]]
  [[ "${lines[7]}" == *"test2.txt" ]]
}

@test "Info with one commit" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Test commit"

  echo "Mark 2"
  run metro info
  [[ "${lines[0]}" == "Current branch is master"* ]]
  [[ "${lines[1]}" == "Not merging"* ]]
  [[ "${lines[2]}" == "Nothing to commit"* ]]
}

@test "Info with one commit and some changes" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Test commit"
  echo "Test content" > test.txt
  git add test.txt
  echo "Test content 2" > test2.txt

  echo "Mark 2"
  git status
  run git status
  [[ "${lines[1]}" == "Changes to be committed:" ]]
  [[ "${lines[3]}" == *"new file:   test.txt" ]]
  [[ "${lines[4]}" == "Untracked files:" ]]
  [[ "${lines[6]}" == *"test2.txt" ]]

  echo "Mark 3"
  metro info
  run metro info
  [[ "${lines[0]}" == "Current branch is master"* ]]
  [[ "${lines[1]}" == "Not merging"* ]]
  [[ "${lines[2]}" == "2 files to add"* ]]

  # Mustn't change index state
  echo "Mark 4"
  git status
  run git status
  [[ "${lines[1]}" == "Changes to be committed:" ]]
  [[ "${lines[3]}" == *"new file:   test.txt" ]]
  [[ "${lines[4]}" == "Untracked files:" ]]
  [[ "${lines[6]}" == *"test2.txt" ]]
}

@test "Info while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Test commit"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 2"
  run metro info
  [[ "${lines[0]}" == "Head is detached at commit "* ]]
  [[ "${lines[1]}" == "Not merging"* ]]
  [[ "${lines[2]}" == "Nothing to commit"* ]]
}

# ~~~ Test Resolve ~~~

@test "Resolve while not absorbing" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  run metro resolve
  [[ "${lines[0]}" == "You can only resolve conflicts while absorbing." ]]
}

# See "Absorb branch with content conflict and resolve"

# ~~~ Test List ~~~

@test "List branches" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch x
  git branch y
  git branch z

  echo "Mark 2"
  metro list branches
  run metro list branches
  [[ "${lines[0]}" == *"master"* ]]
  [[ "${lines[1]}" == *"x"* ]]
  [[ "${lines[2]}" == *"y"* ]]
  [[ "${lines[3]}" == *"z"* ]]
}

@test "Empty repo list no branches" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  run metro list branches
  [[ "${#lines[@]}" == 0 ]]
}

@test "List branches while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 2"
  run metro list branches
  [[ "${lines[0]}" == *"master"* ]]
}

@test "List commits" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"

  echo "Mark 2"
  run metro list commits
  [[ "${lines[4]}" == *"Initial Commit"* ]]
}

@test "Empty repo list commits" {
  echo "Mark 1"
  git init

  echo "Mark 2"
  metro list commits
  run metro list commits
  [[ "$output" == "No commits at this location" ]]
}

@test "List commits while detached" {
  echo "Mark 1"
  git init
  git commit --allow-empty -m "Initial Commit"
  git checkout "$(git rev-parse HEAD)"

  echo "Mark 2"
  run metro list commits
  [[ "${lines[4]}" == *"Initial Commit"* ]]
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