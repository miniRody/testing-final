import sys
import subprocess
import os
from os import listdir
import pandas as pd

ckts = ['c432','c499','c880','c1355','c2670','c3540','c6288','c7552']
N = 8

# create & clear pattern dir
pat_dir = 'patterns'
if not os.path.isdir(pat_dir):
    os.mkdir(pat_dir)
else:
    os.system('rm -rf %s/*' % pat_dir)

# create & clear simulation log dir
log_dir = 'log'
if not os.path.isdir(log_dir):
    os.mkdir(log_dir)
else:
    os.system('rm -rf %s/*' % log_dir)

# create & clear report dir
report_dir = 'report'
if not os.path.isdir(report_dir):
    os.mkdir(report_dir)
else:
    os.system('rm -rf %s/*' % report_dir)

# run atpg and sim
for ndet in range(1,N+1):
    lTime = []
    lLen  = []
    lFC   = []
    for i,ckt in enumerate(ckts):
        
        # tdf atpg
        cmd = "./src/atpg -tdfatpg -ndet %d sample_circuits/%s.ckt > %s/%s_%d.pat" % (ndet, ckt, pat_dir, ckt, ndet)
        cmd = "/usr/bin/time -f \"%e\" " + cmd
        result = subprocess.run(cmd, shell=True, capture_output=True)
        runtime = float(result.stderr)
        
        # tdf sim
        cmd = "./bin/golden_tdfsim -ndet %d -tdfsim %s/%s_%d.pat sample_circuits/%s.ckt > %s/%s_%d.log" % (ndet, pat_dir, ckt, ndet, ckt, log_dir, ckt, ndet)
        subprocess.run(cmd, shell=True)

        # parse result
        with open('%s/%s_%d.log' % (log_dir, ckt, ndet)) as f:
            nLen = 0
            for line in f.readlines():
                if line.find('vector') == 0:
                    nLen += 1
                if line.find('fault coverage') != -1:
                    fc = float(line.split()[3])
        lTime.append(runtime)
        lLen.append(nLen)
        lFC.append(fc)

    # write to csv
    dict = { "Name" : ckts,
             "Test Length" : lLen,
             "Fault Coverage" : lFC,
             "Runtime" : lTime }
    df = pd.DataFrame(dict)
    df.sort_values(by='Name')
    df.to_csv("%s/%d.csv" % (report_dir, ndet), index=False)    
