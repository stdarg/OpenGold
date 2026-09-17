# Require both a successful process and the script's completion marker. Godot
# script errors do not consistently produce a nonzero process exit status.
execute_process(
    COMMAND "${GODOT}" --headless --path "${PROJECT}" --script "${SCRIPT}" -- ${ARGS}
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 30)
message("${output}${errors}")
if(NOT result STREQUAL "0" OR NOT output MATCHES "${EXPECTED}" OR
        "${output}${errors}" MATCHES "SCRIPT ERROR|Assertion failed")
    message(FATAL_ERROR "Godot check failed (${result}): ${SCRIPT}")
endif()
