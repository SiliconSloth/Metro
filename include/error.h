#pragma once
#include "pch.h"
#include "git2.h"
#include <exception>

void check_error(int test);

struct MetroException : public std::runtime_error {
    using std::runtime_error::runtime_error;

    MetroException():
        std::runtime_error("Metro exception")
    {};
};

struct GitException : public std::runtime_error {
public:
	GitException(const git_error *error, int code) : error(error), kode(code), std::runtime_error(error->message) {}

	int code() const throw () {
		return kode;
	}

	int klass() const throw () {
		return error->klass;
	}

private:
	const git_error *error;
	const int kode;
};

struct CommandArgumentException : public MetroException {
public:
    const std::string arg;

    explicit CommandArgumentException(const string arg, const string message) : arg(arg),
        MetroException(message.c_str())
    {}
};

struct UnknownOptionException : public CommandArgumentException {
    explicit UnknownOptionException(const string arg):
        CommandArgumentException(arg, "Unknown option: " + arg)
    {}
};

struct MissingValueException : public CommandArgumentException {
    explicit MissingValueException(const string arg):
            CommandArgumentException(arg, "Option needs a value: " + arg)
    {}
};

struct MissingFlagException : public CommandArgumentException {
    explicit MissingFlagException(const string arg):
            CommandArgumentException(arg, "Value without flag: " + arg)
    {}
};

struct UnexpectedValueException : public CommandArgumentException {
    explicit UnexpectedValueException(const string arg):
            CommandArgumentException(arg, "Option doesn't take a value: " + arg)
    {}
};

struct UnexpectedPositionalException : public CommandArgumentException {
    explicit UnexpectedPositionalException(const string arg):
            CommandArgumentException(arg, "Unexpected argument: " + arg)
    {}
};

struct MissingPositionalException : public CommandArgumentException {
    explicit MissingPositionalException(const string arg):
            CommandArgumentException(arg, "Missing argument: " + arg)
    {}
};

struct RepositoryExistsException : public MetroException {
    explicit RepositoryExistsException():
        MetroException("There is already a repository in this directory.")
    {}
};

struct CurrentlyMergingException : public MetroException {
    explicit CurrentlyMergingException():
            MetroException("Branch has conflicts, please finish resolving them.\nRun metro resolve when you are done.")
    {}
};

struct BranchNotFoundException : public MetroException {
    explicit BranchNotFoundException():
            MetroException("Branch not found")
    {}
};

struct UnnecessaryMergeException : public MetroException {
    explicit UnnecessaryMergeException():
            MetroException("Nothing to absorb.")
    {}
};

struct UnsupportedOperationException : public MetroException {
    explicit UnsupportedOperationException(const char* message):
        MetroException(message)
    {}
};