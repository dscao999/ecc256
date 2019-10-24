#!/usr/bin/python3
#
import os
import sys
import threading

if len(sys.argv) > 1:
    dropfile = sys.argv[1]
else:
    print("Missing file name!")
    quit()

if not os.path.isfile(dropfile) or not os.access(dropfile, os.R_OK):
    print("Cannot read file: ", dropfile)
    quit()

numcpus = os.cpu_count()

class ProcFile(threading.Thread):
    def __init__(self, name, fname):
        threading.Thread.__init__(self)
        self.name = name
        self.fname = fname

    def run(self):
        path = "'" + self.fname + "'"
        rs1 = os.popen("./ripe " + path).read().split()
        if len(rs1) > 2 and rs1[0] != rs1[1]:
            print("File mode differs with string mode: ", self.fname)
        rs2 = os.popen("openssl dgst -ripemd160 " + path).read().split()
        ssl = rs2[-1]
        if rs1[0] != ssl:
            print("Conflicting ripemd160 digest: ", self.fname)

        rs1 = os.popen("./sha " + path).read().split()
        if len(rs1) > 2 and rs1[0] != rs1[1]:
            print("File and string mode differ: ", self.fname)
        rs2 = os.popen("openssl dgst -sha256 " + path).read().split()
        ssl = rs2[-1]
        if rs1[0] != ssl:
            print("Conflicting sha256 digest: ", self.fname)

fin = open(dropfile, "r")

thlist = []
count = 0
for line in fin:
    fname = line.rstrip();
    if not os.path.isfile(fname) or not os.access(fname, os.R_OK):
        continue
    mth = ProcFile("check" + str(count), fname)
    thlist.append(mth)
    mth.start()
    if len(thlist) > numcpus + 1:
        mth = thlist[0]
        del thlist[0]
        mth.join()
    count += 1
    if count % 1000 == 0:
        print("Total ", str(count), " files processed.")

fin.close()
for mth in thlist:
    mth.join()

print("Total ", count, " files checked.")
