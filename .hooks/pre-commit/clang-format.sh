#!/bin/bash
if ! hash clang-format 2>/dev/null; then
	echo "'clang-format' must be installed in a global environment."
	exit 1
fi

find ./source -type f -name "*.h" -or -name "*.hpp" -or -name "*.c" -or -name "*.cpp" -exec clang-format -i '{}' \;
