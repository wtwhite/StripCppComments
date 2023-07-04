#include <iostream>
#include <fstream>
#include <set>
#include "CommentStripper.h"

using namespace std;

int main(int argc, char** argv) {
	ios_base::sync_with_stdio(false);	// We never use stdio (printf() etc.). Improves perf
	cin.tie(nullptr);	// Avoid flushing at every write->read transition. Improves perf

	if (argc == 2 && set<string>{"--help", "-h", "/?"}.count(argv[1])) {
		cerr << "Usage: StripComments [<] some_cpp_file.cpp > that_file_without_comments.cpp\n";
		return 0;
	}

	try {
		istream* namedInputFile = (argc == 2 ? new ifstream(argv[1]) : nullptr);
		if (namedInputFile && !*namedInputFile) {
			delete namedInputFile;
			cerr << "Could not open input file '" << argv[1] << "', aborting." << endl;
			return 1;
		}

		commentstripper::stripComments(namedInputFile ? *namedInputFile : cin, cout);

		delete namedInputFile;
		return 0;
	} catch (exception& e) {
		cerr << "An error occurred: " << e.what() << endl;
		return 1;
	}
}
