#pragma once

#include "error.h"
#include "git2.h"

class Index {
public:
	Index(git_index * index) : index(index) {};
	~Index();

	void add_all(const git_strarray pathspec, unsigned int flags, git_index_matched_path_cb callback);
	git_oid write_tree();
	void write();

private:
	git_index * index;
};

