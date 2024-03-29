#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include "mpfit.h"
#include "common_types.h"
#include "math.h"
#include "LeastSquareSolver.h"
#include "DBJson.h"
#include <vector>
#include <array>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtMath>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


namespace {
using std::vector;
using std::string;

static bool is_first_run = true;
constexpr uint16_t NUMBER_WAVELENGTH = 601;
constexpr double TAU_M_0 = 0.101;
constexpr double LAMBDA_0 = 0.55;
constexpr double P = 1.25;
constexpr double Q = 1;
constexpr double TAU_E = 0.04;
constexpr double pi = 3.14159265358979323846;

struct vars_struct {
  double* tau_0_a_err;
  double* beta_err;
  double* g_err;
  double* albedo_err;
};

/*struct result_values{
    double tau_0_a; // фикс при вычеслении lookup table
    double beta;    // фикс при вычеслении lookup table
    double g;       // фикс при вычеслении lookup table
    double albedo;  // искомое для матрицы
};*/

double mu_0 = qCos(qDegreesToRadians(41.3));
static result_values rv;
void calculDividerList(vector<vector<double> >& responses);
inline vector <double> compute_tau_m(const vector<double>& list);

void loadAllLists() {
  if (!is_first_run)
    return;
  is_first_run = false;
  db_json::getJsonArrayFromFile("sdb.json", sdb);
  db_json::getJsonObjectFromFile("satellites.json", satellites);
  qDebug() << "sdb size: " << sdb.size();
  qDebug() << "satellites: " << satellites["4"].toArray().size();

  auto sats = satellites.value("4").toArray();
  for (int i = 0; i < sats.size(); ++i) {
    satellites_list.append(sats[i].toString());
  }
  satellite_name_key = satellites_list[0];
  qDebug()<<"initial satellite:"<<satellite_name_key;
  for (int i = 0; i < sdb.size(); ++i) {
    T_H2O_list.push_back(sdb[i].toObject()["h2o"].toDouble());
    T_O2_list.push_back(sdb[i].toObject()["o2"].toDouble());
    T_O3_list.push_back(sdb[i].toObject()["o3"].toDouble());
    lambda_list.push_back(sdb[i].toObject()["wavelength"].toDouble());
    B_lambda_teta_list.push_back(sdb[i].toObject()["sun"].toDouble());
    auto response = sdb[i].toObject()[satellite_name_key].toArray();
    for (int j = 0; j < response.size(); ++j) {
      S_lambda_lists[j].push_back(response[j].toDouble());
    }
  }
  qDebug()<<"Responses: "<<S_lambda_lists[0].size();
  calculDividerList(S_lambda_lists);
  tau_m = compute_tau_m(lambda_list);

  //This is a draft for editing sdb.json
  /*auto sats5 = satellites.value("5").toArray();
  for (int i = 0; i < sats5.size(); ++i) {
    satellites_list.append(sats5[i].toString());
  }
  QJsonArray sats_array;
  for(int j=0;j<sdb.size();++j){
  QJsonObject object;
  for(int i=0;i<satellites_list.size();++i){
      auto sat_array = sdb[j].toObject()[satellites_list[i]].toArray();
      QStringRef newKey(&satellites_list[i],1,satellites_list[i].size()-1);
      object[newKey] = sat_array;
  }
  object["h2o"] = sdb[j].toObject()["h2o"];
  object["o2"] = sdb[j].toObject()["o2"];
  object["o3"] = sdb[j].toObject()["o3"];
  object["sun"] = sdb[j].toObject()["sun"];
  object["wavelength"] = sdb[j].toObject()["wavelength"];
  sats_array.append(object);
  }
  db_json::saveJsonArrayToFile("sdb.json",sats_array,QJsonDocument::Compact);*/
}

void calculDividerList(vector<vector<double> >& responses) {
  double sum1 = 0;
  double sum2 = 0;
  double sum3 = 0;
  double sum4 = 0;
  size_t sizeList = responses[0].size();
  Q_ASSERT(sizeList == NUMBER_WAVELENGTH);
  for (uintmax_t i = 0; i < responses.size(); ++i) {
    for (size_t j = 0; j < sizeList; ++j) {
      if (i == 0) {
        sum1 += responses[i][j];
      }
      if (i == 1) {
        sum2 += responses[i][j];
      }
      if (i == 2) {
        sum3 += responses[i][j];
      }
      if (i == 3) {
        sum4 += responses[i][j];
      }
    }
  }
  divider_list = {sum1, sum2, sum3, sum4};
  qDebug() << "DIVIDED: " << sum1 << sum2 << sum3 << sum4;
}

inline vector <double> compute_tau_m(const vector<double>& list) {
  std::vector <double> result;
  for (uintmax_t i = 0; i < list.size(); ++i) {
    auto lambda_0_lambda = LAMBDA_0 / list[i];
    result.push_back(TAU_M_0 * pow(lambda_0_lambda, 4));
  }
  return result;
}

inline double compute_x_m(const double& mu_0) {
  return  3 * (1 + pow(mu_0, 2)) / 4;
}

inline double compute_x_a(const double& mu_0, const double& g) {
  return (1 - pow(g, 2)) / pow((1 + pow(g, 2) + 2 * g * mu_0), 1.5);
}

inline vector <double> compute_tau_a(const double& tau_0_a,
                                     const double& beta,
                                     const vector <double>& list) {
  std::vector <double> result;
  for (uintmax_t i = 0; i < list.size(); ++i) {
    double lambda_0_lambda = LAMBDA_0 / list[i];
    result.push_back(tau_0_a * pow(lambda_0_lambda, beta));
  }
  return result;
}


inline vector <double> compute_tau_lambda(const double& tau_e,
                                          const double& tau_0_a,
                                          const double& beta,
                                          const vector <double>& tau_m,
                                          const vector <double>& list) {
  std::vector <double> result;
  std::vector <double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (uintmax_t i = 0; i < list.size(); ++i) {
    result.push_back(tau_m[i] + tau_a[i] + tau_e);
  }
  return result;
}

inline vector <double> compute_omega(const double& tau_e,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {
  std::vector<double> result;
  std::vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (uintmax_t i = 0; i < list.size(); ++i) {
    result.push_back((tau_m[i] + tau_a[i]) / (tau_m[i] + tau_a[i] + tau_e));
  }
  return result;
}

inline vector <double> compute_g(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> result;
  vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  for (uintmax_t i = 0; i < list.size(); ++i) {
    result.push_back(g * tau_a[i] / (tau_m[i] + tau_a[i]));
  }
  return result;
}

inline vector <double> compute_x(const double& mu_0,
                                 const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> result;
  vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
  auto x_m = compute_x_m(mu_0);
  auto x_a = compute_x_a(mu_0, g);
  for (uintmax_t i = 0; i < list.size(); ++i) {
    result.push_back((x_m * tau_m[i] + x_a * tau_a[i]) / (tau_m[i] + tau_a[i]));
  }
  return result;
}

inline vector <double> compute_B_atm(const double& mu_0,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const double& g,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {
  vector<double> result;
  vector<double> tau_lambda = compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> x = compute_x(mu_0, g, tau_0_a, beta, tau_m, list);

  for (uintmax_t i = 0; i < list.size(); ++i) {
    auto b_atm = omega_lambda[i] * x[i] / (4.0 * (1.0 + mu_0)) * (1.0 - exp(-tau_lambda[i] * (
                                                                                1.0 / mu_0 + 1.0))) * (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i], P));
    result.push_back(b_atm);

  }
  return result;
}

inline double compute_B1(const vector <double>& T_O2_list,
                         const vector <double>& T_O3_list,
                         const vector <double>& T_H2O_list,
                         const vector <double>& S_lambda_list,
                         const vector <double>& B_lambda_teta_list,
                         const double& mu_0,
                         const double& tau_0_a,
                         const double& beta,
                         const double& g,
                         const vector <double>& tau_m,
                         const vector <double>& list) {
  double B1 = 0.0;
  auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
  for (uintmax_t i = 0; i < list.size(); ++i) {
    auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
    auto S_lambda = S_lambda_list[i];
    auto B_sun = B_lambda_teta_list[i];
    auto b = B_atm[i];
    B1 += b * T_g_lambda * S_lambda * B_sun;
  }
  return B1;
}

inline vector <double> compute_E_lambda(const double& mu_0,
                                        const double& albedo,
                                        const double& tau_0_a,
                                        const double& beta,
                                        const double& g,
                                        const vector <double>& tau_m,
                                        const vector <double>& list) {
  vector<double> E;
  vector<double> tau_lambda = compute_tau_lambda(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (size_t i = 0; i < list.size(); ++i) {
    auto E_lmb = 4.0 * pi * omega_lambda[i]
                 * mu_0 /
                 (4.0 + 3.0 * (1.0 - g_lmb[i]) * (1.0 - albedo) * tau_lambda[i])
                 * ((0.5 + 0.75 * mu_0) + (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i] / mu_0))
                 + (1.0 - omega_lambda[i]) * pi * mu_0 * exp(-tau_lambda[i] / mu_0);
    E.push_back(E_lmb);

  }
  return E;
}



inline vector <double> compute_u(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> u;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (size_t i = 0; i < list.size(); ++i) {
    auto h0 = -1.88227 + 0.53661 * g_lmb[i] - 1.8047 * pow(g_lmb[i], 2) +
              3.26348 * pow(g_lmb[i], 3) - 2.3 * pow(g_lmb[i], 4);
    auto h1 = 5.97763 - 2.04621 * g_lmb[i] - 2.0173 *
              pow(g_lmb[i], 2) + 1.44843 * pow(g_lmb[i], 3);
    auto h2 = -5.47825 + 2.42154 * g_lmb[i] - 3.37057 *
              pow(g_lmb[i], 2) + 6.13805 * pow(g_lmb[i], 3);
    auto h3 = 2.07593 - 2.03761 * g_lmb[i] + 6.25975 *
              pow(g_lmb[i], 2) - 7.35503 * pow(g_lmb[i], 3);
    u.push_back(h0 + h1 + h2 + h3);
  }
  return u;
}


inline vector <double> compute_v(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> v;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (size_t i = 0; i < list.size(); ++i) {
    auto ro_0 = 0.4923 + 1.0471 * g_lmb[i] - 2.61112 *
                pow(g_lmb[i], 2) + 1.53155 * pow(g_lmb[i], 3);
    auto ro_1 = 4.01521 - 0.25886 * g_lmb[i] - 2.85378 *
                pow(g_lmb[i], 2) + 3.61515 * pow(g_lmb[i], 3);
    auto ro_2 = 3.76447 + 3.29106 * g_lmb[i] - 12.37951 *
                pow(g_lmb[i], 2) + 9.85 * pow(g_lmb[i], 3);
    v.push_back(ro_0 + ro_1 * exp(-ro_2));
  }
  return v;
}


inline vector <double> compute_w(const double& g,
                                 const double& tau_0_a,
                                 const double& beta,
                                 const vector <double>& tau_m,
                                 const vector <double>& list) {
  vector<double> w;
  vector<double> g_lmb = compute_g(g, tau_0_a, beta, tau_m, list);

  for (size_t i = 0; i < list.size(); ++i) {
    auto q_0 = 0.000076 - 0.316 * g_lmb[i] + 0.67744 * \
               pow(g_lmb[i], 2) - 0.4093 * pow(g_lmb[i], 3);
    auto q_1 = -1.31136 - 0.8901 * g_lmb[i] + 3.55 * \
               pow(g_lmb[i], 2) - 3.0646 * pow(g_lmb[i], 3);
    auto q_2 = 5.21931 + 7.2255 * g_lmb[i] - 23.43878 * \
               pow(g_lmb[i], 2) + 17.65629 * pow(g_lmb[i], 3);
    w.push_back(q_0 + q_1 * exp(-q_2));
  }
  return w;
}

inline vector <double> compute_T_dif(const double& tau_e,
                                     const double& tau_0_a,
                                     const double& beta,
                                     const double& g,
                                     const vector <double>& tau_m,
                                     const vector <double>& list) {

  auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
  auto u = compute_u(g, tau_0_a, beta, tau_m, list);
  auto v = compute_v(g, tau_0_a, beta, tau_m, list);
  auto w = compute_w(g, tau_0_a, beta, tau_m, list);
  vector<double> T_dif;

  for (size_t i = 0; i < list.size(); ++i) {
    T_dif.push_back(tau_lambda[i] * exp(-u[i] - v[i] * tau_lambda[i] - w[i] * tau_lambda[i] * tau_lambda[i]));
  }
  return T_dif;
}

inline vector <double> compute_T_lambda(const double& tau_e,
                                        const double& tau_0_a,
                                        const double& beta,
                                        const double& g,
                                        const vector <double>& tau_m,
                                        const vector <double>& list) {

  auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
  auto T_dif = compute_T_dif(tau_e, tau_0_a, beta, g, tau_m, list);
  vector<double> T_lmb;

  for (size_t i = 0; i < list.size(); ++i) {
    T_lmb.push_back((exp(-tau_lambda[i])) + T_dif[i]);
  }
  return T_lmb;
}

inline double compute_B2(const vector <double>& S_lambda_list,
                         const vector <double>& B_lambda_teta_list,
                         const vector <double>& T_O2_list,
                         const vector <double>& T_O3_list,
                         const vector <double>& T_H2O_list,
                         const double& mu_0,
                         const double& albedo,
                         const double& tau_0_a,
                         const double& beta,
                         const double& g,
                         const vector <double>& tau_m,
                         const vector <double>& list) {
  auto E_lambda = compute_E_lambda(mu_0, albedo, tau_0_a, beta, g, tau_m, list);
  auto T_lambda = compute_T_lambda(TAU_E, tau_0_a, beta, g, tau_m, list);
  double B2 = 0.0;
  auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
  for (size_t i = 0; i < list.size(); ++i) {
    auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
    auto S_lambda = S_lambda_list[i];
    auto B_sun = B_lambda_teta_list[i];
    auto T = T_lambda[i];
    auto E = E_lambda[i];
    B2 += E * T * T_g_lambda * S_lambda * B_sun;
  }
  return B2;
}

inline double compute_eq(const double& B1,
                         const double& B2,
                         const double& albedo,
                         const double& divider,
                         const double& dark_pixel) {
  double B = (B1 + B2 * albedo / pi) / divider;
  return dark_pixel - B;
}


inline vector <double> compute_EQ(const vector <double>& B_lambda_teta_list,
                                  const vector <double>& T_O2_list,
                                  const vector <double>& T_O3_list,
                                  const vector <double>& T_H2O_list,
                                  const vector <vector<double>>& S_lambda_lists,
                                  const double& mu_0,
                                  const double& albedo,
                                  const double& tau_0_a,
                                  const double& beta,
                                  const double& g,
                                  const vector <double>& tau_m,
                                  const vector <double>& list,
                                  const vector <double>& dividers,
                                  const vector <double>& dark_pixels) {

  vector <double> EQ;
  for (size_t i = 0; i < dark_pixels.size(); ++i) {
    auto B1 = compute_B1(T_O2_list, T_O3_list, T_H2O_list, S_lambda_lists[i],
                         B_lambda_teta_list, mu_0, tau_0_a, beta, g, tau_m, list);
    auto B2 = compute_B2(S_lambda_lists[i], B_lambda_teta_list, T_O2_list,
                         T_O3_list, T_H2O_list, mu_0, albedo, tau_0_a, beta, g, tau_m, list);
    EQ.push_back(compute_eq(B1, B2, albedo, dividers[i], dark_pixels[i]));
  }
  return EQ;
}

int quadfunc(int m, int n, double* p, double* dy, double** dvec, void* vars) {

  auto tau_0_a = p[0];
  auto beta    = p[1];
  auto g       = p[2];
  auto albedo  = p[3];

  auto eq = compute_EQ(B_lambda_teta_list,
                       T_O2_list,
                       T_O3_list,
                       T_H2O_list,
                       S_lambda_lists,
                       mu_0,
                       albedo,
                       tau_0_a,
                       beta,
                       g,
                       tau_m,
                       lambda_list,
                       divider_list,
                       dark_pixels
                      );

  for (int i = 0; i < m; i++) {
    dy[i] = eq[i];
  }
  rv.err_tau = dy[0];
  rv.err_beta = dy[1];
  rv.err_g = dy[2];
  rv.err_albedo = dy[3];
  return 0;
}

} // namespace

