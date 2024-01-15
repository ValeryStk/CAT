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
#include "satellite_adder.h"

namespace{
using std::vector;

constexpr double TAU_M_0 = 0.101;
constexpr double LAMBDA_0 = 0.55;
constexpr double P = 1.25;
constexpr double Q = 1;
constexpr double TAU_E = 0.04;
constexpr double pi = 3.14159265358979323846;
vector <double>  dark_pixel = {39.535587, 25.645323, 11.881793, 4.310712};
double mu_0 = qCos(qDegreesToRadians(41.3));
static result_values rv;

void loadList(QString path,vector<double>&list);
void loadBkaList(QString path, vector<double> &m_bkaList);
void calculDividerList(vector<vector<double> > &allBka);
inline vector <double> compute_tau_m(const vector<double> &list);



void loadAllLists()
{
    //S_lambda_lists_sentinel
    loadBkaList(":/sattelites_params/bka/i1.txt",S_lambda_lists_bka[0]);//bka
    loadBkaList(":/sattelites_params/bka/i2.txt",S_lambda_lists_bka[1]);
    loadBkaList(":/sattelites_params/bka/i3.txt",S_lambda_lists_bka[2]);
    loadBkaList(":/sattelites_params/bka/i4.txt",S_lambda_lists_bka[3]);
    loadBkaList(":/sattelites_params/sentinel/i1.txt",S_lambda_lists_sentinel[0]);//sentinel
    loadBkaList(":/sattelites_params/sentinel/i2.txt",S_lambda_lists_sentinel[1]);
    loadBkaList(":/sattelites_params/sentinel/i3.txt",S_lambda_lists_sentinel[2]);
    loadBkaList(":/sattelites_params/sentinel/i4.txt",S_lambda_lists_sentinel[3]);
    loadList(":/sattelites_params/h2o.txt",T_H2O_list);
    loadList(":/sattelites_params/lmb.txt",lambda_list);
    loadList(":/sattelites_params/o2.txt",T_O2_list);
    loadList(":/sattelites_params/o3.txt",T_O3_list);
    loadList(":/sattelites_params/sun.txt",B_lambda_teta_list);
    calculDividerList(S_lambda_lists_bka);
    tau_m = compute_tau_m(lambda_list);

    QJsonArray bka_array;
    QJsonArray common_params;
    for(size_t j=0;j<S_lambda_lists_bka[0].size();++j){
    QJsonObject obj_response;
    QJsonObject params;
    QJsonArray jarr_bka;
    QJsonArray jarr_sentinel;
    obj_response["wavelength"] = lambda_waves[j];
    params["wavelength"] = lambda_waves[j];
    params["h2o"] = T_H2O_list[j];
    params["o2"] = T_O2_list[j];
    params["o3"] = T_O3_list[j];
    params["sun"] = B_lambda_teta_list[j];

    for(size_t i=0;i<4;++i){
     jarr_bka.append(S_lambda_lists_bka[i][j]);
     jarr_sentinel.append(S_lambda_lists_sentinel[i][j]);
    }
    obj_response["response"] = jarr_bka;
    params["_bka"] = jarr_bka;
    params["_sentinel"] = jarr_sentinel;
    common_params.append(params);
    bka_array.append(obj_response);
    }
    QJsonObject obj;
    obj.insert("bka_bands",bka_array);

    db_json::saveJsonObjectToFile("bka.json",obj,QJsonDocument::Indented);
    db_json::saveJsonArrayToFile("common_compact.json",common_params,QJsonDocument::Indented);
    cat::add_new_satellite("landsat8");
    //cat::add_new_satellite("landsat9");
    //cat::add_new_satellite("sentinel2a-10m");
    //cat::add_new_satellite("sentinel2a-20m");
    //cat::add_new_satellite("sentinel2b-10m");
    //cat::add_new_satellite("sentinel2b-20m");
}

void loadList(QString path,vector<double>&list)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){

        return;
    }
    QTextStream qts(&file);
    QString line;
    while(qts.readLineInto(&line)){

        double var = line.toDouble();
        list.push_back(var);
    }
    file.close();
    qDebug()<<"List "<<path<<" "<<list.size();
}

