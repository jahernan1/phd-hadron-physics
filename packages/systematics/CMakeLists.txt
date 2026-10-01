add_library(GxanaSystematics SHARED src/PlotSpread.cxx src/TrackHists.cxx)
target_include_directories(GxanaSystematics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaSystematics PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Gpad ROOT::Graf ROOT::ROOTDataFrame ROOT::Tree GxanaCommon)
ROOT_GENERATE_DICTIONARY(G__GxanaSystematics
    gxana/systematics/PlotSpread.h
    MODULE GxanaSystematics
    LINKDEF LinkDef.h)

add_executable(gxana_syst_plot apps/gxana_syst_plot.cxx)
target_link_libraries(gxana_syst_plot PRIVATE GxanaSystematics)

add_executable(gxana_syst_track apps/gxana_syst_track.cxx)
target_link_libraries(gxana_syst_track PRIVATE GxanaSystematics)

add_executable(test_systematics tests/cpp/test_systematics.cxx)
target_link_libraries(test_systematics PRIVATE GxanaSystematics)
add_test(NAME systematics.unit COMMAND test_systematics $<TARGET_FILE:gxana_syst_plot> $<TARGET_FILE:gxana_syst_track>)

add_executable(test_read_equiv tests/cpp/test_read_equiv.cxx)
target_link_libraries(test_read_equiv PRIVATE GxanaSystematics)
add_test(NAME systematics.read_equiv COMMAND test_read_equiv)
