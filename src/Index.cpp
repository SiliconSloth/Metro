#include "pch.h"
#include "Index.h"

Index::~Index() {
	git_index_free(index);
}

void Index::add_all(const git_strarray pathspec, unsigned int flags, git_index_matched_path_cb callback) {
	int err = git_index_add_all(index, &pathspec, flags, callback, NULL);
	check_error(err);
}

git_oid Index::write_tree() {
	git_oid oid;
	int err = git_index_write_tree(&oid, index);
	check_error(err);
	return oid;
}

void Index::write() {
	git_index_write(index);
}
