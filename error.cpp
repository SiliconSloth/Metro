#include "pch.h"
#include "error.h"
#include <exception>
#include <memory>

void checkError(int test) {
	if (test >= 0) return;

	const git_error *e = git_error_last();
	throw GitException(e, test);
}