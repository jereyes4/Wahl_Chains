#!/usr/bin/env python3
# Run from ../
import glob, os

Files = glob.glob("jsonl/*.jsonl")

for filename in Files:
    outfilename = "summary/" + filename.split("/")[1][:-6] + ".tex"
    os.system("./Utilities/Summary_generator.py -af {0} -o{1}".format(filename,outfilename))