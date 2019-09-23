#include "pch.h"
#include "Repository.h"
#include "error.h"

Repository::~Repository() {
	git_repository_free(repo);
}

Repository& Repository::init(std::string path, bool isBare) {
	git_repository *gitRepo = NULL;
	int err = git_repository_init(&gitRepo, path.c_str(), isBare);
	checkError(err);

	Repository repo(gitRepo);
	return repo;
}

Repository& Repository::open(std::string path) {
	git_repository *gitRepo = NULL;
	int err = git_repository_open(&gitRepo, path.c_str());
	checkError(err);

	Repository repo(gitRepo);
	return repo;
}