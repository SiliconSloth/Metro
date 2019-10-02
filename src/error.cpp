#include "pch.h"

void check_error(int test) {
	if (test >= 0) return;

	const git_error *e = git_error_last();
	throw GitException(e, test);
}
