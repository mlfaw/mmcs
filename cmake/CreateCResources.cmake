
# "create_resources" (c) by Youka, Gary, and mlfaw
# "create_resources" is licensed under a
# Creative Commons Attribution-ShareAlike 3.0 Unported License.
# You should have received a copy of the license along with this
# work.  If not, see <http://creativecommons.org/licenses/by-sa/3.0/>.

# Retrieved from Stack Overflow and with edits incorporated.

# "Embed resources (eg, shader code; images) into executable/library with CMake"
#   Question link: Nicu Stiurca (author/asker) - https://stackoverflow.com/q/11813271
#   Answer link: https://stackoverflow.com/a/27206982
#   Original author:
#     Youka - https://stackoverflow.com/users/2546626/youka
#   Author for Regex edit & stringifying (const char*):
#     Gary - https://stackoverflow.com/users/128433/gary
#     https://stackoverflow.com/questions/11813271/embed-resources-eg-shader-code-images-into-executable-library-with-cmake/27206982#comment57327422_27206982

# Additional Edits:
#   Produces ${output}.h & ${output}.c.
#   Generated variable names include a "GEN_" prefix.
#   Misc C++ stuff (extern "C").
#   Changed functions around so single files can be generated...
#   Removed const from variables...

function(resources_start output)
	# Create empty output file
	file(WRITE "${output}.h" "")
	file(WRITE "${output}.c" "")
	# Get short filename
	string(REGEX MATCH "([^/]+)$" filename "${output}")
	#
	file(APPEND "${output}.h" "#pragma once\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")
	file(APPEND "${output}.c" "#include \"${filename}.h\"\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")
endfunction()

function(resources_end output)
	# Close out the extern "C" stuff
	file(APPEND "${output}.h" "#ifdef __cplusplus\n}\n#endif\n")
	file(APPEND "${output}.c" "#ifdef __cplusplus\n}\n#endif\n")
endfunction()

function(resources_append_raw input output)
	# Get short filename
	string(REGEX MATCH "([^/]+)$" filename "${input}")
	# Replace filename spaces & extension separator for C compatibility
	string(REGEX REPLACE "\\.| |-" "_" filename "${filename}")
	# Read hex data from file
	file(READ "${input}" filedata HEX)
	# Convert hex data for C compatibility
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
	# Append data to output file
	file(APPEND "${output}.h" "extern unsigned char GENERATED_${filename}[];\nextern unsigned GENERATED_${filename}_size;\n\n")
	file(APPEND "${output}.c" "unsigned char GENERATED_${filename}[] = {${filedata}};\nunsigned GENERATED_${filename}_size = sizeof(GENERATED_${filename});\n\n")
endfunction()

# The version that creates a `char *` for the data and includes a 0x00 byte at the end.
# The size variable has 1 subtracted from it too.
function(resources_append_string input output)
	# Get short filename
	string(REGEX MATCH "([^/]+)$" filename "${input}")
	# Replace filename spaces & extension separator for C compatibility
	string(REGEX REPLACE "\\.| |-" "_" filename "${filename}")
	# Read hex data from file
	file(READ ${input} filedata HEX)
	# Convert hex data for C compatibility
	string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
	# Append data to output file
	file(APPEND "${output}.h" "extern char GENERATED_${filename}[];\nextern unsigned GENERATED_${filename}_size;\n\n")
	file(APPEND "${output}.c" "char GENERATED_${filename}[] = {${filedata}0x00};\nunsigned GENERATED_${filename}_size = sizeof(GENERATED_${filename})-1;\n\n")
endfunction()

function(resources_file_raw input output)
	resources_start("${output}")
	resources_append_raw("${input}" "${output}")
	resources_end("${output}")
endfunction()

function(resources_file_string input output)
	resources_start("${output}")
	resources_append_string("${input}" "${output}")
	resources_end("${output}")
endfunction()

function(resources_dir_raw input output)
	# Collect input files
	file(GLOB bins "${input}/*")
	resources_start("${output}")
	foreach(bin ${bins})
		resources_append_raw("${bin}" "${output}")
	endforeach()
	resources_end("${output}")
endfunction()

function(resources_dir_string input output)
	# Collect input files
	file(GLOB bins "${input}/*")
	resources_start("${output}")
	foreach(bin ${bins})
		resources_append_string("${bin}" "${output}")
	endforeach()
	resources_end("${output}")
endfunction()
