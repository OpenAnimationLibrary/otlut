file(MAKE_DIRECTORY "${TEST_DIR}")

set(SOURCE_PPM "${TEST_DIR}/source.ppm")
set(TARGET_PPM "${TEST_DIR}/target.ppm")
set(OUTPUT_CUBE "${TEST_DIR}/look.cube")

file(WRITE "${SOURCE_PPM}" "P3\n2 2\n255\n0 0 0\n255 0 0\n0 255 0\n255 255 255\n")
file(WRITE "${TARGET_PPM}" "P3\n2 2\n255\n20 10 0\n255 20 0\n20 230 10\n255 245 230\n")

execute_process(
  COMMAND "${OTLUT_EXE}"
    --source "${SOURCE_PPM}"
    --target "${TARGET_PPM}"
    --output "${OUTPUT_CUBE}"
    --size 3
  RESULT_VARIABLE RESULT
  OUTPUT_VARIABLE STDOUT_TEXT
  ERROR_VARIABLE STDERR_TEXT
)

if(NOT RESULT EQUAL 0)
  message(FATAL_ERROR "otlut CLI pair test failed: ${STDERR_TEXT}")
endif()

if(NOT EXISTS "${OUTPUT_CUBE}")
  message(FATAL_ERROR "otlut did not create the expected .cube file")
endif()

file(READ "${OUTPUT_CUBE}" CUBE_TEXT)

if(NOT CUBE_TEXT MATCHES "LUT_3D_SIZE 3")
  message(FATAL_ERROR "Generated LUT has the wrong grid header")
endif()

string(REGEX MATCHALL "\n[-0-9]+\\.[0-9]+ [-0-9]+\\.[0-9]+ [-0-9]+\\.[0-9]+" DATA_LINES "${CUBE_TEXT}")
list(LENGTH DATA_LINES DATA_COUNT)

if(NOT DATA_COUNT EQUAL 27)
  message(FATAL_ERROR "Generated 3x3x3 LUT should contain 27 data entries; got ${DATA_COUNT}")
endif()

if(NOT STDOUT_TEXT MATCHES "Interpolated/fill LUT cells")
  message(FATAL_ERROR "CLI output did not report interpolation/fill coverage")
endif()
