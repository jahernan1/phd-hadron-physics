# packages/studies: analysis studies run by `gxana run studies` (see README.md). No ROOT dictionary:
# nothing loads GxanaStudies into the interpreter.
add_library(GxanaStudies SHARED src/CutScan.cxx src/DataMC.cxx)
target_include_directories(GxanaStudies PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaStudies PUBLIC GxanaFit GxanaPeriodHists ROOT::Core ROOT::Hist ROOT::Gpad ROOT::Graf
                      ROOT::RooFit ROOT::RooFitCore)

add_executable(gxana_study_cutscan apps/gxana_study_cutscan.cxx)
target_link_libraries(gxana_study_cutscan PRIVATE GxanaStudies)

add_executable(gxana_study_datamc apps/gxana_study_datamc.cxx)
target_link_libraries(gxana_study_datamc PRIVATE GxanaStudies)
