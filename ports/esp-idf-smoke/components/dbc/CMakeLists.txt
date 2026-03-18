idf_component_register(
    SRCS
        "../../../../src/base64.c"
        "../../../../src/cJSON.c"
        "../../../../src/deathbycaptcha.c"
    INCLUDE_DIRS
        "../../../../src"
)

# HTTPS transport is libcurl-based in this project; disable it for ESP-IDF smoke builds.
target_compile_definitions(${COMPONENT_LIB} PRIVATE DBC_HAS_CURL=0)
