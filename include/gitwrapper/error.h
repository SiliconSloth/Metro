#pragma once
#include "pch.h"

void check_error(int test);

struct GitException : public std::exception {
public:
	GitException(const git_error *error, int code) : error(error), kode(code) {}

	int code() const throw () {
		return kode;
	}

	int klass() const throw () {
		return error->klass;
	}

	const char *what() const throw () {
		return error->message;
	}

private:
	const git_error *error;
	int kode;
};