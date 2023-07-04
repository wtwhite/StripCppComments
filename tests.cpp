#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "CommentStripper.h"

using namespace std;
using namespace commentstripper;

// Unhygienic, but convenient
#define EXPECT_UNCHANGED() EXPECT_STREQ(oss.str().c_str(), iss.str().c_str());

// Boundary conditions
TEST(CommentStripper, EmptyStringIsUnchanged) {
	istringstream iss("");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, OneLetterStringIsUnchanged) {
	istringstream iss("a");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustNulCharIsUnchanged) {
	istringstream iss(string("\0", 1));
	EXPECT_EQ(iss.str().size(), 1);	// Sanity check input
	ostringstream oss;
	stripComments(iss, oss);
	ASSERT_EQ(oss.str().size(), 1);
	EXPECT_EQ(oss.str()[0], iss.str()[0]);
}

TEST(CommentStripper, JustNewlineIsUnchanged) {
	istringstream iss("\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustSlashIsUnchanged) {
	istringstream iss("/");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustBackslashIsUnchanged) {
	istringstream iss("\\");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

// Basic passthrough tests
TEST(CommentStripper, CommentFreeInputWithoutTrailingNewlineIsUnchanged) {
	istringstream iss("No comments here.");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, CommentFreeInputWithTrailingNewlineIsUnchanged) {
	istringstream iss("No comments here, trailing newline though.\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, CommentFreeMultilineInputIsUnchanged) {
	istringstream iss(
		"No comments here\n"
		"Line 2\n"
		"The end.\n"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, CommentFreeInputWithUnterminatedStringIsUnchanged) {
	istringstream iss(
		"No comments here\n"
		"Line 2 contains an \"unterminated string\n"
		"The end.\n"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, CommentFreeInputWithUnterminatedMulticharLiteralIsUnchanged) {
	istringstream iss(
		"No comments here\n"
		"Line 2 contains an 'unterminated multicharacter literal\n"
		"The end.\n"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, CommentFreeMultilineInputWithNulCharsIsUnchanged) {
	const char rawStr[] =
		"No comments \0here\n"
		"Line 2\n"
		"Th\0e end.\0\0\n"
		"\0";
	istringstream iss(string(rawStr, sizeof rawStr));
	ASSERT_GT(iss.str().size(), 20);	// Sanity check input
	ostringstream oss;
	stripComments(iss, oss);
	ASSERT_EQ(oss.str().size(), iss.str().size());
	for (unsigned i = 0; i < oss.str().size(); ++i) {
		EXPECT_EQ(oss.str()[i], iss.str()[i]);
	}
}

// Single-line comments
TEST(CommentStripper, SingleLineCommentIsRemoved) {
	istringstream iss("Single line comment//This should be removed");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Single line comment");
}

TEST(CommentStripper, SingleLineCommentWithNewlineIsRemoved) {
	istringstream iss("Single line comment//This should be removed\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Single line comment\n");
}

TEST(CommentStripper, SingleLineCommentWithFollowingLinesIsRemoved) {
	istringstream iss(
		"Single line comment//This should be removed\n"
		"Next line\n"
		"Final line"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"Single line comment\n"
		"Next line\n"
		"Final line"
	);
}

TEST(CommentStripper, StringContainingSingleLineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string that //looks like a comment\" but isn't");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, StringFollowedBySingleLineCommentRemovesComment) {
	istringstream iss("Single line with a \"string\" then //a comment");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Single line with a \"string\" then ");
}

TEST(CommentStripper, StringContainingDoubleQuoteThenSingleLineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string containing a double quote\\\" and then what //looks like a comment\"");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, SingleLineCommentAfterStringEndingWithQuotedBackslashIsRemoved) {
	istringstream iss("Single line with a \"string ending with a backslash \\\\\" and then //a comment\"");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Single line with a \"string ending with a backslash \\\\\" and then ");
}

TEST(CommentStripper, StringContainingBackslashThenDoubleQuoteThenSingleLineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string containing a backslash and double quote \\\\\\\" and then what //looks like a comment\"");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, MulticharLiteralContainingSingleLineCommentIsUnchanged) {
	istringstream iss("Single line with a 'multicharacter literal that //looks like a comment' but isn't");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

// Multiline comments.
// Note that multiline comments are replaced with a single space so that "abc/*---*/def" continues to parse as 2 tokens.
TEST(CommentStripper, MultilineCommentAtStartIsRemoved) {
	istringstream iss("/*This should be removed*/Multiline comment");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), " Multiline comment");
}

TEST(CommentStripper, MultilineCommentAtEndIsRemoved) {
	istringstream iss("Multiline comment/*This should be removed*/");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Multiline comment ");
}

TEST(CommentStripper, UnterminatedMultilineCommentIsRemoved) {
	istringstream iss("Multiline comment/*This should still be removed");
	ostringstream oss;
	stripComments(iss, oss);

	// Permit result to end with a space, or not
	string target = "Multiline comment";
	if (oss.str() == target || oss.str() == target + " ") {
		SUCCEED();
	} else {
		FAIL();
	}
}

TEST(CommentStripper, MultilineCommentWithTrailingAsterisksIsRemoved) {
	istringstream iss("Multiline comment/* This should be removed ***/Back out again\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Multiline comment Back out again\n");
}

TEST(CommentStripper, MultilineCommentContainingNewlinesAndAsterisksIsRemoved) {
	istringstream iss(
		"Multiline comment/* This should\n"
		" be\n"
		" *** removed */Back out again\n"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Multiline comment Back out again\n");
}

TEST(CommentStripper, StringContainingMultilineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string that /*looks like a comment*/\" but isn't");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, StringContainingUnterminatedMultilineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string that /*looks like an unterminated comment\" but isn't");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, StringFollowedByMultilineCommentRemovesComment) {
	istringstream iss("Single line with a \"string\" then /*a comment*/.");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Single line with a \"string\" then  .");
}

