#!/bin/bash

# Usage
# - Call argparse after registering all Options and Commands.
#
#! Option Handlers: '-name', '--name'
# - $1 will be the exact key (The 'name' part of '-name', '--name').
# - $2 will be the value either assigned with = or as a second argument.
# - ARGPARSE_USED must be set to 1 if $2 was used by the handler.
# - A status code of 0 must be returned if parsing was successful.
# - A status code of 1 must be returned if parsing failed.
#
#! Command Handlers: 'name'
# - $1 will be the exact key ('name' part)
# - $2 will contain the remaining arguments.
# - ARGPARSE_USED must be set to the number of arguments used by the handler.
# - A status code of 0 must be returned if parsing was successful.
# - A status code of 1 must be returned if parsing failed.

################################################################################
# Variables
################################################################################
# Where is this script really?
SELF=`realpath "$0"`

# Known and Unknown arguments
declare -a ARGPARSE_KNOWN
declare -a ARGPARSE_UNKNOWN

# Options
declare -a ARGPARSE_OPTIONS

# Commands
declare -a ARGPARSE_COMMANDS

################################################################################
# Main Functionality
################################################################################
function argparse_split_option {
	OPTION="$1"
	IFS=";" read -a OPTION <<< ${OPTION}
	IFS="," read -a OPTION_NAMES <<< ${OPTION[0]}
	OPTION_FUNCTION=${OPTION[1]}
	OPTION_HELP=${OPTION[2]}
}

function argparse_parse_arg {
 	ARGPARSE_KEY=$1
 	ARGPARSE_VALUE=$2
	ARGPARSE_HAS_ASSIGN=`false`
 	if [[ "$KEY" =~ .*=.* ]]; then
		ARGPARSE_HAS_ASSIGN=`true`
 		VALUE=${${KEY}//.*=/}
 		KEY=${${KEY}//=.*/}
 	fi
}

function argparse_option {
 	# Parse Key=Value structures

	argparse_split_option ${3}
	for OPTION_NAME in "${OPTION_NAMES[@]}"; do
		if [[ "$1" == "${OPTION_NAME}" ]]; then
			${OPTION_FUNCTION} ${}
		fi
	done

}

function argparse {
	# Parse both Options and Commands
	while (( $# )); do
		argparse_parse_arg "$@"

		echo "$1"
		for ARGPARSE_OPTION in "${ARGPARSE_OPTIONS[@]}"; do
		done
		for ARGPARSE_COMMAND in "${ARGPARSE_COMMANDS[@]}"; do
			argparse_split_option ${ARGPARSE_COMMAND}
			echo " c $ARGPARSE_COMMAND"
		done

		if ARGPARSE_HAS_ASSIGN; then
			shift
		else
			shift 2
		fi
	done

#declare -a ARGS
#while (( $# )); do
#	ARGS+=("$1")
#	shift
#done
#	echo "$7"
#	# Parse the array one by one, filling either the known or unknown array.
#	while (( ${#ARGPARSE_ARGS[@]} )); do
#		argparse_parse
#		if $? =
#	done
}

################################################################################
# Command: Help
################################################################################

# function argparse_help {
# 	echo "${SELF} [<options>] <command> ..."
# 	echo "  Options: "

# 	echo ""
# 	echo "  Commands: "

# 	# We only handled $1, so return 0
# 	return 0
# }
# COMMANDS+=("-h,--help;argparse_help;Show this help.")


# function parse_flag {
# 	RET_NO_ARGS=0
# 	RET_ONE_ARG=1
# 	RET_TWO_ARG=2


# 	case "$KEY" in
# 		"-qt"|"--qt-version")
# 			QT_VERSION=${VALUE}
# 			return ${RET_TWO_ARG}
# 			;;
# 		"-ssl"|"--ssl-flavor")
# 			SSL=${VALUE}
# 			return ${RET_TWO_ARG}
# 			;;
# 		"-s"|"--silent"|"-q"|"--quiet")
# 			SILENT=TRUE
# 			return ${RET_ONE_ARG}
# 			;;
# 		*) # Nothing was parsed.
# 			return ${RET_NO_ARGS}
# 	esac
# }

