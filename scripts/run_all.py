#!/bin/python3

#################################
#   RUN FROM ROOT DIR: spectify #
#################################

import shlex
import subprocess as sub
import os
from os import path
import sys
import re
from datetime import datetime

DBG = False
PROG_REPETITIONS = 9
stdout_backup = sys.stdout

dirs  = ["uninitialized_read","bounds_check_bypass", "buffer_overflow", "use_after_free",  "pthread_lock", "semaphore"]
progs = ["uninit_read","bcb", "overflow", "use_after_free", "lock", "semaphore"]

if not path.exists("data"):
    print("created data directory")
    os.mkdir("data")

date_time = datetime.today()
timelabel = date_time.strftime("%Y-%m-%d_%H;%M;%S")
filepath = f"../data/measurement_{timelabel}.txt"

#avg for average
def avg(lst):
    return sum(lst) / len(lst)

print("starting first program...")
for prog_index in range(0, len(progs)):
    dir = dirs[prog_index]
    prog = progs[prog_index]

    if not path.exists(dir):
        print(f"ERROR: Missing directory {dir}")
        exit(1)

    os.chdir(dir)

    if not path.exists(f"{prog}.c"):
        print(f"ERROR: Missing directory {prog}")
        exit(1)

    repetitions_list                = [0   for x in range(PROG_REPETITIONS)]
    secret_list                     = [0   for x in range(PROG_REPETITIONS)]
    secret_size_list                = [0   for x in range(PROG_REPETITIONS)]
    measured_time_list              = [0.0 for x in range(PROG_REPETITIONS)]
    total_bytes_list                = [0   for x in range(PROG_REPETITIONS)]
    median_accuracy_list            = [0   for x in range(PROG_REPETITIONS)]
    total_accuracy_list             = [0   for x in range(PROG_REPETITIONS)]
    median_accuracy_percentage_list = [0.0 for x in range(PROG_REPETITIONS)]
    total_accuracy_percentage_list  = [0.0 for x in range(PROG_REPETITIONS)]
    bytes_per_second_list           = [0.0 for x in range(PROG_REPETITIONS)]
    leaked_bytes_per_second_list    = [0.0 for x in range(PROG_REPETITIONS)]

    for prog_repetition in range(0, PROG_REPETITIONS):
        print(f"{prog} repetition {prog_repetition}")

        command = f"make clean && make && ./{prog}"
        #command = f"make clean && make && gdb {prog} -ex 'r' -ex 'q'"   #run in GDB, prevents uninit_read crashes
        output = sub.check_output(command, shell=True).decode("utf-8")
        # print(output)

        repetitions_list[prog_repetition]               = int(  re.search('.*grep_repetitions.*'               , output).group(0).partition(' ')[2])
        secret_list[prog_repetition]                    =       re.search('.*grep_secret.*'                    , output).group(0).partition(' ')[2]
        secret_size_list[prog_repetition]               = int(  re.search('.*grep_secret_size.*'               , output).group(0).partition(' ')[2])
        measured_time_list[prog_repetition]             = float(re.search('.*grep_measured_time.*'             , output).group(0).partition(' ')[2])
        total_bytes_list[prog_repetition]               = int(  re.search('.*grep_total_bytes.*'               , output).group(0).partition(' ')[2])
        median_accuracy_list[prog_repetition]           = int(  re.search('.*grep_median_accuracy.*'           , output).group(0).partition(' ')[2])
        total_accuracy_list[prog_repetition]            = int(  re.search('.*grep_total_accuracy.*'            , output).group(0).partition(' ')[2])
        median_accuracy_percentage_list[prog_repetition]= float(re.search('.*grep_median_accuracy_percentage.*', output).group(0).partition(' ')[2])
        total_accuracy_percentage_list[prog_repetition] = float(re.search('.*grep_total_accuracy_percentage.*' , output).group(0).partition(' ')[2])
        bytes_per_second_list[prog_repetition]          = float(re.search('.*grep_bytes_per_second.*'          , output).group(0).partition(' ')[2])
        leaked_bytes_per_second_list[prog_repetition]   = float(re.search('.*grep_leaked_bytes_per_second.*'   , output).group(0).partition(' ')[2])

        if DBG:
            print(f"{prog} run {prog_repetition}:")
            print(f"    repetitions                     = {repetitions_list[prog_repetition]}")
            print(f"    secret                          = {secret_list[prog_repetition]}")
            print(f"    secret_size                     = {secret_size_list[prog_repetition]}")
            print(f"    measured_time                   = {measured_time_list[prog_repetition]}")
            print(f"    total_bytes                     = {total_bytes_list[prog_repetition]}")
            print(f"    median_accuracy                 = {median_accuracy_list[prog_repetition]}")
            print(f"    total_accuracy                  = {total_accuracy_list[prog_repetition]}")
            print(f"    median_accuracy_percentage      = {median_accuracy_percentage_list[prog_repetition]}")
            print(f"    total_accuracy_percentage       = {total_accuracy_percentage_list[prog_repetition]}")
            print(f"    bytes_per_second                = {bytes_per_second_list[prog_repetition]}")
            print(f"    leaked_bytes_per_second         = {leaked_bytes_per_second_list[prog_repetition]}")


    avg_measured_time               = avg(measured_time_list)
    sum_measured_time               = sum(measured_time_list)
    sum_total_bytes                 = sum(total_bytes_list)
    avg_median_accuracy             = avg(median_accuracy_list)
    sum_total_accuracy              = sum(total_accuracy_list)
    avg_total_accuracy              = avg(total_accuracy_list)
    avg_median_accuracy_percentage  = avg(median_accuracy_percentage_list)
    avg_total_accuracy_percentage   = avg(total_accuracy_percentage_list)
    avg_bytes_per_second            = avg(bytes_per_second_list)
    avg_leaked_bytes_per_second     = avg(leaked_bytes_per_second_list)

    file = open(filepath, 'a')

    for i in range(0, 2):
        print(f"\nResults of repeating {prog} {PROG_REPETITIONS} times:")
        print(f"    secret                          = {secret_list[0]}")
        print(f"    secret_size                     = {secret_size_list[0]}")
        print(f"    repetitions                     = {repetitions_list[0]}")
        print(f"    avg_measured_time               = {round(avg_measured_time,6)}\t\tseconds")
        print(f"    sum_measured_time               = {round(sum_measured_time,6)}\t\tseconds")
        print(f"    sum_total_bytes                 = {sum_total_bytes}\t\tbytes attempted to leak")
        print(f"    avg_median_accuracy             = {round(avg_median_accuracy,6)}\t\taverage result accuracy")
        print(f"    sum_total_accuracy              = {sum_total_accuracy}\t\tbytes leaked in total")
        print(f"    avg_total_accuracy              = {round(avg_total_accuracy,6)}\tbytes leaked on average/run")
        print(f"    avg_median_accuracy_percentage  = {round(avg_median_accuracy_percentage,6)}%\t\tcorrect result")
        print(f"    avg_total_accuracy_percentage   = {round(avg_total_accuracy_percentage,6)}%\taccuracy of all attempts")
        print(f"    avg_bytes_per_second            = {round(avg_bytes_per_second,6)}\tbytes per second")
        print(f"    avg_leaked_bytes_per_second     = {round(avg_leaked_bytes_per_second,6)}\tcorrectly leaked bytes/second")
        print(f"                                                        (leakage rate)")
        print("\n\n")
        sys.stdout = file

    file.close()
    sys.stdout = stdout_backup


    os.chdir("..")












