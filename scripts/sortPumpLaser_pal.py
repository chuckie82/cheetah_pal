import numpy as np
import h5py
import argparse
parser = argparse.ArgumentParser()
parser.add_argument("fname", help="Full path to Cheetah .cxi filename", type=str)
args = parser.parse_args()

fname =	args.fname
print("fname: ", fname)
onList = fname.split('.')[0]+'-laserOn.lst'
offList = fname.split('.')[0]+'-laserOff.lst'

f = h5py.File(fname, "r")
laserOn = f["/instrument/pump_laser_on"].value
f.close()
print("Writing laser on list: ", onList)
print("Writing laser off list: ", offList)
with open(onList, "w") as on_file:
    with open(offList, "w") as off_file:
        for i, val in enumerate(laserOn):
            if val == 1: 
                on_file.write("{} //{}\n".format(fname, i))
            elif val == 0:
                off_file.write("{} //{}\n".format(fname, i))
