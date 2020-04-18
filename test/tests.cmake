# Use CTest for tests
enable_testing()
set(test_dir "${PROJECT_SOURCE_DIR}/test")
set(CTEST_OUTPUT_ON_FAILURE 1)

# Validate locales
file(GLOB locales data/*.mse-locale)
add_test(
  NAME "validate-locales"
  COMMAND perl "${test_dir}/locale/validate_locale.pl" "${PROJECT_SOURCE_DIR}/src" ${locales}
)

# Scripting language tests
# TODO

# Rendering tests
# TODO
