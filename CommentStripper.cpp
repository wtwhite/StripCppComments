#include <vector>
#include <cctype>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace commentstripper {
	// Line continuation with <backslash><newline> complicates parsing, since any number of these pairs can appear even in the
	// middle of a "//" single-line comment marker ("|" chars below just show the "page boundary"):
	//
	// |// Ordinary single-line comment                                  |
	// |                                                                 |
	// |/\                                                               |
	// |/ Single-line comment with comment marker split over 2 lines!    |
	// |int some_code;   /\                                              |
	// |\                                                                |
	// |\                                                                |
	// |/ Another single\                                                |
	// | line com\                                                       |
	// |ment!                                                            |
	// |float more_code;                                                 |
	//
	// Another quirk is that these pairs are parsed at a different level than the backslashes used in string and character
	// literals, so, e.g.:
	//
	// |cout << "Line with one backslash\                                |
	// |x41<--there." << endl;                                           |
	// |cout << "Line with two backslashes\\                             |
	// |x41<--there." << endl;                                           |
	// |cout << "Line with three backslashes\\\                          |
	// |x41<--there." << endl;                                           |
	//
	// produces:
	//
	// |Line with one backslashx41<--there.                              |
	// |Line with two backslashesA<--there.                              |
	// |Line with three backslashes\x41<--there.                         |
	//
	// (Note in particular that the final backslash on the line ending with two backslashes retains its line-continuing
	// power, and the escape sequence begun by its first backslash continues on the second line, resulting in "\x41" == "A".)
	//
	// To reproduce all these <backslash><newline> pairs in the output while bounding memory usage, we treat the input not as
	// a sequence of characters but as a sequence of (nBackslashNewlinePairs, char) pairs, with nBackslashNewlinePairs
	// most of the time being 0.
	class BackslashNewlineReader {
	public:
		BackslashNewlineReader(istream& is) : is(is), nBackslashNewlinePairs(0), backslash(false), lookedAhead(false), lookaheadBuf() {}

		pair<unsigned, int> get() {
			while (true) {
				int c = getWithLookahead();	// int so EOF will fit

				if (backslash) {
					backslash = false;

					if (c == '\n') {
						++nBackslashNewlinePairs;
					} else {
						pushBack(c);
						auto retVal = make_pair(nBackslashNewlinePairs, static_cast<int>('\\'));
						nBackslashNewlinePairs = 0;
						return retVal;
					}
				} else {
					if (c == '\\') {
						backslash = true;
					} else {
						auto retVal = make_pair(nBackslashNewlinePairs, c);
						nBackslashNewlinePairs = 0;
						return retVal;
					}
				}
			}
		}

		bool eof() {
			return is.eof() && !lookedAhead;
		}

	private:
		int getWithLookahead() {
			if (lookedAhead) {
				lookedAhead = false;
				return lookaheadBuf;
			} else {
				return is.get();
			}
		}

		void pushBack(int c) {
			lookedAhead = true;
			lookaheadBuf = c;
		}

		istream& is;
		bool backslash;	// Did we just read a backslash?
		unsigned nBackslashNewlinePairs;
		int lookaheadBuf;
		bool lookedAhead;	// Did we read a character too early?
	};

	void putOnlyBackslashNewlinePairs(ostream& os, unsigned nBackslashNewlinePairs) {
		for (unsigned i = 0; i < nBackslashNewlinePairs; ++i) {
			os.put('\\');
			os.put('\n');
		}
	}

	void put(ostream& os, pair<unsigned, int> p) {
		putOnlyBackslashNewlinePairs(os, p.first);
		os.put(p.second);
	}

	void stripComments(istream& is, ostream& os) {
		enum class State {
			NORMAL,
			IN_STRING,
			IN_CHAR,
			SLASH,
			ASTERISK_IN_MULTILINE_COMMENT,
			IN_SINGLE_LINE_COMMENT,
			IN_MULTILINE_COMMENT
		};

		State state = State::NORMAL;
		bool backslashSeen = false;
		BackslashNewlineReader reader{is};

		while (true) {
			auto p = reader.get();
			auto c = p.second;	// Convenience
			if (reader.eof()) {
				putOnlyBackslashNewlinePairs(os, p.first);

				if (state == State::SLASH) {
					os.put('/');
				}

				break;
			}

			if (is.bad()) {
				throw runtime_error{"An unexpected error occurred while stripping comments"};
			}

			switch (state) {
			case State::NORMAL:
				switch (c) {
				case '"':
					state = State::IN_STRING;
					put(os, p);
					backslashSeen = false;
					break;

				case '\'':
					state = State::IN_CHAR;
					put(os, p);
					backslashSeen = false;
					break;

				case '\\':
					put(os, p);
					backslashSeen = !backslashSeen;
					break;

				case '/':
					state = State::SLASH;
					putOnlyBackslashNewlinePairs(os, p.first);
					backslashSeen = false;
					break;

				default:
					put(os, p);
					backslashSeen = false;
					break;
				}
				break;

			case State::IN_STRING:
				switch (c) {
				case '"': // Fall through
				case '\n': // Newline before end of string: Syntax error
					if (!backslashSeen) {
						state = State::NORMAL;
					}
					put(os, p);
					backslashSeen = false;
					break;

				case '\\':
					put(os, p);
					backslashSeen = !backslashSeen;
					break;

				default:
					put(os, p);
					backslashSeen = false;
					break;
				}
				break;

			case State::IN_CHAR:
				switch (c) {
				case '\'': // Fall through
				case '\n': // Newline before end of character literal: Syntax error
					if (!backslashSeen) {
						state = State::NORMAL;
					}
					put(os, p);
					backslashSeen = false;
					break;

				case '\\':
					put(os, p);
					backslashSeen = !backslashSeen;
					break;

				default:
					put(os, p);
					backslashSeen = false;
					break;
				}
				break;

			case State::SLASH:
				switch (c) {
				case '/':
					state = State::IN_SINGLE_LINE_COMMENT;
					break;

				case '*':
					state = State::IN_MULTILINE_COMMENT;
					break;

				default:
					state = State::NORMAL;
					os.put('/');
					put(os, p);
					break;
				}
				break;

			case State::IN_SINGLE_LINE_COMMENT:
				if (c == '\n') {
					state = State::NORMAL;
					os.put('\n');
				}
				break;

			case State::IN_MULTILINE_COMMENT:
				if (c == '*') {
					state = State::ASTERISK_IN_MULTILINE_COMMENT;
				}
				break;
				
			case State::ASTERISK_IN_MULTILINE_COMMENT:
				switch (c) {
				case '/':
					os.put(' ');	// Insert a space to preserve parsing of "abc/*---*/def" as 2 tokens
					state = State::NORMAL;
					break;

				case '*': break;	// Stay in ASTERISK_IN_MULTILINE_COMMENT

				default:
					state = State::IN_MULTILINE_COMMENT;
					break;
				}
				break;
			}
		}
	}
}