namespace lss {

void setElevationAngle(const double& elAngle) {
  mu_0 = qCos(qDegreesToRadians(90 - elAngle));
  qDebug() << "elevation angle: " << elAngle;
  qDebug() << "cos mu_0: " << mu_0;
}

void updateSatelliteResponses(const QString& satellite_name) {

  qDebug() << "Update satellite name...." << satellite_name;
  if (satellite_name == satellite_name_key) {
    return;
  }
  auto a1 = satellites["4"].toArray();
  //auto a2 = satellites["5"].toArray();
  bool isExists = false;
  for (int i = 0; i < a1.size(); ++i) {
    if (i == satellite_name) {
      isExists = true;
      break;
    }
  }
  /*if(!isExists){
    for(int i=0;i<a2.size();++i){
       if(i==satellite_name){
         isExists = true;
         break;
       }
    }
  }*/
  if (!isExists) {
    return;
  }
  satellite_name_key = satellite_name;
  for (int i = 0; i < sdb.size(); ++i) {
    auto response = sdb[i].toObject()[satellite_name_key].toArray();
    for (int j = 0; j < response.size(); ++j) {
      S_lambda_lists[j][i] = response[j].toDouble();
    }
  }
}

QStringList getSatellitesList() {

  return satellites_list;
}

result_values optimize(const QString& sat_name, const QVector<double>& blacks) {

  if (is_first_run) {
    loadAllLists();
    is_first_run = false;
  }
  updateSatelliteResponses(sat_name);
  double p[] =      {0.1, 2, 0.01, 0.01};//{0.1, 2, 0.01, 0.01};               /* Initial conditions */
  dark_pixels = {blacks[0], blacks[1], blacks[2], blacks[3]};
  double perror[4];                                    /* Returned parameter errors */
  mp_par pars[4];                                        /* Parameter constraints */
  vars_struct v;
  int status;
  mp_result result;

  memset(&result, 0, sizeof(result));      /* Zero results structure */
  result.xerror = perror;
  memset(pars, 0, sizeof(pars));          /* Initialize constraint structure */

  //[0.1, 2, 0.1, 0.01]), bounds=([0, 0.1, 0.1, 0.001], [1, 4, 1, 0.5]))

  //tau_0_a
  pars[0].limits[0] = 0;
  pars[0].limits[1] = 1;
  pars[0].limited[0] = 1;
  pars[0].limited[1] = 1;
  pars[0].side = 0;
  pars[0].step = 0.01;

  //beta
  pars[1].limits[0] = 0.001;
  pars[1].limits[1] = 4;
  pars[1].side = 0;
  pars[1].step = 0.01;
  pars[1].limited[0] = 1;
  pars[1].limited[1] = 1;

  //g
  pars[2].limits[0] = 0.0001;
  pars[2].limits[1] = 1;
  pars[2].side = 0;
  pars[2].step = 0.001;
  pars[2].limited[0] = 1;
  pars[2].limited[1] = 1;

  //albedo
  pars[3].limits[0] = 0.001;
  pars[3].limits[1] = 0.5;
  pars[3].side = 0;
  pars[3].step = 0.01;
  pars[3].limited[0] = 1;
  pars[3].limited[1] = 1;

  status = mpfit(quadfunc, 4, 4, p, pars, 0, (void*) &v, &result);

  qDebug() << "\nSTATUS: " << status;
  qDebug() << "VALUES: " << p[0] << p[1] << p[2] << p[3];
  qDebug() << "ERROR: " << rv.err_tau << rv.err_beta << rv.err_g << rv.err_albedo;

  rv.tau_0_a = p[0];
  rv.beta = p[1];
  rv.g = p[2];
  rv.albedo = p[3];

  return rv;
}


}
