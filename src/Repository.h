#pragma once

class Repository {
private:
	Repository(git_repository * repo): repo(repo) {}
	git_repository * repo;

public:
	Repository() = delete;
	~Repository();

	Repository operator=(Repository r) = delete;

	static Repository init(std::string path, bool isBare);
	static Repository open(std::string path);

	git_signature & default_signature();
	Index & index();
	git_tree * lookup_tree(const git_oid & oid);
	git_oid create_commit(std::string update_ref, const git_signature & author, const git_signature & committer, std::string message_encoding, std::string message, const git_tree * tree, std::vector<git_commit*> parents);
	git_object & revparse_single(std::string spec);
	void reset_to_commit(const git_commit &, git_reset_t, const git_checkout_options);
	git_status_list & status_list_new(const git_status_options);
};

