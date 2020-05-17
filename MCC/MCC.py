#!/usr/bin/env python3

import os
import subprocess

def getCnfFileNames():
    return sorted(list(os.walk('wcnf'))[0][2])

def getAddmcCommand():
    command = '../addmc'
    command = 'timeout 1s {}'.format(command)
    return command

def runAddmcCommands():
    os.makedirs('wmc', exist_ok=True)
    successes = []
    failures = []
    for cnfFileName in getCnfFileNames()[:]:
        print(cnfFileName)
        cnfFile = open('wcnf/{}'.format(cnfFileName))
        mcFile = open('wmc/{}'.format(cnfFileName), 'w')
        try:
            command = getAddmcCommand()
            subprocess.check_call(command.split(), stdin=cnfFile, stdout=mcFile)
            successes.append(cnfFileName)
        except subprocess.CalledProcessError:
            failures.append(cnfFileName)
    print('\n{} successes\n\t{}'.format(len(successes), ' '.join(successes)))
    print('\n{} failures\n\t{}'.format(len(failures), ' '.join(failures)))

if __name__ == '__main__':
    runAddmcCommands()
