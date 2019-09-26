#include "pch.h"
#include <iostream>
#include "git2.h"
#include "Repository.h"
#include "error.h"

int main() {
	// Required
	git_libgit2_init();

	try {
		Repository repo = Repository::init("path", false);
		repo.reset_to_commit();
	} catch (GitException &e) {
		printf("%s, %d, %d", e.what(), e.klass(), e.code());
	}

	// Required
	git_libgit2_shutdown();
}