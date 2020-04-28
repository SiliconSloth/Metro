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


@test "Create repo in current dir" {
  metro create

  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Create repo in subdir" {
  metro create repo/test
  cd repo/test

  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Commit file" {
  metro create
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
  metro create remote/repo

  mkdir local
  cd local
  metro clone ../remote/repo
  cd repo
  
  run git log
  [[ "${lines[3]}" == *"Create repository"* ]]
}

@test "Clone repo with one commit" {
  metro create remote/repo
  cd remote/repo
  echo "Remote file content" > remote.txt
  metro commit "Remote commit message"

  cd ../..
  mkdir local
  cd local
  metro clone ../remote/repo
  cd repo
  
  run git log
  [[ "${lines[3]}" == *"Remote commit message"* ]]
}

@test "Push and pull commit from one local to another with sync" {
  git init remote/repo --bare
  cd remote/repo

  cd ../..
  mkdir local1
  cd local1
  git clone ../remote/repo
  cd repo
  echo "local1 file content" > local1.txt
  metro commit "Local1 commit message"

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

@test "Create branch" {
  metro create
  metro branch other

  run git branch --list
  [[ "${lines[1]}" == "  other" ]]
}

@test "Create WIP branch afer switch branch" {
  metro create
  echo "Test file content" > test.txt
  metro branch other
  metro switch other

  [[ ! -f "test.txt" ]]

  run git branch --list
  [[ "${lines[1]}" == "  master#wip" ]]
  [[ "${lines[2]}" == "* other" ]]

  run git log master#wip
  [[ "${lines[3]}" == *"WIP" ]]
}

@test "Delete only branch" {
  metro create
  run metro delete branch master
  [[ "$status" != 0 ]]
}

@test "Delete other branch" {
  metro create
  metro branch other

  run git branch --list
  [[ "${lines[1]}" == "  other" ]]

  metro delete branch other
  
  run git branch --list
  [[ "$output" != *"other"* ]]
}