void loadBkaList(QString path, vector<double> &m_bkaList)
{
    bool static isfillWaves = true;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){

        return;
    }
    QTextStream qts(&file);
    QString line;
    while(qts.readLineInto(&line)){
        double var2;
        QStringList twoParams = line.split("\t");
        if(twoParams.count()==2){
            var2 = twoParams.at(1).toDouble();
            if(isfillWaves){
            lambda_waves.push_back(twoParams.at(0).toDouble());
            }
        }
        else{
            var2 = line.toDouble();
        }
        m_bkaList.push_back(var2);
    }
    file.close();
    isfillWaves = false;
    qDebug()<<"List "<<path<<" "<<m_bkaList.size();
}

void calculDividerList(vector<vector<double> > &allBka)
{

    double sum1=0;
    double sum2=0;
    double sum3=0;
    double sum4=0;
    int sizeList = allBka[0].size();

    for(uintmax_t i=0;i<allBka.size();++i){

        for(int j=0;j<sizeList;++j){
            if(i==0){sum1+=allBka[i][j];}
            if(i==1){sum2+=allBka[i][j];}
            if(i==2){sum3+=allBka[i][j];}
            if(i==3){sum4+=allBka[i][j];}

        }
    }
    divider_list = {sum1,sum2,sum3,sum4};
    qDebug()<<"DIVIDED: "<<sum1<<sum2<<sum3<<sum4;
}


inline vector <double> compute_tau_m(const vector<double> &list){
    std::vector <double> result;
    for(uintmax_t i = 0; i < list.size(); ++i){
        auto lambda_0_lambda = LAMBDA_0 / list[i];
        result.push_back(TAU_M_0 * pow(lambda_0_lambda,4));
    }
    return result;
}

inline double compute_x_m(const double &mu_0) {
    return  3 * (1 + pow(mu_0,2)) / 4;
}

inline double compute_x_a(const double &mu_0, const double &g){
    return (1 - pow(g,2)) / pow((1 + pow(g,2) + 2 * g * mu_0),1.5);
}

inline vector <double> compute_tau_a(const double &tau_0_a,
                                     const double &beta,
                                     const vector <double> &list){
    std::vector <double> result;
    for(uintmax_t i = 0; i < list.size(); ++i){
        double lambda_0_lambda = LAMBDA_0 / list[i];
        result.push_back(tau_0_a * pow(lambda_0_lambda,beta));
    }
    return result;
}


inline vector <double> compute_tau_lambda(const double &tau_e,
                                          const double &tau_0_a,
                                          const double &beta,
                                          const vector <double> &tau_m,
                                          const vector <double> &list){
    std::vector <double> result;
    std::vector <double> tau_a = compute_tau_a(tau_0_a,beta,list);
    for(uintmax_t i=0; i<list.size(); ++i){
        result.push_back(tau_m[i] + tau_a[i] + tau_e);
    }
    return result;
}

inline vector <double> compute_omega(const double &tau_e,
                                     const double &tau_0_a,
                                     const double &beta,
                                     const vector <double> &tau_m,
                                     const vector <double> &list){
    std::vector<double> result;
    std::vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    for(uintmax_t i = 0; i < list.size(); ++i){
        result.push_back((tau_m[i] + tau_a[i]) / (tau_m[i] + tau_a[i] + tau_e));
    }
    return result;
}

inline vector <double> compute_g(const double &g,
                                 const double &tau_0_a,
                                 const double &beta,
                                 const vector <double> &tau_m,
                                 const vector <double> &list){
    vector<double> result;
    vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    for(uintmax_t i = 0; i < list.size(); ++i){
        result.push_back(g * tau_a[i] / (tau_m[i] + tau_a[i]));
    }
    return result;
}

