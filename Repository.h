#pragma once
#include <string>
#include "git2.h"

class Repository {
private:
	Repository(git_repository *repo): repo(repo) {}
	git_repository* repo;

public:
	Repository() = delete;
	~Repository();

	static Repository& init(std::string path, bool isBare);
	static Repository& open(std::string path);
};

