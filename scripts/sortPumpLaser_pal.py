import numpy as np
import h5py

fname = "ue_180424_SFX1-r0123-c00.cxi"
onList = fname.split('.')[0]+'-laserOn.lst'
offList = fname.split('.')[0]+'-laserOff.lst'

f = h5py.File(fname, "r")
laserOn = f["/instrument/pump_laser_on"].value
f.close()

with open(onList, "w") as on_file:
    with open(offList, "w") as off_file:
        for i, val in enumerate(laserOn):
            if val == 1: 
                on_file.write("{} //{}\n".format(fname, i))
            elif val == 0:
                off_file.write("{} //{}\n".format(fname, i))
