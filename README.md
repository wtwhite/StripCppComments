# C++ Comment Stripper

A programming exercise completed for my Research Software Engineer job application to Veracity at Victoria University.

## Instructions for Linux/Unix

```sh
$ sudo apt-get install cmake # Or similar; only needed if cmake >= 3.14 is not already installed
$ git clone https://github.com/wtwhite/StripCppComments # Download repo
$ cd StripCppComments
$ mkdir build
$ cd build
$ cmake .. # Downloads local copy of GoogleTest test framework from GitHub
$ make # Build main executable and unit tests
$ ctest # Or ./Tests (run unit tests)
$ ./StripCppComments < some_cplusplus_file.cpp > that_file_without_comments.cpp
```

## Instructions for MS Visual C++ on Windows

- Install [Git for Windows](https://gitforwindows.org/) if not already installed
- Open a Command Prompt window, and download the repo by entering:
```
C:\Users\yourname\Documents>git clone https://github.com/wtwhite/StripCppComments
```
- Open MS Visual Studio 17.0.1 or later
- At the "What would you like to do?" dialog, choose "Open a local file or folder" and navigate to the cloned repo
- Hit F7 to build main executable and unit tests
- To run tests, choose "Run All Tests" from the Test menu
- To run the main executable, in the Command Prompt window from earlier, enter:
```
C:\Users\yourname\Documents>StripCppComments\out\build\x64-Debug\StripCppComments < some_cplusplus_file.cpp > that_file_without_comments.cpp
```

## Design choices

### Features:
- **Accuracy.** Except with respect to the known issues listed under "Limitations", a well-formed C++ input file should be transformed into a well-formed and semantically equivalent comment-free output file. In particular:
- **Correct handling, and exact preservation, of backslash-newline line continuations**, which can occur *inside* string backslash-escape sequences or even *comment markers*:
```c++
// A normal single-line comment

char* a_string = "A single string containing a double-quote character\\
"<---here";

int some_code;
/\
\
\
/ A single-line comment split across 4 lines by 3 backslash-newline line continuations
int some_more_code;
```
- **Streaming state-machine design with 1-character lookahead for guaranteed tiny memory usage and usability in a pipeline.** To achieve streaming, bounded memory *and* correct handling of line continuations required decomposing the input stream in an unusual way -- treating it not as a sequence of characters, but rather a sequence of (count, character) *pairs*, where the count is the number of backslash-newline character pairs immediately preceding the character. See `BackslashNewlineReader` in `CommentStripper.cpp`.
- **No regexes or external parser libraries.** The standard C++ library has regexes, but it is unlikely that they can be used in a streaming, bounded-lookahead design.
- Multiline comments are replaced with a single space character, so that `abc/*---*/def` continues to parse as 2 tokens. This is also how [the C++ standard prescribes](https://en.cppreference.com/w/cpp/comment) a compiler should internally handle them.
- Graceful handling of unterminated strings and multiline comments.
- Correct handling of multicharacter literals (e.g., `'ABC'`). Their behaviour is implementation-defined according to the C++ standard, so preprocessing tools should leave them intact.
- Portable [`cmake`](https://cmake.org/)-based build with [GoogleTest](https://github.com/google/googletest) unit tests: build and test easily on Linux or Windows.

### Limitations:
The program is not especially future-proof due to its state-machine design, which tends to make maintenance cumbersome. Its straightforward implementation using nested `switch` statements is concise but error-prone, especially in the absence of a standard C++ mechanism for checking that a `switch` statement's `case`s are exhaustive (`g++` has `-Wswitch`, at least).

`tests.cpp` contains a disabled test (that would currently fail) corresponding to each feature known not to be implemented:
- **Raw strings.** Slashes and quote characters are permitted in raw-string delimiters, which can be up to 16 characters long, so correctly parsing these would necessitate 16-character lookahead to handle the likes of:
```c++
cout << R""//NOODLE(
Some text
")"//NOTYET, we are still inside the string...
)"//NOODLE";
```
- **Carriage returns.** The bottom of `tests.cpp` explains why it is impossible to both preserve carriage return characters, and efficiently handle line continuations that may or may not include them. As implemented, "pure" newlines become `\r\n` pairs on Windows; on other platforms, carriage returns are preserved as-is, but are not permitted inside line continuations.
- **Single quotes in numeric literals.** Since C++14, numeric literals can contain interspersed single quote characters (e.g., `1'234`), which we don't attempt to handle -- these will be treated as starting a multicharacter literal, which can lead to incorrect output when combined with comments.
- **Trigraphs.** Ancient C and C++ compilers allowed some characters to be specified using 3-character "trigraphs", e.g., using `??/` instead of `\`. This translation occurs very early -- it can affect line continuations and escape sequences in strings. Trigraphs are almost never used in practice, and were finally dropped from the standard in C++17. We don't handle them.
