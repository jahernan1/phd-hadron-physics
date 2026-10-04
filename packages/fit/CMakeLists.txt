add_library(GxanaFit SHARED src/Model.cxx src/Fit.cxx src/Johnson.cxx)
target_include_directories(GxanaFit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaFit PUBLIC ROOT::Core ROOT::MathCore ROOT::Hist ROOT::Tree ROOT::RooFit ROOT::RooFitCore)
ROOT_GENERATE_DICTIONARY(G__GxanaFit
    gxana/fit/Model.h gxana/fit/Fit.h gxana/fit/Johnson.h
    MODULE GxanaFit
    LINKDEF LinkDef.h)

add_executable(test_fit tests/cpp/test_fit.cxx)
target_link_libraries(test_fit PRIVATE GxanaFit)
add_test(NAME fit.unit COMMAND test_fit)

# The library is usable from interpreted ROOT macros, and its Johnson moments equal the
# same statements run by the interpreter, bit for bit.
add_test(NAME fit.cling
    COMMAND ${ROOT_EXE} -l -b -q
        -e "gInterpreter->AddIncludePath(\"${CMAKE_CURRENT_SOURCE_DIR}/include\");"
        -e "gSystem->Load(\"$<TARGET_FILE:GxanaFit>\");"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/cpp/cling_fit.C")
