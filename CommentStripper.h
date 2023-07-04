#pragma once

#include <iostream>

namespace commentstripper {
	/**
	 * Writes is to os, stripping all C++ single-line and multiline comments as it goes.
	 * See bottom of tests.cpp for known limitations.
	 * Throws a runtime_error on I/O failure.
	 */
	void stripComments(std::istream& is, std::ostream& os);
}
