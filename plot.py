import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

ckts = ['c432','c499','c880','c1355','c2670','c3540','c6288','c7552']
ndet = np.arange(1, 9, dtype=int)
length = np.empty((len(ckts),8), dtype=int)
length_C = np.empty((len(ckts),8), dtype=int)

for N in ndet:
    df = pd.read_csv('report/%d.csv' % N).reset_index()
    arr = np.array(df.iloc[:,2], dtype=int).reshape(len(ckts))
    length[:,N-1] = arr
    arr = np.array(df.iloc[:,5], dtype=int)
    length_C[:,N-1] = arr

# w/o
for N in ndet:
    plt.plot(ndet, length[N-1].reshape(8), marker='.', label=ckts[N-1])
plt.title("Test Length(w/o compression)")
plt.xlabel("ndet")
plt.ylabel("length")
plt.legend(loc="best")
plt.savefig("Length1.png")
plt.clf()

# w
for N in ndet:
    plt.plot(ndet, length_C[N-1].reshape(8), marker='.', label=ckts[N-1])
plt.title("Test Length(w/ compression)")
plt.xlabel("ndet")
plt.ylabel("length")
plt.legend(loc="best")
plt.savefig("Length2.png")
