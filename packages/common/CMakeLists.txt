add_library(GxanaCommon SHARED src/Style.cxx src/Strings.cxx src/Paths.cxx src/GraphIO.cxx)
target_include_directories(GxanaCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaCommon PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Graf ROOT::Gpad)
ROOT_GENERATE_DICTIONARY(G__GxanaCommon
    gxana/common/Style.h gxana/common/Strings.h gxana/common/Paths.h gxana/common/GraphIO.h
    MODULE GxanaCommon
    LINKDEF LinkDef.h)

add_executable(test_common tests/cpp/test_common.cxx)
target_link_libraries(test_common PRIVATE GxanaCommon)
add_test(NAME common.unit COMMAND test_common)

# The library is usable from interpreted ROOT macros.
add_test(NAME common.cling
    COMMAND ${ROOT_EXE} -l -b -q
        -e "gInterpreter->AddIncludePath(\"${CMAKE_CURRENT_SOURCE_DIR}/include\");"
        -e "#include \"gxana/common/Strings.h\""
        -e "gSystem->Load(\"$<TARGET_FILE:GxanaCommon>\");"
        -e "gApplication->Terminate(gxana::NumericCompare(\"a_emin_7.4\", \"a_emin_10.2\") ? 0 : 1);")
