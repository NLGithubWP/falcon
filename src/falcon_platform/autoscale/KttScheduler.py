import math
import numpy as np
from scipy.optimize import minimize

def MaxV(a, b):
    if a > b:
        return a
    else:
        return b

def beta(x, t):
    mapper = dict()
    mapper[1] = 1.0
    mapper[2] = 0.95
    mapper[3] = 0.92
    mapper[4] = 0.91
    mapper[5] = 0.90
    mapper[6] = 0.88
    mapper[7] = 0.85
    mapper[8] = 0.80
    mapper[9] = 0.76
    mapper[10] = 0.71
    mapper[11] = 0.68
    mapper[12] = 0.65
    mapper[13] = 0.63
    mapper[14] = 0.58
    mapper[15] = 0.54
    mapper[16] = 0.53
    mapper[17] = 0.51
    mapper[18] = 0.47
    mapper[19] = 0.43
    mapper[20] = 0.39

    y = -2.94587705*math.pow(10, -8) * x ** 7 + 2.50848776*math.pow(10, -6) * x ** 6 - 8.78441842 *math.pow(10, -5) * x**5 + \
        0.00160672440 * x ** 4 - 0.01605 * x ** 3 + 0.08281 * x ** 2 - 0.2161 * x + 1.14976780
    # print(y, float(x))
    result = float(t) / (y* float(x))
    # result = float(t) * (1* 1/float(x))
    return result

def objective(x):
    return measure_total_time(x)

def worker_constraint(x):
    #  >=0
    return totalWorkers - measure_total_worker(x)

def time_constraint(x):
     # >=0
    return deadLine - measure_total_time(x) -1


def measure_total_worker(x):
    return 1 + x[0] + x[1] + x[2] * (x[3] + x[4])

def measure_total_time(x):
    return MaxV(beta(x[0], t2), beta(x[1], t3)) + math.ceil(6 / x[2]) * (beta(x[3], t4) + beta(x[4], t5))


def schedule():

    # initial guesses
    n = 5
    x0 = np.zeros(n)
    x0[0] = 1.0
    x0[1] = 1.0
    x0[3] = 1.0
    x0[4] = 1.0
    x0[2] = 1.0
    WorkerResult = x0

    # show initial objective
    # print('Initial SSE Objective: ' + str(objective(x0)))

    try:
        # optimize
        b = (1.0, totalWorkers-5)
        c = (1.0, classNum)
        bnds = (b, b, c, b, b)
        con1 = {'type': 'ineq', 'fun': worker_constraint} # constraint1 >=0
        con2 = {'type': 'ineq', 'fun': time_constraint} # constraint1 >=0
        cons = ([con1, con2])
        solution = minimize(fun=objective, x0=x0, method='SLSQP', bounds=bnds, constraints=cons)
        x = solution.x

        # show final objective
        # print solution
        # print(' ====== Original Solution ====== ')
        # print('x2 = ' + str(x[0]))
        # print('x3 = ' + str(x[1]))
        # print('x4 = ' + str(x[3]))
        # print('x5 = ' + str(x[4]))
        # print('xc = ' + str(x[2]))
        # print('====== Final SSE Objective, worker used=' + str(objective(x)))
        # print('====== timeUsed = ' + str(
        #     MaxV(beta(x[0], t2), beta(x[1], t3)) + math.ceil(6 / x[2]) * (beta(x[3], t4) + beta(x[4], t5))))

        X0 = [math.ceil(x[0]), math.floor(x[0])]
        X1 = [math.ceil(x[1]), math.floor(x[1])]
        X3 = [math.ceil(x[3]), math.floor(x[3])]
        X4 = [math.ceil(x[4]), math.floor(x[4])]
        X2 = [math.ceil(x[2]), math.floor(x[2])]

        bestResult = float("inf")
        for a in X0:
            for b in X1:
                for c in X2:
                    for d in X3:
                        for e in X4:
                            inputX = [a,b,c,d,e]
                            if time_constraint(inputX) >=0 and worker_constraint(inputX) >=0:
                                if objective(inputX) < bestResult:
                                    bestResult = objective(inputX)
                                    WorkerResult = inputX
    except:
        pass
        print("use default")

    x = WorkerResult
    # print(' ====== updated Solution ====== ', x)
    print(str(int(x[0])))
    print(str(int(x[1])))
    print(str(int(x[3])))
    print(str(int(x[4])))
    print(str(int(x[2])))
    # print('====== Final SSE Objective, worker used=' + str(measure_total_worker(x)))
    # print('====== timeUsed = ' + str(measure_total_time(x)))

def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--worker', type=int, help='total workers', default=7)
    args = parser.parse_args()

    global t2, t3, t4 ,t5, classNum, totalWorkers, deadLine
    # prediction time
    t2 = 60
    # // instance weighting time
    t3 = 30
    # // feature selection time
    t4 = 80
    # // VFL model training time
    t5 = 150
    classNum = 6

    totalWorkers = args.worker
    deadLine = 500

    schedule()

main()