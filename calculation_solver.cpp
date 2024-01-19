#include "calculation_solver.h"
#include "LeastSquareSolver.cpp"

calculation_solver::calculation_solver()
{
   ::loadAllLists();
}

void calculation_solver::setElavationAngle(double angle)
{
    lss::setElevationAngle(angle);
}

void calculation_solver::solve_dark_pixels(const QString& satellite_name,
                                           const QVector<double>& dark_pixels)
{
    qDebug()<<"----------SOLVE DARK PIXEL------------------------";
    if(dark_pixels.size()<4)return;
    auto result = lss::optimize(satellite_name, dark_pixels);
    emit darkpixels_calculation_finished(result);
}
