cmake_minimum_required(VERSION 3.20)

foreach(required_variable IN ITEMS RUNNER FIXTURES OUTPUT_DIR)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable: ${required_variable}")
    endif()
endforeach()

if(NOT EXISTS "${RUNNER}")
    message(FATAL_ERROR "Metadata runner executable does not exist: ${RUNNER}")
endif()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

function(run_runner result_out stdout_out stderr_out)
    execute_process(
        COMMAND "${RUNNER}" ${ARGN}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr)

    set(${result_out} "${result}" PARENT_SCOPE)
    set(${stdout_out} "${stdout}" PARENT_SCOPE)
    set(${stderr_out} "${stderr}" PARENT_SCOPE)
endfunction()

function(assert_equal actual expected context)
    if(NOT "${actual}" STREQUAL "${expected}")
        message(FATAL_ERROR
            "${context}: expected '${expected}', got '${actual}'")
    endif()
endfunction()

function(assert_nonzero actual context)
    if("${actual}" STREQUAL "0")
        message(FATAL_ERROR "${context}: expected a non-zero exit code")
    endif()
endfunction()

function(assert_contains text expected context)
    string(FIND "${text}" "${expected}" match_offset)
    if(match_offset EQUAL -1)
        message(FATAL_ERROR
            "${context}: output does not contain '${expected}'\nOutput:\n${text}")
    endif()
endfunction()

function(load_valid_json path json_out)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Expected JSON output file was not created: ${path}")
    endif()

    file(READ "${path}" json)
    string(JSON root_type ERROR_VARIABLE json_error TYPE "${json}")
    if(NOT "${json_error}" STREQUAL "NOTFOUND")
        message(FATAL_ERROR "Invalid JSON in ${path}: ${json_error}")
    endif()
    assert_equal("${root_type}" "OBJECT" "JSON root type")

    foreach(section IN ITEMS processing decision storage)
        string(JSON section_type ERROR_VARIABLE section_error TYPE "${json}" "${section}")
        if(NOT "${section_error}" STREQUAL "NOTFOUND")
            message(FATAL_ERROR "Missing JSON section '${section}': ${section_error}")
        endif()
        assert_equal("${section_type}" "OBJECT" "JSON section '${section}'")
    endforeach()

    string(JSON alarms_type ERROR_VARIABLE alarms_error TYPE "${json}" alarms)
    if(NOT "${alarms_error}" STREQUAL "NOTFOUND")
        message(FATAL_ERROR "Missing JSON section 'alarms': ${alarms_error}")
    endif()
    assert_equal("${alarms_type}" "ARRAY" "JSON section 'alarms'")

    set(${json_out} "${json}" PARENT_SCOPE)
endfunction()

function(json_get json_out json)
    string(JSON value ERROR_VARIABLE json_error GET "${json}" ${ARGN})
    if(NOT "${json_error}" STREQUAL "NOTFOUND")
        message(FATAL_ERROR "Could not read JSON path '${ARGN}': ${json_error}")
    endif()
    set(${json_out} "${value}" PARENT_SCOPE)
endfunction()

function(json_array_length length_out json)
    string(JSON length ERROR_VARIABLE json_error LENGTH "${json}" ${ARGN})
    if(NOT "${json_error}" STREQUAL "NOTFOUND")
        message(FATAL_ERROR "Could not read JSON array '${ARGN}': ${json_error}")
    endif()
    set(${length_out} "${length}" PARENT_SCOPE)
endfunction()

set(inside_fixture "${FIXTURES}/person_inside_dangerous_zone.xml")
set(outside_fixture "${FIXTURES}/person_outside_dangerous_zone.xml")
set(empty_fixture "${FIXTURES}/empty_no_person_metadata.xml")

set(inside_json_path "${OUTPUT_DIR}/inside-result.json")
file(REMOVE "${inside_json_path}")
run_runner(
    inside_result inside_stdout inside_stderr
    --camera-id CAM-001
    --metadata-file "${inside_fixture}"
    --output-json "${inside_json_path}")
assert_equal("${inside_result}" "0" "Inside-zone fixture exit code")
assert_contains("${inside_stdout}" "Detections processed: 1" "Inside-zone summary")
assert_contains("${inside_stdout}" "Tracks upserted: 1" "Inside-zone summary")
assert_contains("${inside_stdout}" "Events created: 1" "Inside-zone summary")
assert_contains("${inside_stdout}" "Decisions evaluated: 1" "Inside-zone summary")
assert_contains("${inside_stdout}" "Violations: 1" "Inside-zone summary")
assert_contains("${inside_stdout}" "Alarms created: 1" "Inside-zone summary")
load_valid_json("${inside_json_path}" inside_json)
json_get(value "${inside_json}" processing detectionsProcessed)
assert_equal("${value}" "1" "Inside-zone detectionsProcessed")
json_get(value "${inside_json}" processing tracksUpserted)
assert_equal("${value}" "1" "Inside-zone tracksUpserted")
json_get(value "${inside_json}" processing eventsCreated)
assert_equal("${value}" "1" "Inside-zone eventsCreated")
json_get(value "${inside_json}" decision decisionsEvaluated)
assert_equal("${value}" "1" "Inside-zone decisionsEvaluated")
json_get(value "${inside_json}" decision violations)
assert_equal("${value}" "1" "Inside-zone violations")
json_get(value "${inside_json}" decision alarmsCreated)
assert_equal("${value}" "1" "Inside-zone alarmsCreated")
json_array_length(alarms_length "${inside_json}" alarms)
assert_equal("${alarms_length}" "1" "Inside-zone alarms length")
json_get(alarm_reason "${inside_json}" alarms 0 reason)
if("${alarm_reason}" STREQUAL "")
    message(FATAL_ERROR "Inside-zone alarm reason must not be empty")
