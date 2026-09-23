#include "gxana/xsection/Barlow.h"

#include <TMath.h>

#include <cmath>
#include <iostream>

namespace gxana {
namespace xsec {

// From AnalysisNote/systematics/PlotXSecBarlowChiSqNdf.C (same in all six PlotXSecBarlow*.C).
TGraphErrors* calc_barlow(TGraphErrors *nominal, TGraphErrors *variation)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma, barlow;
    for(int i = 0; i < nominal->GetN(); i++) {
        Delta = nominal->GetPointY(i) - variation->GetPointY(i);
        sigma = TMath::Sqrt( std::abs( std::pow(nominal->GetErrorY(i),2)
                                  - std::pow(variation->GetErrorY(i),2) ) );
        barlow = Delta / sigma;
        std::cout << "Nominal Point:  " <<  nominal->GetPointY(i) << " +/- " << nominal->GetErrorY(i) << "\n"
             << "Variation Point:  " <<  variation->GetPointY(i) << " +/- " << variation->GetErrorY(i)
             << " @ " << nominal->GetPointX(i) << std::endl;
        std::cout << "Results:  " << Delta << "  " << sigma << "  " << barlow
             << " @ " << nominal->GetPointX(i) << std::endl;
        if(sigma != 0.0)    graph->SetPointY( i, barlow );
        else                graph->SetPointY( i, 0.0 );
        graph->SetPointX( i, nominal->GetPointX(i) );
        graph->SetPointError( i, variation->GetErrorX(i), 0 );
    }

    return graph;
}

TGraphErrors* calculateStdDevGraph(const std::vector<TGraphErrors*>& graphs) {
    if (graphs.empty()) {
        std::cerr << "No graphs provided for standard deviation calculation!" << std::endl;
        return nullptr;
    }

    size_t numPoints = graphs[0]->GetN(); // Number of points in each graph
    size_t numGraphs = graphs.size();

    std::vector<double> meanX(numPoints, 0.0); // To store x-coordinates
    std::vector<double> stdDevY(numPoints, 0.0); // To store std deviation of y-values
    std::vector<double> stdDevX(numPoints, 0.0); // Assuming no x-errors for std-dev

    // Process each point
    for (size_t i = 0; i < numPoints; ++i) {
        std::vector<double> yValues;

        // Collect all y-values for the current x-value
        for (size_t j = 0; j < numGraphs; ++j) {
            double x, y;
            graphs[j]->GetPoint(i, x, y);

            // Use the x-value from the first graph (assume aligned x-points)
            if (j == 0) {
                meanX[i] = x;
            }

            yValues.push_back(y);
            std::cout << "XValue, YValues: " << x << " " << y << std::endl;
        }

        // Compute standard deviation for y-values
        double sumY = 0.0, sumYSq = 0.0;
        for (double y : yValues) {
            sumY += y;
            sumYSq += y * y;
        }


        double meanY = sumY / yValues.size();
        double varianceY = (sumYSq / yValues.size()) - (meanY * meanY);
        stdDevY[i] = (varianceY > 0) ? std::sqrt(varianceY) : 0.0;
        std::cout << "Mean: " << meanY << " Variance: " << varianceY << " StdDev: " << stdDevY[i] << std::endl;
    }

    // Create and populate the new TGraphErrors for standard deviation
    TGraphErrors* stdDevGraph = new TGraphErrors(numPoints);
    for (size_t i = 0; i < numPoints; ++i) {
        stdDevGraph->SetPoint(i, meanX[i], stdDevY[i]);
        stdDevGraph->SetPointError(i, stdDevX[i], 0); // x-errors are zero
    }

    stdDevGraph->SetMarkerStyle(21);
    stdDevGraph->SetMarkerColor(kRed);
    stdDevGraph->SetLineColor(kRed);

    return stdDevGraph;
}

} // namespace xsec
} // namespace gxana
