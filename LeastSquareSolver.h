#ifndef LEAST_SQUARE_SOLVER
#define LEAST_SQUARE_SOLVER

#include <QJsonArray>
#include <QJsonObject>
#include <vector>
using std::vector;

vector<vector<double>> S_lambda_lists(4);// TODO adopt this container for 5 channels
vector<double> lambda_waves;
vector<double> T_H2O_list;
vector<double> lambda_list;
vector<double> T_O2_list;
vector<double> T_O3_list;
vector<double> B_lambda_teta_list;
vector<double> divider_list;
vector<double> tau_m;
QJsonObject satellites;
QJsonArray sdb;
QString satellite_name_key = "_bka";
QStringList satellites_list;
vector <double>  dark_pixels = {39.535587, 25.645323, 11.881793, 4.310712};

#endif // ENVIMODULE_H