inline vector <double> compute_x(const double &mu_0,
                                 const double &g,
                                 const double &tau_0_a,
                                 const double &beta,
                                 const vector <double> &tau_m,
                                 const vector <double> &list){
    vector<double> result;
    vector<double> tau_a = compute_tau_a(tau_0_a, beta, list);
    auto x_m = compute_x_m(mu_0);
    auto x_a = compute_x_a(mu_0, g);
    for(uintmax_t i = 0; i < list.size(); ++i){
        result.push_back((x_m * tau_m[i] + x_a * tau_a[i]) / (tau_m[i] + tau_a[i]));
    }
    return result;
}

inline vector <double> compute_B_atm(const double &mu_0,
                                     const double &tau_0_a,
                                     const double &beta,
                                     const double &g,
                                     const vector <double> &tau_m,
                                     const vector <double> &list){
    vector<double> result;
    vector<double> tau_lambda = compute_tau_lambda(TAU_E,tau_0_a, beta,tau_m, list);
    vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> x = compute_x(mu_0, g, tau_0_a, beta,tau_m, list);

    for(uintmax_t i = 0; i < list.size(); ++i){
        auto b_atm = omega_lambda[i] * x[i] / (4.0 * (1.0 + mu_0)) * (1.0 - exp(-tau_lambda[i] * (
                                                                                    1.0/mu_0 + 1.0))) * (1.0 + Q * pow(omega_lambda[i] * tau_lambda[i],P));
        result.push_back(b_atm);

    }
    return result;
}

inline double compute_B1(const vector <double> &T_O2_list,
                         const vector <double> &T_O3_list,
                         const vector <double> &T_H2O_list,
                         const vector <double> &S_lambda_list,
                         const vector <double> &B_lambda_teta_list,
                         const double &mu_0,
                         const double &tau_0_a,
                         const double &beta,
                         const double &g,
                         const vector <double> &tau_m,
                         const vector <double> &list){
    double B1 = 0.0;
    auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
    for(uintmax_t i = 0; i < list.size(); ++i){
        auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
        auto S_lambda = S_lambda_list[i];
        auto B_sun = B_lambda_teta_list[i];
        auto b = B_atm[i];
        B1 += b * T_g_lambda * S_lambda * B_sun;
    }
    return B1;
}

inline vector <double> compute_E_lambda(const double &mu_0,
                                        const double &albedo,
                                        const double &tau_0_a,
                                        const double &beta,
                                        const double &g,
                                        const vector <double> &tau_m,
                                        const vector <double> &list){
    vector<double> E;
    vector<double> tau_lambda = compute_tau_lambda(TAU_E,tau_0_a, beta,tau_m,list);
    vector<double> omega_lambda = compute_omega(TAU_E, tau_0_a, beta, tau_m, list);
    vector<double> g_lmb = compute_g(g, tau_0_a, beta,tau_m,list);

    for(uintmax_t i=0;i < list.size();++i){
        auto E_lmb = 4.0 * pi * omega_lambda[i]
                * mu_0 /
                (4.0 + 3.0 * (1.0 - g_lmb[i]) * (1.0 - albedo) * tau_lambda[i])
                * ((0.5 + 0.75 * mu_0) + (0.5 - 0.75 * mu_0) * exp(-tau_lambda[i]/mu_0))
                + (1.0 - omega_lambda[i]) * pi * mu_0 * exp(-tau_lambda[i]/mu_0);
        E.push_back(E_lmb);

    }
    return E;
}



