#ifndef GXANA_COMMON_STYLE_H
#define GXANA_COMMON_STYLE_H

namespace gxana {

// Thesis plot style (gStyle settings), from AnalysisNote/xsection/PlotFunctions.cpp.
void SetStyle();

// One gStyle value that a style may or may not set.
template <typename T>
struct StyleValue {
    bool set = false;
    T value{};
    StyleValue& operator=(T v)
    {
        set = true;
        value = v;
        return *this;
    }
};

// The gStyle members the package plot styles write, one field per TStyle setter call
// (axis "X"/"Y"/"T" as named). A field that is not set leaves gStyle's current value.
struct StyleParams {
    StyleValue<int> canvasColor, canvasBorderSize, canvasBorderMode, canvasDefH, canvasDefW;
    StyleValue<int> padColor, padBorderSize, padBorderMode;
    StyleValue<double> padBottomMargin, padTopMargin, padLeftMargin, padRightMargin;
    StyleValue<int> padGridX, padGridY, padTickX, padTickY;
    StyleValue<int> frameFillStyle, frameFillColor, frameLineColor, frameLineStyle, frameLineWidth,
        frameBorderSize, frameBorderMode;
    StyleValue<int> ndivisionsX;
    StyleValue<int> lineWidth, histLineWidth;
    StyleValue<int> legendFillColor, legendBorderSize, legendFont;
    StyleValue<double> legendTextSize;
    StyleValue<double> markerSize;
    StyleValue<int> markerStyle;
    StyleValue<double> labelSizeX, labelSizeY, labelOffsetX, labelOffsetY;
    StyleValue<int> labelFontX, labelFontY;
    StyleValue<int> titleBorderSize, titleFontT, titleFontX, titleFontY, titleAlign;
    StyleValue<double> titleX, titleSizeT, titleSizeX, titleSizeY, titleOffsetX, titleOffsetY;
    StyleValue<double> textSize;
    StyleValue<int> textFont;
    StyleValue<int> optStat;
    bool forceStyle = false; // gROOT->ForceStyle() after the setters
};

// One TStyle setter per set field, then gROOT->ForceStyle() if forceStyle.
void ApplyStyle(const StyleParams& params);

// The final gStyle writes of each existing style function (every field holds the last
// value the function wrote; repeated and no-op calls are not replayed).
StyleParams ThesisStyle();                                   // gxana::SetStyle
StyleParams FitStyle();                                      // gxana::xsec::SetFitStyle
StyleParams ComparisonStyle();                               // gxana::systematics::StyleFormat
StyleParams TrackStyle();                                    // style of the track-efficiency figures
StyleParams BarlowStyle(int canvasDefW, double titleOffsetY); // gxana::barlow::SetBarlowStyle
StyleParams GridTrailingTweak(); // the five gStyle writes after each 3x3 grid is drawn

} // namespace gxana

#endif
