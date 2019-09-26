#include "pch.h"
#include "Repository.h"
#include "error.h"

Repository::~Repository() {
	git_repository_free(repo);
}

Repository & Repository::init(std::string path, bool isBare) {
	git_repository *gitRepo = NULL;
	int err = git_repository_init(&gitRepo, path.c_str(), isBare);
	checkError(err);

	Repository repo(gitRepo);
	return repo;
}

Repository & Repository::open(std::string path) {
	git_repository *gitRepo = NULL;
	int err = git_repository_open(&gitRepo, path.c_str());
	checkError(err);

	Repository repo(gitRepo);
	return repo;
}

git_signature & Repository::default_signature() {
	git_signature **sig;
	int err = git_signature_default(sig, repo);
	checkError(err);
	return **sig;
}

git_index & Repository::index() {
	git_index **index;
	int err = git_repository_index(index, repo);
	checkError(err);
	return **index;
}

git_tree & Repository::lookup_tree(const git_oid & oid) {
	git_tree **tree;
	int err = git_tree_lookup(tree, repo, &oid);
	checkError(err);
	return **tree;
}

git_commit & Repository::create_commit(git_oid & id, std::string update_ref, const git_signature & author, const git_signature & committer, std::string message_encoding, std::string message, const git_tree & tree, const std::vector<git_commit> parents) {
	git_commit **commit;
	git_commit parents_array[parents.max_size];
	std::copy(parents.begin(), parents.end(), parents_array);
	int err = git_commit_create(&id, repo, update_ref.c_str(), &author, &committer, message_encoding.c_str(), message.c_str(), &tree, parents.max_size, parents_array);
	checkError(err);
	return **commit;
}

git_object & Repository::revparse_single(std::string spec) {
	git_object **obj;
	int err = git_revparse_single(obj, repo, spec.c_str());
	checkError(err);
	return **obj;
}

void Repository::reset_to_commit(const git_commit & commit, git_reset_t type, const git_checkout_options ops) {
	int err = git_reset(repo, (git_object *) & commit, type, & ops);
	checkError(err);
}

git_status_list & Repository::status_list_new(const git_status_options ops) {
	git_status_list **status;
	int err = git_status_list_new(status, repo, & ops);
	checkError(err);
	return **status;
}