inline vector <double> compute_u(const double &g,
                                 const double &tau_0_a,
                                 const double &beta,
                                 const vector <double> &tau_m,
                                 const vector <double> &list){
    vector<double> u;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta,tau_m,list);

    for(uintmax_t i = 0; i < list.size(); ++i){
        auto h0 = -1.88227 + 0.53661 * g_lmb[i] - 1.8047 * pow(g_lmb[i],2) +
                3.26348 * pow(g_lmb[i],3) - 2.3 * pow(g_lmb[i],4);
        auto h1 = 5.97763 - 2.04621 * g_lmb[i] - 2.0173 *
                pow(g_lmb[i],2) + 1.44843 * pow(g_lmb[i],3);
        auto h2 = -5.47825 + 2.42154 * g_lmb[i] - 3.37057 *
                pow(g_lmb[i],2) + 6.13805 * pow(g_lmb[i],3);
        auto h3 = 2.07593 - 2.03761 * g_lmb[i] + 6.25975 *
                pow(g_lmb[i],2) - 7.35503 * pow(g_lmb[i],3);
        u.push_back(h0 + h1 + h2 + h3);
    }
    return u;
}


inline vector <double> compute_v(const double &g,
                                 const double &tau_0_a,
                                 const double &beta,
                                 const vector <double> &tau_m,
                                 const vector <double> &list){
    vector<double> v;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta,tau_m,list);

    for(uintmax_t i = 0; i < list.size(); ++i){
        auto ro_0 = 0.4923 + 1.0471 * g_lmb[i] - 2.61112 *
                pow(g_lmb[i],2) + 1.53155 * pow(g_lmb[i],3);
        auto ro_1 = 4.01521 - 0.25886 * g_lmb[i] - 2.85378 *
                pow(g_lmb[i],2) + 3.61515 * pow(g_lmb[i],3);
        auto ro_2 = 3.76447 + 3.29106 * g_lmb[i] - 12.37951 *
                pow(g_lmb[i],2) + 9.85 * pow(g_lmb[i],3);
        v.push_back(ro_0 + ro_1 * exp(-ro_2));
    }
    return v;
}


inline vector <double> compute_w(const double &g,
                                 const double &tau_0_a,
                                 const double &beta,
                                 const vector <double> &tau_m,
                                 const vector <double> &list){
    vector<double> w;
    vector<double> g_lmb = compute_g(g, tau_0_a, beta,tau_m,list);

    for(uintmax_t i=0; i<list.size(); ++i){
        auto q_0 = 0.000076 - 0.316 * g_lmb[i] + 0.67744 * \
                pow(g_lmb[i],2) - 0.4093 * pow(g_lmb[i],3);
        auto q_1 = -1.31136 - 0.8901 * g_lmb[i] + 3.55 * \
                pow(g_lmb[i],2) - 3.0646 * pow(g_lmb[i],3);
        auto q_2 = 5.21931 + 7.2255 * g_lmb[i] - 23.43878 * \
                pow(g_lmb[i],2) + 17.65629 * pow(g_lmb[i],3);
        w.push_back(q_0 + q_1 * exp(-q_2));
    }
    return w;
}



inline vector <double> compute_T_dif(const double &tau_e,
                                     const double &tau_0_a,
                                     const double &beta,
                                     const double &g,
                                     const vector <double> &tau_m,
                                     const vector <double> &list){

    auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    auto u = compute_u(g, tau_0_a, beta,tau_m,list);
    auto v = compute_v(g, tau_0_a, beta,tau_m,list);
    auto w = compute_w(g, tau_0_a, beta,tau_m,list);
    vector<double> T_dif;

    for(uintmax_t i = 0; i < list.size(); ++i){
        T_dif.push_back(tau_lambda[i] * exp(-u[i] - v[i] * tau_lambda[i] - w[i] * tau_lambda[i] * tau_lambda[i]));
    }
    return T_dif;
}


inline vector <double> compute_T_lambda(const double &tau_e,
                                        const double &tau_0_a,
                                        const double &beta,
                                        const double &g,
                                        const vector <double> &tau_m,
                                        const vector <double> &list){

    auto tau_lambda = compute_tau_lambda(tau_e, tau_0_a, beta, tau_m, list);
    auto T_dif = compute_T_dif(tau_e, tau_0_a, beta, g, tau_m, list);
    vector<double> T_lmb;

    for(uintmax_t i = 0; i < list.size(); ++i){
        T_lmb.push_back((exp(-tau_lambda[i])) + T_dif[i]);
    }
    return T_lmb;
}


