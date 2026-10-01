add_library(GxanaSystematics SHARED src/PlotSpread.cxx)
target_include_directories(GxanaSystematics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaSystematics PUBLIC ROOT::Core ROOT::RIO ROOT::Hist ROOT::Gpad ROOT::Graf GxanaCommon)
ROOT_GENERATE_DICTIONARY(G__GxanaSystematics
    gxana/systematics/PlotSpread.h
    MODULE GxanaSystematics
    LINKDEF LinkDef.h)

add_executable(gxana_syst_plot apps/gxana_syst_plot.cxx)
target_include_directories(gxana_syst_plot PRIVATE ${CMAKE_SOURCE_DIR}/packages/xsection/apps)
target_link_libraries(gxana_syst_plot PRIVATE GxanaSystematics)

add_executable(test_systematics tests/cpp/test_systematics.cxx)
target_link_libraries(test_systematics PRIVATE GxanaSystematics)
add_test(NAME systematics.unit COMMAND test_systematics $<TARGET_FILE:gxana_syst_plot>)
