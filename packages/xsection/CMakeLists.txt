add_library(GxanaXsec SHARED src/Binning.cxx src/YieldFit.cxx src/Flux.cxx src/XSec.cxx src/Plotting.cxx)
target_include_directories(GxanaXsec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GxanaXsec PUBLIC ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist ROOT::Gpad ROOT::Graf
    ROOT::ROOTDataFrame ROOT::RooFit ROOT::RooFitCore GxanaCommon)
ROOT_GENERATE_DICTIONARY(G__GxanaXsec
    gxana/xsection/Binning.h gxana/xsection/YieldFit.h gxana/xsection/Flux.h gxana/xsection/XSec.h
    gxana/xsection/Plotting.h
    MODULE GxanaXsec
    LINKDEF LinkDef.h)

add_executable(gxana_xsec_bin apps/gxana_xsec_bin.cxx)
target_include_directories(gxana_xsec_bin PRIVATE apps)
target_link_libraries(gxana_xsec_bin PRIVATE GxanaXsec)

add_executable(gxana_xsec_tables apps/gxana_xsec_tables.cxx)
target_include_directories(gxana_xsec_tables PRIVATE apps)
target_link_libraries(gxana_xsec_tables PRIVATE GxanaXsec)

add_executable(test_xsection tests/cpp/test_xsection.cxx)
target_include_directories(test_xsection PRIVATE apps)
target_link_libraries(test_xsection PRIVATE GxanaXsec)
add_test(NAME xsection.unit COMMAND test_xsection)

# The library is usable from interpreted ROOT macros.
add_test(NAME xsection.cling
    COMMAND ${ROOT_EXE} -l -b -q
        -e "gInterpreter->AddIncludePath(\"${CMAKE_CURRENT_SOURCE_DIR}/include\");"
        -e "#include \"gxana/xsection/Binning.h\""
        -e "gSystem->Load(\"$<TARGET_FILE:GxanaXsec>\");"
        -e "gApplication->Terminate(gxana::xsec::BinName(6.4, 7.4, 0.1, 0.35) == \"emin_6.40_emax_7.40_tmin_0.10_tmax_0.35\" ? 0 : 1);")