inline double compute_B2(const vector <double> &S_lambda_list,
                         const vector <double> &B_lambda_teta_list,
                         const vector <double> &T_O2_list,
                         const vector <double> &T_O3_list,
                         const vector <double> &T_H2O_list,
                         const double &mu_0,
                         const double &albedo,
                         const double &tau_0_a,
                         const double &beta,
                         const double &g,
                         const vector <double> &tau_m,
                         const vector <double> &list){
    auto E_lambda = compute_E_lambda(mu_0, albedo, tau_0_a, beta, g,tau_m, list);
    auto T_lambda = compute_T_lambda(TAU_E,tau_0_a, beta, g,tau_m, list);
    double B2 = 0.0;
    auto B_atm = compute_B_atm(mu_0, tau_0_a, beta, g, tau_m, list);
    for(uintmax_t i = 0; i < list.size(); ++i){
        auto T_g_lambda = T_O2_list[i] * T_O3_list[i] * T_H2O_list[i];
        auto S_lambda = S_lambda_list[i];
        auto B_sun = B_lambda_teta_list[i];
        auto T = T_lambda[i];
        auto E = E_lambda[i];
        B2 += E * T * T_g_lambda * S_lambda * B_sun;
    }
    return B2;
}



inline double compute_eq(const double &B1,
                         const double &B2,
                         const double &albedo,
                         const double &divider,
                         const double &dark_pixel){
    double B = (B1 + B2* albedo / pi) / divider;
    return dark_pixel - B;
}


inline vector <double> compute_EQ(const vector <double> &B_lambda_teta_list,
                                  const vector <double> &T_O2_list,
                                  const vector <double> &T_O3_list,
                                  const vector <double> &T_H2O_list,
                                  const vector <vector<double>> &S_lambda_lists,
                                  const double &mu_0,
                                  const double &albedo,
                                  const double &tau_0_a,
                                  const double &beta,
                                  const double &g,
                                  const vector <double> &tau_m,
                                  const vector <double> &list,
                                  const vector <double> &dividers,
                                  const vector <double> &dark_pixels){

    vector <double> EQ;
    for(uintmax_t i = 0; i< dark_pixels.size(); ++i){
        auto B1 = compute_B1(T_O2_list,T_O3_list,T_H2O_list,S_lambda_lists[i],
                             B_lambda_teta_list,mu_0, tau_0_a, beta, g, tau_m, list);
        auto B2 = compute_B2(S_lambda_lists[i], B_lambda_teta_list, T_O2_list,
                             T_O3_list, T_H2O_list, mu_0, albedo, tau_0_a, beta, g, tau_m, list);
        EQ.push_back(compute_eq(B1,B2,albedo,dividers[i], dark_pixels[i]));
    }
    return EQ;
}

}