TEST(CommentStripper, StringWithDoubleQuoteThenMultilineCommentIsUnchanged) {
	istringstream iss("Single line with a \"string containing a double quote\\\" and then what /*looks like a comment*/\"");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, MulticharLiteralContainingMultilineCommentIsUnchanged) {
	istringstream iss("Single line with a 'multicharacter literal that /*looks like a comment*/' but isn't");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, MultilineCommentsDoNotNest) {
	istringstream iss("Apparently /* nested /* multiline comments */, but in fact we are already outside */ the comment");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "Apparently  , but in fact we are already outside */ the comment");
}

TEST(CommentStripper, NoSingleLineCommentInsideMultilineComments) {
	istringstream iss(
		"A /* multiline comment\n"
		"That contains what appears to be a //single-line comment that masks its end */, but doesn't"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "A  , but doesn't");
}

TEST(CommentStripper, FlushMultilineCommentLeavesSeparateTokensEitherSide) {
	istringstream iss("abc/* multiline comment */def");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "abc def");
}

// Backslash-newline line continuation
TEST(CommentStripper, JustTwoBackslashesAreUnchanged) {
	istringstream iss("\\\\");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustBackslashNewlineIsUnchanged) {
	istringstream iss("\\\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustTwoBackslashNewlinesIsUnchanged) {
	istringstream iss("\\\n\\\n");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, JustBackslashLetterIsUnchanged) {
	istringstream iss("\\X");
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, BackslashNewlineIsUnchanged) {
	istringstream iss(
		"Here\\\n"
		" is a backslash-newline pair."
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, MultiBackslashNewlineIsUnchanged) {
	istringstream iss(
		"Here\\\n"
		"\\\n"
		"\\\n"
		" are 3 consecutive backslash-newline pairs."
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();
}

TEST(CommentStripper, StringSurvivesBackslashNewline) {
	istringstream iss(
		"char* a_string = \"Line 1 of a string.\\\n"
		" Line 2 but still in the string so //this is not a comment\"; /*but this is*/, and //so is this"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"char* a_string = \"Line 1 of a string.\\\n"
		" Line 2 but still in the string so //this is not a comment\";  , and "
	);
}

TEST(CommentStripper, SingleLineCommentSurvivesBackslashNewline) {
	istringstream iss(
		"int some_code; // Single-line\\\n"
		" comment split across 2 lines\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code; \n"
		"int more_code;"
	);
}

TEST(CommentStripper, SingleLineCommentSurvivesBackslashNewlineInsideCommentMarker) {
	istringstream iss(
		"int some_code; /\\\n"
		"/ Single-line comment with comment marker split across 2 lines\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code; \n"
		"int more_code;"
	);
}

TEST(CommentStripper, SingleLineCommentSurvivesMultipleBackslashNewlineInsideCommentMarker) {
	istringstream iss(
		"int some_code; /\\\n"
		"\\\n"
		"\\\n"
		"/ Single-line comment with comment marker split across 4 lines\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code; \n"
		"int more_code;"
	);
}

TEST(CommentStripper, MultilineCommentContainingBackslashNewlineIsRemoved) {
	istringstream iss(
		"int some_code; /* Multiline\\\n"
		" comment split across 2 lines */\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code;  \n"
		"int more_code;"
	);
}

TEST(CommentStripper, MultilineCommentSurvivesBackslashNewlineInsideCommentStartMarker) {
	istringstream iss(
		"int some_code; /\\\n"
		"* Multiline comment with comment start marker split across 2 lines */\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code;  \n"
		"int more_code;"
	);
}

TEST(CommentStripper, MultilineCommentSurvivesMultipleBackslashNewlineInsideCommentStartMarker) {
	istringstream iss(
		"int some_code; /\\\n"
		"\\\n"
		"\\\n"
		"* Multiline comment with comment start marker split across 4 lines */\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code;  \n"
		"int more_code;"
	);
}

TEST(CommentStripper, MultilineCommentSurvivesBackslashNewlineInsideCommentEndMarker) {
	istringstream iss(
		"int some_code; /* Multiline comment with comment end marker split across 2 lines *\\\n"
		"/\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code;  \n"
		"int more_code;"
	);
}

TEST(CommentStripper, MultilineCommentSurvivesMultipleBackslashNewlineInsideCommentEndMarker) {
	istringstream iss(
		"int some_code; /* Multiline comment with comment end marker split across 4 lines *\\\n"
		"\\\n"
		"\\\n"
		"/\n"
		"int more_code;"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int some_code;  \n"
		"int more_code;"
	);
}

// Escape sequences
TEST(CommentStripper, BackslashNewlineDoesNotInterruptNewlineEscapeSequenceInString) {
	istringstream iss(
		"char* a_string = \"Line 1 of inner string\\\\\n"
		"nLine 2 still inside the string so //this is not a comment\"; /*but this is*/, and //so is this"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"char* a_string = \"Line 1 of inner string\\\\\n"
		"nLine 2 still inside the string so //this is not a comment\";  , and "
	);
}

TEST(CommentStripper, BackslashNewlineDoesNotInterruptDoubleQuoteEscapeSequenceInString) {
	istringstream iss(
		"char* a_string = \"Inner string with double-quote escape sequence split across 2 lines\\\\\n"
		"\"Line 2 still inside the string so //this is not a comment\"; /*but this is*/, and //so is this"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"char* a_string = \"Inner string with double-quote escape sequence split across 2 lines\\\\\n"
		"\"Line 2 still inside the string so //this is not a comment\";  , and "
	);
}

// Tests that would fail if "DISABLED_" were removed from their names, due to limitations in the code

// Handling raw strings (available since C++11) would require 16-character lookahead to check the delimiters
TEST(CommentStripper, DISABLED_RawStringWithCommentInDelimitersAndContainingCommentIsUnchanged) {
	istringstream iss(
		"cout << R\"\"//NOODLE(\n"		// "/", "\"" are permitted in raw string delimiters
		"Some text\n"
		"\")\"//NOTYET, we are still inside the string...\n"
		")\"//NOODLE\";"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_UNCHANGED();

	// No-op demonstration of syntactic validity:
	auto notCalled = [] {
		cout << R""//NOODLE(
Some text
")"//NOTYET, we are still inside the string...
)"//NOODLE";
	};
}

// Since C++14, numeric literals can be written with single-quote digit separators, like 1'234.
TEST(CommentStripper, DISABLED_QuotesInNumericLiteralsDoNotHideComments) {
	istringstream iss(
		"int x = 1'234//This is really a comment' /* So this does not start a new multiline comment\n"
		"+1\n"
		"/* x should be 1235; misinterpreting first ' as start of char literal gives 1234. End 'both' multiline comments: */\n"
		";"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(),
		"int x = 1'234\n"
		"+1\n"
		" \n"
		";"
	);
}

// On Windows, file-based streams default to text mode, with \r\n pairs translated to \n on input and back again
// on output. Although it would be possible to preserve all \r characters exactly while treating both <backslash><newline>
// and <backslash><carriage_return><newline> as line continuations, this would complicate the logic; also, doing this
// requires either the ability to seek arbitrarily far back in the input stream (ruling out use in a pipeline) or an
// unbounded amount of memory, because of the following input scenario:
//
//     A slash, followed by a large number of consecutive line continuations, each being either <backslash><newline> or
//     <backslash><carriage_return><newline>, followed by some non-backslash character X.
// 
// If X is a slash or asterisk, a comment has begun and we must discard all these characters; if it is
// any other character, we must print them all out. This means that we must either remember the location of the original
// slash in the input and seek back to it when we see X, or buffer the entire sequence in memory until we see X.
//
// The approach taken here is to rely on the default text translation mode, which on Windows will detect both types
// of line continuation but will translate all bare newlines to \r\n pairs, and on other platforms will ignore
// line continuations containing carriage returns.
TEST(CommentStripper, DISABLED_HandlesCarriageReturnInLineContinuation) {
	istringstream iss(
		"// Comment split into 2 lines using\\\r\n"
		" a line continuation containing a carriage return"
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "");
}

// Before C++17, trigraphs should be changed to their corresponding single characters (e.g., "??/" => "\"), and
// this happens even before line continuation.
TEST(CommentStripper, DISABLED_HandlesLineTrigrahInLineContinuation) {
	istringstream iss(
		"// Single-line comment extended with line-continuation using the backslash trigraph ??/\n"
		"to second line."
	);
	ostringstream oss;
	stripComments(iss, oss);
	EXPECT_STREQ(oss.str().c_str(), "");
}
