add_library(GxanaCommon SHARED src/Style.cxx src/Strings.cxx src/Paths.cxx src/GraphIO.cxx src/AcceptanceCorrect.cxx src/BinNames.cxx)
target_include_directories(GxanaCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaCommon PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Graf ROOT::Gpad)
ROOT_GENERATE_DICTIONARY(G__GxanaCommon
    gxana/common/Style.h gxana/common/Strings.h gxana/common/Paths.h gxana/common/GraphIO.h
    gxana/common/AcceptanceCorrect.h gxana/common/BinNames.h
    MODULE GxanaCommon
    LINKDEF LinkDef.h)

add_executable(test_common tests/cpp/test_common.cxx)
target_link_libraries(test_common PRIVATE GxanaCommon)
add_test(NAME common.unit COMMAND test_common)

add_executable(test_binnames tests/cpp/test_binnames.cxx)
target_link_libraries(test_binnames PRIVATE GxanaCommon)
add_test(NAME common.binnames COMMAND test_binnames)

add_executable(test_acceptance tests/cpp/test_acceptance.cxx)
target_link_libraries(test_acceptance PRIVATE GxanaCommon)
add_test(NAME common.acceptance COMMAND test_acceptance)

# The library is usable from interpreted ROOT macros.
add_test(NAME common.cling
    COMMAND ${ROOT_EXE} -l -b -q
        -e "gInterpreter->AddIncludePath(\"${CMAKE_CURRENT_SOURCE_DIR}/include\");"
        -e "#include \"gxana/common/Strings.h\""
        -e "gSystem->Load(\"$<TARGET_FILE:GxanaCommon>\");"
        -e "gApplication->Terminate(gxana::NumericCompare(\"a_emin_7.4\", \"a_emin_10.2\") ? 0 : 1);")

# Style harness: the plot-style functions of every package against the legacy bodies
# (tests/cpp/test_style.cxx). The other packages' targets are defined later; CMake
# resolves target names at generate time.
add_executable(test_style tests/cpp/test_style.cxx)
target_link_libraries(test_style PRIVATE GxanaCommon GxanaXsec GxanaSystematics GxanaBarlow)
add_test(NAME common.style COMMAND test_style)
