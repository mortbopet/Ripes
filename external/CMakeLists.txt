######################################################################
## External project subdirectories
######################################################################

include(FetchContent)

# Fancy tab bar
add_subdirectory(fancytabbar)

# VSRTL
set(VSRTL_BUILD_APP OFF)
set(VSRTL_BUILD_TESTS OFF)
add_subdirectory(VSRTL SYSTEM)


# Softfloat
# finding all source files
set(SOFTFLOAT_SPECIALIZATION RISCV)


set(SOFTFLOAT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/softfloat/source)

file(GLOB SOFTFLOAT_SOURCES_SPECIALIZATION
    ${SOFTFLOAT_PATH}/${SOFTFLOAT_SPECIALIZATION}/*.c
)

# filter out 128-bit and 80-bit float files
list(FILTER SOFTFLOAT_SOURCES_SPECIALIZATION EXCLUDE REGEX ".*(128)|(80).*")


set(SOFTFLOAT_SOURCES
    ${SOFTFLOAT_PATH}/softfloat_state.c

    # base files needed for f32 operations
    ${SOFTFLOAT_PATH}/f32_add.c
    ${SOFTFLOAT_PATH}/f32_div.c
    ${SOFTFLOAT_PATH}/f32_eq.c
    ${SOFTFLOAT_PATH}/f32_eq_signaling.c
    ${SOFTFLOAT_PATH}/f32_isSignalingNaN.c
    ${SOFTFLOAT_PATH}/f32_le.c
    ${SOFTFLOAT_PATH}/f32_le_quiet.c
    ${SOFTFLOAT_PATH}/f32_lt.c
    ${SOFTFLOAT_PATH}/f32_lt_quiet.c
    ${SOFTFLOAT_PATH}/f32_mulAdd.c
    ${SOFTFLOAT_PATH}/f32_mul.c
    ${SOFTFLOAT_PATH}/f32_rem.c
    ${SOFTFLOAT_PATH}/f32_roundToInt.c
    ${SOFTFLOAT_PATH}/f32_sqrt.c
    ${SOFTFLOAT_PATH}/f32_sub.c

    ${SOFTFLOAT_PATH}/f32_to_i32.c
    ${SOFTFLOAT_PATH}/f32_to_i32_r_minMag.c
    ${SOFTFLOAT_PATH}/f32_to_i64.c
    ${SOFTFLOAT_PATH}/f32_to_i64_r_minMag.c
    
    ${SOFTFLOAT_PATH}/f32_to_ui32.c
    ${SOFTFLOAT_PATH}/f32_to_ui32_r_minMag.c
    ${SOFTFLOAT_PATH}/f32_to_ui64.c
    ${SOFTFLOAT_PATH}/f32_to_ui64_r_minMag.c
    
    ${SOFTFLOAT_PATH}/i32_to_f32.c
    ${SOFTFLOAT_PATH}/i64_to_f32.c
    
    ${SOFTFLOAT_PATH}/ui32_to_f32.c
    ${SOFTFLOAT_PATH}/ui64_to_f32.c

    # dependencies of base files
    ${SOFTFLOAT_PATH}/s_subMagsF32.c
    ${SOFTFLOAT_PATH}/s_addMagsF32.c
    ${SOFTFLOAT_PATH}/s_mulAddF32.c
    
    ${SOFTFLOAT_PATH}/s_shiftRightJamM.c
    ${SOFTFLOAT_PATH}/s_shiftRightJam32.c
    ${SOFTFLOAT_PATH}/s_shiftRightJam64.c
    ${SOFTFLOAT_PATH}/s_shortShiftRightJam64.c
    ${SOFTFLOAT_PATH}/s_shortShiftRightJamM.c

    ${SOFTFLOAT_PATH}/s_roundMToI64.c
    ${SOFTFLOAT_PATH}/s_roundMToUI64.c
    ${SOFTFLOAT_PATH}/s_roundToI32.c
    ${SOFTFLOAT_PATH}/s_roundToUI32.c
    ${SOFTFLOAT_PATH}/s_roundToI64.c
    ${SOFTFLOAT_PATH}/s_roundToUI64.c

    ${SOFTFLOAT_PATH}/s_roundPackToF32.c
    ${SOFTFLOAT_PATH}/s_normRoundPackToF32.c
    ${SOFTFLOAT_PATH}/s_normSubnormalF32Sig.c

    ${SOFTFLOAT_PATH}/s_approxRecip_1Ks.c
    ${SOFTFLOAT_PATH}/s_approxRecip32_1.c
    ${SOFTFLOAT_PATH}/s_approxRecipSqrt_1Ks.c
    ${SOFTFLOAT_PATH}/s_approxRecipSqrt32_1.c
)

# creating the platform.h file for softfloat
include(TestBigEndian)
include(CheckCSourceCompiles)
test_big_endian(IS_BIG_ENDIAN)

if (IS_BIG_ENDIAN)
    set(SOFTFLOAT_LITTLEENDIAN 0)
else()
    set(SOFTFLOAT_LITTLEENDIAN 1)
endif()

# 64-bit host detection -------------------------------------------------------
# since high-performance 64-bit operations are not critical for Ripes' usage of
# softfloat, we disable them to maximize compatibility with 32-bit systems and codebase.
# Else use this code to enable fast 64-bit operations on 64-bit systems
# Note: However then some functions must be replaced in code and here in CMakeLists.txt
# if (CMAKE_SIZEOF_VOID_P EQUAL 8)
#     set(SOFTFLOAT_FAST_INT64 1)
# endif()

# intrinsic 128-bit type detection --------------------------------------------
# we disable this to maximize compatibility with 32-bit systems and codebase.
# Else use this code to enable if the compiler supports it
# Note: However then some functions must be replaced in code and here in CMakeLists.txt
# check_c_source_compiles("
# #include <stdint.h>
# int main(void) {
#     __int128 x = 0;
#     return (int)(x);
# }
# " HAVE_INTRINSIC_INT128)
# if (HAVE_INTRINSIC_INT128)
#     set(SOFTFLOAT_INTRINSIC_INT128 1)
# endif()

# count leading zeros builtin detection ---------------------------------------
check_c_source_compiles("
int main(void) {
    unsigned int x = 1;
    return __builtin_clz(x) + __builtin_clzll(x);
}
" HAVE_BUILTIN_CLZ)
if (HAVE_BUILTIN_CLZ)
    set(SOFTFLOAT_BUILTIN_CLZ 1)
    set(SOFTFLOAT_SOURCES_BUILTIN_CLZ "")
else()
    # add source files for builtin clz implementations
    set(SOFTFLOAT_SOURCES_BUILTIN_CLZ
        ${SOFTFLOAT_PATH}/s_countLeadingZeros8.c
        ${SOFTFLOAT_PATH}/s_countLeadingZeros16.c
        ${SOFTFLOAT_PATH}/s_countLeadingZeros32.c
        ${SOFTFLOAT_PATH}/s_countLeadingZeros64.c
    )
endif()

message("Softfloat configuration: LITTLEENDIAN=${SOFTFLOAT_LITTLEENDIAN}, FAST_INT64=${SOFTFLOAT_FAST_INT64}, BUILTIN_CLZ=${SOFTFLOAT_BUILTIN_CLZ}, INTRINSIC_INT128=${SOFTFLOAT_INTRINSIC_INT128}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/platform.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/softfloat/platform.h
    @ONLY
)

add_library(softfloat
    STATIC 
        ${SOFTFLOAT_SOURCES}
        ${SOFTFLOAT_SOURCES_SPECIALIZATION}
        ${SOFTFLOAT_SOURCES_BUILTIN_CLZ}
)

target_include_directories(softfloat
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/softfloat/source/${SOFTFLOAT_SPECIALIZATION} # specialization headers
        ${CMAKE_CURRENT_SOURCE_DIR}/softfloat/source/include                     # general softfloat headers
        ${CMAKE_CURRENT_BINARY_DIR}/softfloat                                    # code-gen platform.h
)

target_compile_definitions(softfloat PRIVATE
    SOFTFLOAT_ROUND_ODD         # include round-to-odd mode
    SOFTFLOAT_FAST_DIV32TO16    # enable fast 32-to-16 division
    SOFTFLOAT_FAST_DIV64TO32    # enable fast 64-to-32 division
    INLINE_LEVEL=5              # set inline level to maximum (for performance)
)

target_compile_features(softfloat PUBLIC c_std_11)


# libelfin
FetchContent_Declare(
    libelfin
    SYSTEM
    GIT_REPOSITORY https://github.com/mortbopet/libelfin.git
    GIT_TAG        c06ecb09c0d2145091119d15b911c54d2d378ea7
)
FetchContent_MakeAvailable(libelfin)

# Make autoMOC and autoUIC happy about the generated libelfin files
set(libelfin_src "${CMAKE_CURRENT_BINARY_DIR}/libelfin")
set_property(SOURCE "${libelfin_src}/elf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)
set_property(SOURCE "${libelfin_src}/dwarf_to_string.cc" PROPERTY SKIP_AUTOGEN ON)

# ELFIO
FetchContent_Declare(
    ELFIO
    SYSTEM
    GIT_REPOSITORY https://github.com/serge1/ELFIO.git
    GIT_TAG 79fcd11595d0793dec593e9072e36c7923c57c1a
)
FetchContent_MakeAvailable(ELFIO)
