#!/bin/python3
import os
import cups

# print(os.environ.get('USER'))

jobStrList = os.popen('lpstat -W all -u %s | awk \'{print $1}\'' % os.environ.get('USER'))

jobs = []
for jobStr in jobStrList:
    jobStr = jobStr.rstrip()
    jobStr = jobStr[jobStr.rindex('-') + 1 : len(jobStr)]
    # print(jobStr)
    jobs.append(int(jobStr))

c = cups.Connection()
for job in jobs:
    try:
        c.cancelJob(job, False)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)
for job in jobs:
    try:
        c.cancelJob(job, True)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)
for job in jobs:
    try:
        c.cancelJob(job, True)
    except cups.IPPError as e:
        (e, s) = e.args
        # print(e,s)