endif()

set(outside_json_path "${OUTPUT_DIR}/outside-result.json")
file(REMOVE "${outside_json_path}")
run_runner(
    outside_result outside_stdout outside_stderr
    --camera-id CAM-001
    --metadata-file "${outside_fixture}"
    --output-json "${outside_json_path}")
assert_equal("${outside_result}" "0" "Outside-zone fixture exit code")
assert_contains("${outside_stdout}" "Detections processed: 1" "Outside-zone summary")
assert_contains("${outside_stdout}" "Tracks upserted: 1" "Outside-zone summary")
assert_contains("${outside_stdout}" "Events created: 1" "Outside-zone summary")
assert_contains("${outside_stdout}" "Violations: 0" "Outside-zone summary")
assert_contains("${outside_stdout}" "Alarms created: 0" "Outside-zone summary")
load_valid_json("${outside_json_path}" outside_json)
json_get(value "${outside_json}" processing detectionsProcessed)
assert_equal("${value}" "1" "Outside-zone detectionsProcessed")
json_get(value "${outside_json}" processing tracksUpserted)
assert_equal("${value}" "1" "Outside-zone tracksUpserted")
json_get(value "${outside_json}" processing eventsCreated)
assert_equal("${value}" "1" "Outside-zone eventsCreated")
json_get(value "${outside_json}" decision violations)
assert_equal("${value}" "0" "Outside-zone violations")
json_get(value "${outside_json}" decision alarmsCreated)
assert_equal("${value}" "0" "Outside-zone alarmsCreated")
json_array_length(alarms_length "${outside_json}" alarms)
assert_equal("${alarms_length}" "0" "Outside-zone alarms length")

set(empty_json_path "${OUTPUT_DIR}/empty-result.json")
file(REMOVE "${empty_json_path}")
run_runner(
    empty_result empty_stdout empty_stderr
    --camera-id CAM-001
    --metadata-file "${empty_fixture}"
    --output-json "${empty_json_path}")
assert_equal("${empty_result}" "0" "Empty fixture exit code")
assert_contains("${empty_stdout}" "Detections processed: 0" "Empty-fixture summary")
assert_contains("${empty_stdout}" "Decisions evaluated: 0" "Empty-fixture summary")
assert_contains("${empty_stdout}" "Violations: 0" "Empty-fixture summary")
assert_contains("${empty_stdout}" "Alarms created: 0" "Empty-fixture summary")
load_valid_json("${empty_json_path}" empty_json)
json_get(value "${empty_json}" processing detectionsProcessed)
assert_equal("${value}" "0" "Empty-fixture detectionsProcessed")
json_get(value "${empty_json}" decision decisionsEvaluated)
assert_equal("${value}" "0" "Empty-fixture decisionsEvaluated")
json_get(value "${empty_json}" decision violations)
assert_equal("${value}" "0" "Empty-fixture violations")
json_get(value "${empty_json}" decision alarmsCreated)
assert_equal("${value}" "0" "Empty-fixture alarmsCreated")
json_array_length(alarms_length "${empty_json}" alarms)
assert_equal("${alarms_length}" "0" "Empty-fixture alarms length")

run_runner(
    missing_camera_result missing_camera_stdout missing_camera_stderr
    --metadata-file "${inside_fixture}")
assert_nonzero("${missing_camera_result}" "Missing --camera-id")
assert_contains("${missing_camera_stdout}" "Usage:" "Missing --camera-id")

run_runner(
    missing_metadata_result missing_metadata_stdout missing_metadata_stderr
    --camera-id CAM-001)
assert_nonzero("${missing_metadata_result}" "Missing --metadata-file")
assert_contains("${missing_metadata_stdout}" "Usage:" "Missing --metadata-file")

run_runner(
    invalid_path_result invalid_path_stdout invalid_path_stderr
    --camera-id CAM-001
    --metadata-file "${FIXTURES}/does-not-exist.xml")
assert_nonzero("${invalid_path_result}" "Invalid metadata file path")
assert_contains(
    "${invalid_path_stderr}"
    "Could not open metadata file"
    "Invalid metadata file path")

message(STATUS "All MetadataFileRunner fixture and CLI integration checks passed")
