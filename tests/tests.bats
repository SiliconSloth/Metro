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
  metro create
  cd .git

  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Create repo in subdir" {
  metro create repo/test
  cd repo/test/.git

  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

# ~~~ Test Commit ~~~

@test "Commit file" {
  git init
  echo "Test file" > test.txt
  metro commit "Test commit message"

  run git log
  [[ "${lines[3]}" == *"Test commit message"* ]]
}

@test "Commit file in empty repo" {
  git init
  echo "Test file" > test.txt
  metro commit "Test commit message"

  run git log
  [[ "${lines[3]}" == *"Test commit message"* ]]
}

# ~~~ Test Clone ~~~

@test "Clone empty repo" {
  mkdir -p create remote/repo
  cd remote/repo
  git init

  cd ../..
  mkdir local
  cd local
  metro clone ../remote/repo
  cd repo

  run git log
  [[ "${lines[0]}" == "fatal: your current branch 'master' does not have any commits yet" ]]
}

@test "Clone repo with initial commit only" {
  git init remote/repo
  cd remote/repo
  git commit --allow-empty -m "Initial Commit"
  cd ../..

  mkdir local
  cd local
  metro clone ../remote/repo
  cd repo

  run git log
  [[ "${lines[3]}" == *"Initial Commit"* ]]
}

@test "Clone repo with one commit" {
  git init remote/repo
  cd remote/repo
  echo "Remote file content" > remote.txt
  git add -A
  git commit -m "Remote commit message"

  cd ../..
  mkdir local
  cd local
  metro clone ../remote/repo
  cd repo

  run git log
  [[ "${lines[3]}" == *"Remote commit message"* ]]
}

# ~~~ Test Sync ~~~

@test "Push and pull commit from one local to another with sync" {
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  git clone ../remote/repo
  cd repo
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "Local1 commit message"

  cd ../..
  mkdir local2
  cd local2
  git clone ../remote/repo

  cd ../local1/repo
  metro sync
  cd ../../local2/repo
  metro sync

  run git log
  [[ "${lines[3]}" == *"Local1 commit message"* ]]
}

@test "Push and pull WIP commit from one local to another with sync" {
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  git clone ../remote/repo
  cd repo
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "Local1 commit message"
  echo "local1 file content" > local1-1.txt

  cd ../..
  mkdir local2
  cd local2
  git clone ../remote/repo

  cd ../local1/repo
  metro sync
  cd ../../local2/repo
  metro sync

  run ls
  [[ "${lines[1]}" == *"local1-1.txt"* ]]
}

@test "Push and pull deleted commit from one local to another with sync" {
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  git clone ../remote/repo
  cd repo
  echo "local1 file content" > local1.txt
  git add -A
  git commit -m "Local1 commit message"
  echo "local1 file content" > local1-1.txt
  git add -A
  git commit -m "Local1 commit message 1"

  cd ../..
  mkdir local2
  cd local2
  git clone ../remote/repo

  cd ../local1/repo
  metro sync
  cd ../../local2/repo
  metro sync

  cd ../../local1/repo
  metro delete commit # Required metro command for cache update
  metro sync
  cd ../../local2/repo
  metro sync

  run git log
  [[ "${lines[3]}" != *"Local1 commit message 1"* ]]
}

# ~~~ Test Branch ~~~

@test "Create branch" {
  git init
  git commit --allow-empty -m "Initial Commit"
  metro branch other

  run git branch --list
  [[ "${lines[1]}" == "  other" ]]
}

# ~~~ Test Switch ~~~

@test "Create WIP branch after switch branch" {
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  git branch other
  metro switch other

  [[ ! -f "test.txt" ]]

  run git branch --list
  [[ "${lines[1]}" == "  master#wip" ]]
  [[ "${lines[2]}" == "* other" ]]

  run git log master#wip
  [[ "${lines[3]}" == *"WIP" ]]
}

# ~~~ Test Delete Branch ~~~

@test "Delete only branch" {
  git init
  git commit --allow-empty -m "Initial Commit"
  run metro delete branch master
  [[ "$status" != 0 ]]
}

@test "Delete other branch" {
  git init
  git commit --allow-empty -m "Initial Commit"
  git branch other

  run git branch --list
  [[ "${lines[1]}" == "  other" ]]

  metro delete branch other

  run git branch --list
  [[ "$output" != *"other"* ]]
}

# ~~~ Test Delete Commit ~~~

@test "Delete last commit" {
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  metro delete commit

  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  run ls
  [[ "$output" != "test.txt" ]]
}

@test "Delete last commit soft" {
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  metro delete commit --soft

  run git log
  [[ "${lines[3]}" != *"Test Commit"* ]]

  run ls
  [[ "$output" == "test.txt" ]]
}

# ~~~ Test Patch ~~~

@test "Patch Commit Contents" {
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"
  echo "Test file content" > test-1.txt
  git add -A

  metro patch

  run git log
  [[ "${lines[3]}" == *"Test Commit"* ]]

  run git status
  [[ "${lines[3]}" != *"modified"* ]]
}

@test "Patch Commit Message" {
  git init
  git commit --allow-empty -m "Initial Commit"
  echo "Test file content" > test.txt
  git add -A
  git commit -m "Test Commit"

  metro patch "Test Commit 1"

  run git log
  [[ "${lines[3]}" == *"Test Commit 1"* ]]
}