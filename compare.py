import sys
import subprocess
import os
from os import listdir
import pandas as pd

ckts = ['c432','c499','c880','c1355','c2670','c3540','c6288','c7552']
ndet = 8

# run atpg and sim
lTime1   = []
lLen1    = []
lTime2   = []
lLen2    = []
lTime3   = []
lLen3    = []
lTime4   = []
lLen4    = []

for i,ckt in enumerate(ckts):
    
    # Normal
    ## tdf atpg
    cmd = "./src/atpg -tdfatpg -ndet %d sample_circuits/%s.ckt > 1" % (ndet, ckt)
    cmd = "/usr/bin/time -f \"%e\" " + cmd
    result = subprocess.run(cmd, shell=True, capture_output=True)
    runtime = float(result.stderr) 
    ## parse result
    with open('1') as f:
        nLen = 0
        for line in f.readlines():
            if line.find('T') == 0:
                nLen += 1
    lTime1.append(runtime)
    lLen1.append(nLen)

    # STC
    ## tdf atpg
    cmd = "./src/atpg -tdfatpg -ndet %d -compression sample_circuits/%s.ckt > 1" % (ndet, ckt)
    cmd = "/usr/bin/time -f \"%e\" " + cmd
    result = subprocess.run(cmd, shell=True, capture_output=True)
    runtime = float(result.stderr) 
    ## parse result
    with open('1') as f:
        nLen = 0
        for line in f.readlines():
            if line.find('T') == 0:
                nLen += 1
    lTime2.append(runtime)
    lLen2.append(nLen)
    
    # DTC
    ## tdf atpg
    cmd = "./src/atpg_d -tdfatpg -ndet %d -compression sample_circuits/%s.ckt > 1" % (ndet, ckt)
    cmd = "/usr/bin/time -f \"%e\" " + cmd
    result = subprocess.run(cmd, shell=True, capture_output=True)
    runtime = float(result.stderr) 
    ## parse result
    with open('1') as f:
        nLen = 0
        for line in f.readlines():
            if line.find('T') == 0:
                nLen += 1
    lTime3.append(runtime)
    lLen3.append(nLen)

    # STC+DTC
    ## tdf atpg
    cmd = "./src/atpg_ds -tdfatpg -ndet %d -compression sample_circuits/%s.ckt > 1" % (ndet, ckt)
    cmd = "/usr/bin/time -f \"%e\" " + cmd
    result = subprocess.run(cmd, shell=True, capture_output=True)
    runtime = float(result.stderr) 
    ## parse result
    with open('1') as f:
        nLen = 0
        for line in f.readlines():
            if line.find('T') == 0:
                nLen += 1
    lTime4.append(runtime)
    lLen4.append(nLen)

os.system('rm 1')
lReduction_S  = [float(lLen1[i]-lLen2[i])/lLen1[i] for i in range(len(lLen1))]
lReduction_D  = [float(lLen1[i]-lLen3[i])/lLen1[i] for i in range(len(lLen1))]
lReduction_DS = [float(lLen1[i]-lLen4[i])/lLen1[i] for i in range(len(lLen1))]
# write to csv
dict = { "Name" : ckts,
         "Reduction_S"  : lReduction_S,
         "Reduction_D"  : lReduction_D,
         "Reduction_DS" : lReduction_DS,
         "Runtime"    : lTime1,
         "Runtime_S"  : lTime2,
         "Runtime_D"  : lTime3,
         "Runtime_DS" : lTime4 }
df = pd.DataFrame(dict)
df.sort_values(by='Name')
df.to_csv("compare.csv", index=False)    
