#include "pch.h"
#include "Repository.h"
#include "error.h"

Repository::~Repository() {
	git_repository_free(repo);
}

Repository Repository::init(std::string path, bool isBare) {
	git_repository *gitRepo = NULL;
	int err = git_repository_init(&gitRepo, path.c_str(), isBare);
	check_error(err);

	return Repository(gitRepo);
}

Repository Repository::open(std::string path) {
	git_repository *gitRepo = NULL;
	int err = git_repository_open(&gitRepo, path.c_str());
	check_error(err);

	return Repository(gitRepo);
}

git_signature & Repository::default_signature() {
	git_signature *sig;
	int err = git_signature_default(&sig, repo);
	check_error(err);
	return *sig;
}

Index & Repository::index() {
	git_index *index;
	int err = git_repository_index(&index, repo);
	check_error(err);
	Index out(index);
	return out;
}

git_tree * Repository::lookup_tree(const git_oid & oid) {
	git_tree *tree;
	int err = git_tree_lookup(&tree, repo, &oid);
	check_error(err);
	return tree;
}
git_oid Repository::create_commit(std::string update_ref, const git_signature & author, const git_signature & committer, std::string message_encoding, std::string message, const git_tree * tree, std::vector<git_commit*> parents) {
	git_commit *commit;
	const git_commit** parents_array = new const git_commit*[parents.size()];
	std::copy(parents.begin(), parents.end(), parents_array);
    git_oid id;
	int err = git_commit_create(&id, repo, update_ref.c_str(), &author, &committer, message_encoding.c_str(), message.c_str(), tree, 0, parents_array);
	check_error(err);
	return id;
}

git_object & Repository::revparse_single(std::string spec) {
	git_object *obj;
	int err = git_revparse_single(&obj, repo, spec.c_str());
	check_error(err);
	return *obj;
}

void Repository::reset_to_commit(const git_commit & commit, git_reset_t type, const git_checkout_options ops) {
	int err = git_reset(repo, (git_object *) & commit, type, & ops);
	check_error(err);
}

git_status_list & Repository::status_list_new(const git_status_options ops) {
	git_status_list *status;
	int err = git_status_list_new(&status, repo, & ops);
	check_error(err);
	return *status;
}
