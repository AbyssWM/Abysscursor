# Exercise only --create, with disposable paths inside the build tree.
string(RANDOM LENGTH 16 ALPHABET 0123456789abcdef suffix)
set(root "${TEST_ROOT}/deletion-guard-${suffix}")
set(input "${root}/input")
file(MAKE_DIRECTORY "${input}/cursors")
file(WRITE "${input}/manifest.hl" "name = deletion-fixture\ncursors_directory = cursors\n")
file(WRITE "${root}/yes" "y\n")
file(WRITE "${root}/no" "n\n")
file(WRITE "${root}/eof" "")

function(run_create case response expected_result expected_message)
  execute_process(
    COMMAND "${UTIL}" --create "${input}" --output "${root}/${case}"
    INPUT_FILE "${root}/${response}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 10)
  if(NOT "${result}" STREQUAL "${expected_result}" OR NOT "${output}" MATCHES "${expected_message}")
    message(FATAL_ERROR "${case}: result=${result}\n${output}\n${error}")
  endif()
  if("${expected_message}" MATCHES "^Refusing" AND "${output}" MATCHES "About to delete")
    message(FATAL_ERROR "${case}: unsafe output reached the deletion prompt")
  endif()
endfunction()

function(require_contents path expected)
  if(NOT EXISTS "${path}")
    message(FATAL_ERROR "Protected fixture missing: ${path}")
  endif()
  file(READ "${path}" actual)
  if(NOT "${actual}" STREQUAL "${expected}")
    message(FATAL_ERROR "Protected fixture changed: ${path}")
  endif()
endfunction()

set(theme theme_deletion-fixture)
# The output's only entry is a directory. Refuse even with an affirmative answer.
file(MAKE_DIRECTORY "${root}/subdirectories/${theme}/nested/empty")
file(WRITE "${root}/subdirectories/${theme}/nested/keep" "keep nested contents")
run_create(subdirectories yes 1 "Refusing to remove.*doesn't look like")
require_contents("${root}/subdirectories/${theme}/nested/keep" "keep nested contents")
if(NOT IS_DIRECTORY "${root}/subdirectories/${theme}/nested/empty")
  message(FATAL_ERROR "Protected empty subdirectory missing")
endif()

# A dangling symlink is also content, without following its target.
file(MAKE_DIRECTORY "${root}/symlink/${theme}")
file(CREATE_LINK "${root}/missing" "${root}/symlink/${theme}/link" SYMBOLIC)
run_create(symlink yes 1 "Refusing to remove.*doesn't look like")
if(NOT IS_SYMLINK "${root}/symlink/${theme}/link")
  message(FATAL_ERROR "Protected dangling symlink missing")
endif()

# A non-directory output fails the filesystem check before deletion.
file(MAKE_DIRECTORY "${root}/file")
file(WRITE "${root}/file/${theme}" "keep regular file")
run_create(file yes 1 "Refusing to remove.*could not be checked")
require_contents("${root}/file/${theme}" "keep regular file")

# Empty and nonexistent outputs remain valid.
file(MAKE_DIRECTORY "${root}/empty/${theme}" "${root}/nonexistent")
run_create(empty yes 0 "About to delete")
run_create(nonexistent yes 0 "Done, written")
require_contents("${root}/empty/${theme}/manifest.hl" "name = deletion-fixture\ncursors_directory = cursors\n")
require_contents("${root}/nonexistent/${theme}/manifest.hl" "name = deletion-fixture\ncursors_directory = cursors\n")

# Both supported theme markers retain the existing confirmation behavior.
foreach(extension hl toml)
  file(MAKE_DIRECTORY "${root}/${extension}/${theme}")
  file(WRITE "${root}/${extension}/${theme}/manifest.${extension}" "old theme")
  file(WRITE "${root}/${extension}/${theme}/keep" "keep until confirmed")
  run_create(${extension} no 1 "Abort")
  require_contents("${root}/${extension}/${theme}/keep" "keep until confirmed")
  run_create(${extension} eof 1 "Abort")
  require_contents("${root}/${extension}/${theme}/keep" "keep until confirmed")
  run_create(${extension} yes 0 "About to delete")
  if(EXISTS "${root}/${extension}/${theme}/keep")
    message(FATAL_ERROR "Confirmed fixture replacement did not remove old contents")
  endif()
endforeach()
