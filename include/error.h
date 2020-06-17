/*
 * This contains code regarding exceptions and error handling within Metro.
 */

#pragma once

/**
 * Checks the return value of a git function for an error.
 * @param test Return value to test.
 */
void check_error(int test);

/**
 * MetroExceptions should be thrown by expected scenarios where an error should be passed back to the user.
 */
struct MetroException : public std::runtime_error {
    using std::runtime_error::runtime_error;

    MetroException():
        std::runtime_error("Metro exception")
    {};
};

/**
 * GitExceptions should be thrown as a result of a libgit2 call producing an error
 */
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

/**
 * CommandArgumentException should be thrown when the format of the arguments provided to a command is invalid.
 */
struct CommandArgumentException : public MetroException {
public:
    const std::string arg;

    explicit CommandArgumentException(const string arg, const string message) : arg(arg),
        MetroException(message.c_str())
    {}
};

/**
 * UnknownOptionException should be thrown when the user passes an option that does not exist.
 */
struct UnknownOptionException : public CommandArgumentException {
    explicit UnknownOptionException(const string arg):
        CommandArgumentException(arg, "Unknown option: " + arg)
    {}
};

/**
 * InvalidOptionException should be thrown when an option has been passed with an invalid parameter.
 */
struct InvalidOptionException : public CommandArgumentException {
    explicit InvalidOptionException(const string arg, const string arg2):
            CommandArgumentException(arg, "Invalid option: " + arg + " with " + arg2)
    {}
};

/**
 * MissingValueException should be thrown when a value was expected but not passed.
 */
struct MissingValueException : public CommandArgumentException {
    explicit MissingValueException(const string arg):
            CommandArgumentException(arg, "Option needs a value: " + arg)
    {}
};

/**
 * MissingFlagException should be thrown when a value was given but no flag was present.
 */
struct MissingFlagException : public CommandArgumentException {
    explicit MissingFlagException(const string arg):
            CommandArgumentException(arg, "Value without flag: " + arg)
    {}
};

/**
 * UnexpectedValueException should be thrown when a value was passed but not expected.
 */
struct UnexpectedValueException : public CommandArgumentException {
    explicit UnexpectedValueException(const string arg):
            CommandArgumentException(arg, "Option doesn't take a value: " + arg)
    {}
};

/**
 * UnexpectedPositionalException should be thrown when an argument was passed when no more can be accepted.
 */
struct UnexpectedPositionalException : public CommandArgumentException {
    explicit UnexpectedPositionalException(const string arg):
            CommandArgumentException(arg, "Unexpected argument: " + arg)
    {}
};

/**
 * MissingPositionalExpection should be thrown when an argument was required, but not provided.
 */
struct MissingPositionalException : public CommandArgumentException {
    explicit MissingPositionalException(const string arg):
            CommandArgumentException(arg, "Missing argument: " + arg)
    {}
};

/**
 * RepositoryExistsException should be thrown when Metro attempts to create a repository in a directory which already has one.
 */
struct RepositoryExistsException : public MetroException {
    explicit RepositoryExistsException():
        MetroException("There is already a repository in this directory.")
    {}
};

/**
 * RepositoryNotExistsException should be thrown when a command is used which requires a repository, but there is currently none.
 */
struct RepositoryNotExistsException : public MetroException {
    explicit RepositoryNotExistsException():
            MetroException("There is no Git repository in this directory.\nYou can create one with metro create.")
    {}
};

/**
 * CurrentlyMergingException should be thrown when a merge is in progress and an operation cannot be performed until the merge has finished.
 */
struct CurrentlyMergingException : public MetroException {
    explicit CurrentlyMergingException():
            MetroException("Branch has conflicts, please finish resolving them.\nRun metro resolve when you are done.")
    {}
};

/**
 * NotMergingException should be thrown when a merge is not in progress and an operation cannot be performed unless a merge is in progress.
 */
struct NotMergingException : public MetroException {
    explicit NotMergingException():
            MetroException("You can only resolve conflicts while absorbing.")
    {}
};

/**
 * BranchNotFoundException should be thrown when an access is attempted to a branch which does not exist.
 */
struct BranchNotFoundException : public MetroException {
    explicit BranchNotFoundException():
            MetroException("Branch not found.")
    {}

    explicit BranchNotFoundException(const string& branch):
            MetroException("Branch \"" + string(branch) + "\" not found.")
    {}
};

/**
 * UnnecessaryMergeException should be thrown when the user attempts to manually start a merge when none is required.
 */
struct UnnecessaryMergeException : public MetroException {
    explicit UnnecessaryMergeException():
            MetroException("Nothing to absorb.")
    {}
};

/**
 * UnsupportedOperationException should be thrown as a general case for an unsupported action by the user.
 */
struct UnsupportedOperationException : public MetroException {
    explicit UnsupportedOperationException(const char* message):
        MetroException(message)
    {}
};

/**
 * ANSIException should be thrown when a command requiring ANSI to be enabled is issued, but fails
 */
struct ANSIException : public MetroException {
    explicit ANSIException():
            MetroException("This terminal is incompatible with special character features.")
    {}
};

/**
 * AttachedWIPException should be thrown when the WIP is not detached from the branch, but should be.
 */
struct AttachedWIPException : public MetroException {
    explicit AttachedWIPException():
            MetroException("This can only be executed on a detached WIP.\nYou can detach the WIP using 'metro fix detach'")
    {}
};