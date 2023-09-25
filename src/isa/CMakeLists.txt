create_ripes_lib(isa LINK_TO_RIPES_LIB)

target_sources(isa_lib
  PRIVATE
    mipsisainfo_common.h mipsisainfo_common.cpp
    mips32isainfo.h
)
