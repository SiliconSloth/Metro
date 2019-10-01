#include "pch.h"

git_strarray pathSpecs(Repository & repo);

int main() {
	// Required
	git_libgit2_init();

	try {
		Repository repo = Repository::init("path", false);
		git_signature author;
		try {
			author = repo.default_signature();
		} catch (GitException &e) {
			// Failed to get default signature - must not be set
            std::cout << "You don't have a username and email set for commits" << std::endl;
            std::cout << "Please enter what username and email to use:" << std::endl;
            std::cout << "Username: ";
			std::string username;
			std::cin >> username;

            std::cout << std::endl << "Email: ";
			std::string email;
			std::cin >> email;
            std::cout << std::endl;

			//SetCreds(repo, username, email);

			author = repo.default_signature();
		}

		Index index = repo.index();

		index.add_all(pathSpecs(repo), GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, NULL);
		git_oid oid = index.write_tree();
        git_tree * tree = repo.lookup_tree(oid);
		index.write();
		std::vector<git_commit *> parents;
		repo.create_commit("HEAD", author, author, "UTF-8", "Initial Commit", tree, parents);

	} catch (GitException &e) {
	    // TODO Change to something more descriptive
		printf("%s, %d, %d", e.what(), e.klass(), e.code());
	}

	// Required
	git_libgit2_shutdown();
}

// Returns the path specs for the Ignore files
git_strarray pathSpecs(Repository & repo) {
    // Finds any ignore files
    struct git_strarray ignore {
        // TODO Actually add the .gitignore and .metroignore
    };
    return ignore;
}
