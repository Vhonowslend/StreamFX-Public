set(CLANG_FORMAT_BIN "clang-format" CACHE PATH "Path (or name) of the clang-format binary")

function(clang_format)
	cmake_parse_arguments(
		PARSE_ARGV 0
		_CLANG_FORMAT
		"DEPENDENCY;GLOBAL"
		"REGEX"
		"TARGETS"
	)

	if(NOT EXISTS ${CLANG_FORMAT_BIN})
		find_program(clang_format_bin_tmp ${CLANG_FORMAT_BIN})
		if(clang_format_bin_tmp)
			set(CLANG_FORMAT_BIN "${clang_format_bin_tmp}" CACHE PATH "Path (or name) of the clang-format binary")
			unset(clang_format_bin_tmp)
		else()
			message(WARNING "Clang: Could not find clang-format at path '${CLANG_FORMAT_BIN}'. Disabling clang-format...")
			return()
		endif()
	endif()

	if(NOT _CLANG_FORMAT_FILTER)
		set(_CLANG_FORMAT_FILTER "\.(h|hpp|c|cpp)$")
	endif()

	foreach(_target ${_CLANG_FORMAT_TARGETS})
#		get_target_property(target_name ${_target} NAME)

		get_target_property(target_sources_rel ${_target} SOURCES)
		set(target_sources "")
		foreach(source_relative ${target_sources_rel})
			get_filename_component(source_absolute ${source_relative} ABSOLUTE)
			list(APPEND target_sources ${source_absolute})
		endforeach()
		list(FILTER target_sources INCLUDE REGEX "${_CLANG_FORMAT_FILTER}")
		unset(target_sources_rel)
		
		get_target_property(target_source_dir_rel ${_target} SOURCE_DIR)
		get_filename_component(target_source_dir ${target_source_dir_rel} ABSOLUTE)
		unset(target_source_dir_rel)

		add_custom_target(${_target}_CLANG-FORMAT
			COMMAND
				${CLANG_FORMAT_BIN}
				-style=file
				-i
				${target_sources}
			COMMENT
				"clang-format: Formatting ${_target}..."
			WORKING_DIRECTORY
				${target_source_dir_rel}
		)

		if(_CLANG_FORMAT_DEPENDENCY)
			add_dependencies(${_target} ${_target}_CLANG-FORMAT)
		endif()

		if(_CLANG_FORMAT_GLOBAL)
			if(TARGET CLANG-FORMAT)
				add_dependencies(CLANG-FORMAT ${_target}_CLANG-FORMAT)
			else()
				add_custom_target(CLANG-FORMAT
					DEPENDS
						${_target}_CLANG-FORMAT
					COMMENT
						"clang-format: Formatting..."
				)
			endif()
		endif()
	endforeach()
endfunction()
