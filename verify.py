#!/usr/bin/python3
#
import os
import sys

if len(sys.argv) > 1:
    dropfile = sys.argv[1]
else:
    print("Missing file name!")
    quit()

fin = open(dropfile, "r")

count = 0
for line in fin:
    fname = line.rstrip();
    rs1 = os.popen("./ripe -f " + fname).read().split()
    rs2 = os.popen("openssl dgst -ripemd160 " + fname).read().split()
    if rs1[0] != rs1[2]:
        print("Bad base64 for: ", fname)
    if rs1[0] != rs2[1]:
        print("Conflicting ripemd160 digest: ", fname)

    rs1 = os.popen("./sha " + fname).read().split()
    if len(rs1) == 4 and rs1[1] != rs1[3]:
        print("File and string mode differ: ", fname)
    rs2 = os.popen("openssl dgst -sha256 " + fname).read().split()
    if rs1[1] != rs2[1]:
        print("Conflicting sha256 digest: ", fname)
    count += 1

print("Total ", count, " files checked.")
fin.close()
