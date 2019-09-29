#include "pch.cpp"

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
			printf("You don't have a username and email set for commits\n");
			printf("Please enter what username and email to use:\n");
			printf("Username: ");
			std::string username;
			//scanf("%s", &username);

			printf("\nEmail: ");
			std::string email;
			//scanf("%s", &email);
			printf("\n");

			//SetCreds(repo, username, email);

			author = repo.default_signature();
		}
	} catch (GitException &e) {
		printf("%s, %d, %d", e.what(), e.klass(), e.code());
	}

	// Required
	git_libgit2_shutdown();
}