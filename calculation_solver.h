#ifndef CALCULATION_SOLVER_H
#define CALCULATION_SOLVER_H

#include <QObject>
#include <QVector>
#include "common_types.h"

class calculation_solver: public QObject {
  Q_OBJECT
 public:
  calculation_solver();
  QStringList getSatellitesList();
  void updateCurrentSatellite(QString sat_name);

 private slots:
  void setElavationAngle(double angle);
  void solve_dark_pixels(const QString& satellite_name,
                         const QVector<double>& dark_pixels);
 signals:
  void darkpixels_calculation_finished(result_values);
};

#endif // CALCULATION_SOLVER_H
