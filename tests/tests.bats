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

@test "Commit file" {
  echo "$ git init & echo \"Test file\" > test.txt & metro commit \"Test commit message\""
  git init
  echo "Test file" > test.txt
  metro commit "Test commit message"

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Test commit message"* ]]
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
  git log
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
  [[ "${lines[0]}" == "  master" ]]
  [[ "${lines[1]}" == "* master#1" ]]

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

  echo "$ git log"
  git log
  run git log
  [[ "${lines[3]}" == *"Local2 commit message"* ]]
  [[ "${lines[7]}" == *"Local1 commit message 2"* ]]
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
  [[ "${lines[1]}" == "  other" ]]
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
  [[ "$output" != "test.txt" ]]
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