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

numcpus = os.cpu_count()

class ProcFile(threading.Thread):
    def __init__(self, name, fname):
        threading.Thread.__init__(self)
        self.name = name
        self.fname = fname

    def run(self):
        print("Processing file: ", self.fname)
        rs1 = os.popen("./ripe -f " + self.fname).read().split()
        rs2 = os.popen("openssl dgst -ripemd160 " + self.fname).read().split()
        if rs1[0] != rs1[2]:
            print("Bad base64 for: ", self.fname)
        if rs1[0] != rs2[1]:
            print("Conflicting ripemd160 digest: ", self.fname)

        rs1 = os.popen("./sha " + self.fname).read().split()
        if len(rs1) == 4 and rs1[1] != rs1[3]:
            print("File and string mode differ: ", self.fname)
        rs2 = os.popen("openssl dgst -sha256 " + self.fname).read().split()
        if rs1[1] != rs2[1]:
            print("Conflicting sha256 digest: ", self.fname)

fin = open(dropfile, "r")

thlist = []
count = 0
for line in fin:
    fname = line.rstrip();
    mth = ProcFile("check" + str(count), fname)
    thlist.append(mth)
    mth.start()
    if len(thlist) > numcpus + 1:
        mth = thlist[0]
        del thlist[0]
        mth.join()
    count += 1

fin.close()
for mth in thlist:
    mth.join()

print("Total ", count, " files checked.")