namespace lss{
/* This is the private data structure which contains the data points
   and their uncertainties */

void setElevationAngle(const double &elAngle){
    mu_0 = qCos(qDegreesToRadians(elAngle));
    qDebug()<<"elevation angle: "<<elAngle;
    qDebug()<<"cos mu_0: "<<mu_0;
}

struct vars_struct {
    double *tau_0_a_err;
    double *beta_err;
    double *g_err;
    double *albedo_err;
};

/*struct result_values{
    double tau_0_a; // фикс при вычеслении lookup table
    double beta;    // фикс при вычеслении lookup table
    double g;       // фикс при вычеслении lookup table
    double albedo;  // искомое для матрицы
};*/

/* Simple routine to print the fit results */
void printresult(double *x, double *xact, mp_result *result)
{
    int i;

    if ((x == 0) || (result == 0)) return;
    printf("  CHI-SQUARE = %f    (%d DOF)\n",
           result->bestnorm, result->nfunc-result->nfree);
    printf("     NPAR    = %d\n", result->npar);
    printf("     NFREE   = %d\n", result->nfree);
    printf("     NPEGGED = %d\n", result->npegged);
    printf("     NITER   = %d\n", result->niter);
    printf("     NFEV    = %d\n", result->nfev);
    printf("\n");
    if (xact) {
        for (i=0; i<result->npar; i++) {
            printf("  P[%d] = %f +/- %f     (ACTUAL %f)\n",
                   i, x[i], result->xerror[i], xact[i]);
        }
    } else {
        for (i=0; i<result->npar; i++) {
            printf("  P[%d] = %f +/- %f\n",
                   i, x[i], result->xerror[i]);
        }
    }
    printf("\n");
}




int quadfunc(int m, int n, double *p, double *dy, double **dvec, void *vars)
{

    //printf ("quadfunc %f %f %f %f\n", p[0], p[1], p[2], p[3]);

    auto tau_0_a = p[0];
    auto beta    = p[1];
    auto g       = p[2];
    auto albedo  = p[3];

    //qDebug()<<"vals: " << tau_0_a << beta<< g << albedo;

    auto eq = compute_EQ(B_lambda_teta_list,
                         T_O2_list,
                         T_O3_list,
                         T_H2O_list,
                         S_lambda_lists_bka,
                         mu_0,
                         albedo,
                         tau_0_a,
                         beta,
                         g,
                         tau_m,
                         lambda_list,
                         divider_list,
                         dark_pixel
                         );

    for (int i=0; i<m; i++) {
        dy[i] = eq[i];
    }
    rv.err_tau = dy[0];
    rv.err_beta = dy[1];
    rv.err_g = dy[2];
    rv.err_albedo = dy[3];
    return 0;
}

result_values optimize(std::array<double,4>blacks)
{
    loadAllLists();
    double p[] =      {0.1, 2, 0.01, 0.01};//{0.1, 2, 0.01, 0.01};               /* Initial conditions */
    dark_pixel = {blacks[0],blacks[1],blacks[2],blacks[3]};
    double perror[4];		                                 /* Returned parameter errors */
    mp_par pars[4];                                        /* Parameter constraints */
    struct vars_struct v;
    int status;
    mp_result result;

    memset(&result,0,sizeof(result));        /* Zero results structure */
    result.xerror = perror;
    memset(pars, 0, sizeof(pars));          /* Initialize constraint structure */

    //[0.1, 2, 0.1, 0.01]), bounds=([0, 0.1, 0.1, 0.001], [1, 4, 1, 0.5]))

    //tau_0_a
    pars[0].limits[0] = 0;
    pars[0].limits[1] = 1;
    pars[0].limited[0]=1;
    pars[0].limited[1]=1;
    pars[0].side = 0;
    pars[0].step = 0.01;

    //beta
    pars[1].limits[0] = 0.001;
    pars[1].limits[1] = 4;
    pars[1].side = 0;
    pars[1].step = 0.01;
    pars[1].limited[0]=1;
    pars[1].limited[1]=1;

    //g
    pars[2].limits[0] = 0.0001;
    pars[2].limits[1] = 1;
    pars[2].side = 0;
    pars[2].step = 0.001;
    pars[2].limited[0]=1;
    pars[2].limited[1]=1;

    //albedo
    pars[3].limits[0] = 0.001;
    pars[3].limits[1] = 0.5;
    pars[3].side = 0;
    pars[3].step = 0.01;
    pars[3].limited[0]=1;
    pars[3].limited[1]=1;

    status = mpfit(quadfunc, 4, 4, p, pars, 0, (void *) &v, &result);

    qDebug()<<"\nSTATUS: "<<status;
    qDebug()<<"VALUES: "<<p[0]<<p[1]<<p[2]<<p[3];

    rv.tau_0_a = p[0];
    rv.beta = p[1];
    rv.g = p[2];
    rv.albedo = p[3];

    return rv;
}
}
