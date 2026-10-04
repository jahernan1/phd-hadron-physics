add_library(GxanaBarlow SHARED src/Barlow.cxx src/VariationTrees.cxx src/PlotBarlow.cxx)
target_include_directories(GxanaBarlow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaBarlow PUBLIC ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist ROOT::Gpad ROOT::Graf
    ROOT::ROOTDataFrame ROOT::RooFit ROOT::RooFitCore GxanaCommon GxanaXsec)
ROOT_GENERATE_DICTIONARY(G__GxanaBarlow
    gxana/barlow/Barlow.h
    gxana/barlow/VariationTrees.h
    MODULE GxanaBarlow
    LINKDEF LinkDef.h)

add_executable(gxana_barlow_trees apps/gxana_barlow_trees.cxx)
target_link_libraries(gxana_barlow_trees PRIVATE GxanaBarlow GxanaFit)

add_executable(gxana_barlow_plot apps/gxana_barlow_plot.cxx)
target_link_libraries(gxana_barlow_plot PRIVATE GxanaBarlow)

add_executable(test_barlow tests/cpp/test_barlow.cxx)
target_link_libraries(test_barlow PRIVATE GxanaBarlow)
add_test(NAME barlow.unit COMMAND test_barlow $<TARGET_FILE:gxana_barlow_trees> $<TARGET_FILE:gxana_barlow_plot>)

# The library is usable from interpreted ROOT macros.
add_test(NAME barlow.cling
    COMMAND ${ROOT_EXE} -l -b -q
        -e "gInterpreter->AddIncludePath(\"${CMAKE_CURRENT_SOURCE_DIR}/include\");"
        -e "#include \"gxana/barlow/Barlow.h\""
        -e "gSystem->Load(\"$<TARGET_FILE:GxanaBarlow>\");"
        -e "gApplication->Terminate(gxana::barlow::calculateStdDevGraph({}) == nullptr ? 0 : 1);")
