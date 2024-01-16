# -*- coding: utf-8 -*-

from math import pi
import numpy as np
import scipy.optimize
from tqdm import tqdm
import time
import catlib


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def load_T_O2_list():
    lines = open('files/o2.txt').readlines()
    return [float(l) for l in lines]

def load_T_O3_list():
    lines = open('files/o3.txt').readlines()
    return [float(l) for l in lines]

def load_T_H2O_list():
    lines = open('files/h2o.txt').readlines()
    return [float(l) for l in lines]

def load_lambda_list():
    lines = open('files/lmb.txt').readlines()
    return [float(l) for l in lines]

def load_B_lambda_teta_list():
    lines = open('files/sun.txt').readlines()
    return [float(l) for l in lines]

def load_S_lambda_lists():
    lists = []
    for i in range(len(dark_pixel)):
        file_name = 'files/i{0}.txt'.format(i+1)
        lines = open(file_name).readlines()
        list_ = []
        for l in lines:
            not_added = True
            while not_added:
                try:
                    list_.append(float(l))
                    not_added = False
                except:
                    l = l[1:]
        lists.append(list_)
    for l in lists:
        assert len(l) == len(lists[0])
    return lists

def compute_divider_list(S_lambda_lists):
    divider_list = []
    for lambda_list in S_lambda_lists:
        sum = 0.0
        for lambda_ in lambda_list:
            sum += lambda_
        divider_list.append(sum)
    return divider_list

def print_welcome():
    print(bcolors.OKCYAN+
          bcolors.BOLD+
          "\n*********************************\n*     cat script is running     *\n*********************************"+bcolors.ENDC)

def print_cos_zenith_angle():
    print(bcolors.WARNING+"\nКосинус зенитного угла Солнца: \n"+bcolors.ENDC, mu_0)
    
def printFilesReadingInfo():
    print(bcolors.WARNING+'\nLoaded files information:'+bcolors.ENDC)
    print('Lambdas loaded:       {0}'.format(list_size))
    print('T_O2 loaded:          {0}'.format(len(T_O2_list)))
    print('T_O3 loaded:          {0}'.format(len(T_O3_list)))
    print('T_H2O loaded:         {0}'.format(len(T_H2O_list)))
    print('B_lambda_teta loaded: {0}'.format(len(B_lambda_teta_list)))
    print('S_lambda loaded:      {0}x{1}'.format(
        len(S_lambda_lists), len(S_lambda_lists[0])))
    print('Dividers:             {0}'.format(divider_list))

def printResults():
    #print(x)
    print(bcolors.WARNING + "\nРезультаты расчетов: "+ bcolors.ENDC)
    print('tau_0_a = ', round(x.x[0], 4),
          '\nbeta = ',  round(x.x[1], 4),
          '\ng = ',     round(x.x[2], 4),
          '\nalb = ',   round(x.x[3], 4))
    print(bcolors.WARNING +'\nОшибки расчетов: '+ bcolors.ENDC, x.fun)
    print(bcolors.WARNING +"Время выполнения: " + bcolors.ENDC,
          (time.time() - start_time))
    print("\n")

def assertListsSizes():
    assert list_size == len(T_O2_list)
    assert list_size == len(T_O3_list)
    assert list_size == len(T_H2O_list)
    assert list_size == len(S_lambda_lists[0])
    assert list_size == len(B_lambda_teta_list)



# Calculate errors with C++ catlib
def equasion(var):
    tau_0_a, beta, g, alb = var        
    EQ = catlib.compute_EQ(
                      B_lambda_teta_list,
                      T_O2_list,
                      T_O3_list,
                      T_H2O_list,
                      S_lambda_lists,
                      mu_0,
                      alb,
                      tau_0_a,
                      beta,
                      g,
                      tau_m,
                      lambda_list,
                      divider_list,
                      dark_pixel
                      )
    return EQ

# Entry point to main function
if __name__ == '__main__':
    # start script time point
    start_time = time.time()
    
    #welcome message
    print_welcome()

    # sun zenith angle
    mu_0 = np.cos(np.deg2rad(41.3))
    print_cos_zenith_angle()

    # dark pixel test data
    dark_pixel = [39.535587, 25.645323, 11.881793, 4.310712]

    # load data from files
    lambda_list        = load_lambda_list()
    list_size          = len(lambda_list)
    S_lambda_lists     = load_S_lambda_lists()
    divider_list       = compute_divider_list(S_lambda_lists)
    T_O2_list          = load_T_O2_list()
    T_O3_list          = load_T_O3_list()
    T_H2O_list         = load_T_H2O_list()
    B_lambda_teta_list = load_B_lambda_teta_list()

    # check lists sizes
    assertListsSizes()

    # print information about sizes of loaded data from files
    printFilesReadingInfo()

    # Calculate tau_m only one time at the start
    tau_m = catlib.compute_tau_m(lambda_list)

    # Least squares solver
    x = scipy.optimize.least_squares(equasion, np.asarray(
        [0.1, 2, 0.1, 0.01]), bounds=([0, 0.1, 0.1, 0.001], [1, 4, 1, 0.5]))

    # print final results
    printResults()
    