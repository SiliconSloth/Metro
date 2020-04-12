/*
 * This file should contain anything not relevant to git or any other specfic part of Metro
 */

#pragma once
#include "gitwrapper/types.h"

using namespace std;

/**
 * Check if a non-terminated char array is equal to a null-terminated char array.
 * The third argument is the length of the first string.
 * @param str1 First non-terminating char array to compare equality.
 * @param str2 Second null-terminating char array to compare equality.
 * @param n Length of string.
 * @return True if strings are equal.
 *
 * Works by checking both strings are the same length (ignoring the terminator)
 * and then comparing the first n characters of each.
 */
#define strnequal(str1, str2, n) ((n) == sizeof(str2)-1 && !strncmp(str1, str2, n))

/**
 * Convert a string to a non-negative integer, returning -1 on failure.
 * @param str The string reference to attempt to convert.
 * @return The positive integer represented by that string, or -1 on failure.
 */
unsigned int parse_pos_int(const string& str);

/**
 * Tests whether a certain string contains a certain prefix.
 * @param str The string reference to compare against the prefix.
 * @param pre The prefix to search for in 'str'.
 * @return True if pre is the prefix of str.
 *
 * For example:
 * @code
 * string test("metro");
 * has_prefix(test, "met"); // Returns true
 * has_prefix(test, "git"); // Returns false
 * @endcode
 */
bool has_prefix(string const& str, string pre);

/**
 * Tests whether a certain string contains a certain suffix.
 * @param str The string reference to compare against the suffix.
 * @param suff The suffix to search for in 'str'.
 * @return True if suff is the suffix of str.
 *
 * For example:
 * @code
 * string test("metro");
 * has_suffix(test, "tro"); // Returns true
 * has_suffix(test, "git"); // Returns false
 * @endcode
 */
bool has_suffix(string const& str, string suff);

/**
 * True if the string contains no non-whitespace characters.
 * @param s String reference to check for whitespace.
 * @return True if the string only contains whitespace characters.
 */
bool whitespace_only(const string& s);

/**
 * Split the given string around the fist occurrence of the given character.
 * If the character is not found, the input string is returned as the first string and "" as the second.
 * The outputs are stored in before and after.
 * @param str The string to split.
 * @param c The character to split at when found.
 * @param before The string reference to store the first half of the string.
 * @param after The string reference to store the second half of the string.
 */
void split_at_first(string const& str, char const& c, string & before, string & after);

/**
 * Split a string consisting of a command followed by space-separated arguments
 * into separate arguments, taking into account quotation marks.
 * The behaviour of this function is modelled on Windows' CommandLineToArgvW function.
 * @param command Full command to split up the arguments of.
 * @return A vector containing the individual strings.
 */
vector<string> split_args(const string& command);

/**
 * Print the given list of options and their contractions and descriptions in a user-friendly manner.
 * @param options List of options to print out.
 */
void print_options(const vector<string>& options);

/**
 * Print string right-padded to given length.
 * @param str String to print our.
 * @param len Number of characters the entire printed word should be, with spaces padding the end.
 *
 * Note that if the word is longer than the pad length, the word will be printed normally.
 */
void print_padded(const string& str, size_t len);

/**
 * Reads the file on the path given, returning it.
 * @param path Path to read the file from.
 * @return The file contents.
 */
string read_all(const string& path);

/**
 * Writes to the file on the path given.
 * @param text Text to write to the file.
 * @param path Path file to write to.
 */
void write_all(const string& text, const string& path);

/**
 * Converts a git::Time object to a corresponding string format.
 * @param time The time as a git::Time object.
 * @return The time in a string format.
 */
string time_to_string(git_time time);

/**
 * Sets the colour of the terminal cross-platform using a set of rules.
 * @param colour A string representing the colour to set the following text to.
 * @param handle The Windows commandline handle.
 *
 * Should be in format like "rgbi-----" or "r--i-gb--"
 * rgb is colour, i is intensity. The first 4 are the
 * text, and the second 4 are the background. The last
 * one is the mode: - for all, f for foreground, b for
 * background and r for reset
 */
void set_text_colour(const string colour, void* handle);

/**
 * Prints a progress bar using the given percentage.
 * @param progress A percentage between 0 and 1 of how far along the progress bar is.
 */
void print_progress(unsigned int progress);

/**
 * Prints a progress bar using the given percentage an indicator at the end
 * specifying the current amount transferred and transfer speed.
 * @param progress A percentage between 0 and 1 of how far along the progress bar is.
 * @param bytes The number of bytes currently transferred.
 */
void print_progress(unsigned int progress, size_t bytes);

/**
 * If a progress bar has been printed but not yet cleared, this will clear that progress bar.
 *
 * Otherwise nothing will happen.
 */
void clear_progress_bar();

/**
 * Clears a line of all text.
 */
void clear_line();

/**
 * Converts bytes to a string representation - eg. 4000 bytes will become "4KB".
 * @param bytes Bytes to convert to string.
 * @return String representing final size, reduced to the largest appropriate size up to TB's.
 */
string bytes_to_string(size_t bytes);

/**
 * Enables ANSI codes on Windows.
 *
 * Does nothing on other OSs.
 */
void enable_ansi();


/**
 * Disables ANSI codes on Windows.
 *
 * Does nothing on other OSs.
 */
void disable_ansi();