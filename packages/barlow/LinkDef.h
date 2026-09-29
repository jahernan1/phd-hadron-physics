#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ namespace gxana::barlow;
#pragma link C++ function gxana::barlow::calc_barlow;
#pragma link C++ function gxana::barlow::calculateStdDevGraph;
#pragma link C++ struct gxana::barlow::Variation+;
#pragma link C++ struct gxana::barlow::VariationTreesSpec+;
#pragma link C++ function gxana::barlow::WriteVariationTrees;
#pragma link C++ function gxana::barlow::SplitAssign;
#pragma link C++ struct gxana::barlow::CheckSpec+;
#pragma link C++ function gxana::barlow::CheckVariationYields;
#pragma link C++ struct gxana::barlow::BarlowPlotStyle+;
#pragma link C++ struct gxana::barlow::BarlowPlotSpec+;
#pragma link C++ function gxana::barlow::SetBarlowStyle;
#pragma link C++ function gxana::barlow::PlotBarlow;
#endif
