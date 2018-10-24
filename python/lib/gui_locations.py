# -*- coding: utf-8 -*-
#
#	CFEL image handling tools
#	Anton Barty
#

import socket

#
#   Determine where we are running the Cheetah GUI
#   Useful for setting default directory paths, batch queue commands, etc
#
def determine_location():

    print("Determining location:")
    hostname = socket.getfqdn()
    print("Hostname: ", hostname)



    #
    #   Determine where we are
    #   Enables location configuration to be overridden separately from what is determined here, eg: from command line
    #
    if hostname.endswith("slac.stanford.edu"):
        print("Looks like we are at SLAC.")
        location = 'LCLS'

    elif hostname.endswith("pcdsn"):
        print("Looks like we are on psana at LCLS.")
        location = 'LCLS'

    elif hostname.startswith('max') and hostname.endswith("desy.de"):
        print("Looks like we are on a max-cfel node.")
        location = 'max-cfel'

    elif hostname.endswith("desy.de"):
        print("Looks like we are at CFEL/DESY.")
        location = 'DESY'

    elif hostname.endswith("xfel.eu"):
        print("Looks like we are at euXFEL.")
        location = 'euXFEL'

    elif hostname.endswith("xfel.pal"):
        print("Looks like we are at PAL.")
        location = 'PAL'

    elif hostname.startswith("ur1p") and hostname.endswith("4thbl"):
        print("Looks like we are at PAL.")
        location = 'PAL'

    elif hostname.endswith("sdfarm.kr"):
        print("Looks like we are at KISTI.")
        location = 'KISTI'

    else:
        print("Unable to determine location from hostname")
        location = 'None'

    return location




#
#   Set location specific default directory paths, batch queue commands, etc
#
def set_location_configuration(location="Default"):

    print("Setting location as ", location)
    #
    #   Location specific configurations, as needed
    #
    result = {}

    if  location=='LCLS':
        LCLS = {
            'qcommand' : 'bsub -q psanaq'
        }
        result.update(LCLS)

    elif  location=='CFEL':
        CFEL = {
            'qcommand' : 'slurm'
        }
        result.update(CFEL)

    elif  location=='euXFEL':
        euXFEL = {
            'qcommand' : 'bsub'
        }
        result.update(euXFEL)

    if  location=='PAL':
        PAL = {
            'qcommand' : ' '
        }
        result.update(PAL)

    if  location=='KISTI':
        KISTI = {
            'qcommand' : ' '
        }
        result.update(KISTI)

    else:
        default = {
            'qcommand' : ' '
        }
        result.update(default)


    # Add location to the dictionary for completeness
    result.update({'location' : location})

    # Return
    return result


