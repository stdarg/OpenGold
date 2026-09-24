# Require both a successful process and the script's completion marker. Godot
# script errors do not consistently produce a nonzero process exit status.
set(display_arguments --headless)
if(GRAPHICAL)
    set(display_arguments)
endif()
if(NOT DEFINED TEST_TIMEOUT)
    set(TEST_TIMEOUT 30)
endif()
execute_process(
    COMMAND "${GODOT}" ${display_arguments} --path "${PROJECT}" --script "${SCRIPT}" -- ${ARGS}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT "${TEST_TIMEOUT}")
message("${output}${errors}")
if(NOT result STREQUAL "0" OR NOT output MATCHES "${EXPECTED}" OR
        "${output}${errors}" MATCHES "SCRIPT ERROR|Assertion failed")
    message(FATAL_ERROR "Godot check failed (${result}): ${SCRIPT}")
endif()
