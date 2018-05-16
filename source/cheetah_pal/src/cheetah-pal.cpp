//
//  cheetah-pal.cpp
//
//  Originally created by Anton Barty on 20/1/14.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//
//  Created by Chun Hong Yoon on 21/3/18.
//  Copyright (c) 2018 Chun Hong Yoon. All rights reserved.
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "cheetah.h"
#include "hdf5-reader.h"

int main(int argc, const char * argv[])
{
    if (argc != 3) {
        printf("Usage: %s pal.h5 cheetah.ini\n", argv[0]);
        return -1;
    }

    static time_t startT = 0;
    time(&startT);

    // PAL hdf5 input file and Cheetah configuration file
    char fname[1024];
    char cheetahini[1024];

    // Take configuration from command line arguments
    strcpy(fname, argv[1]);
    strcpy(cheetahini, argv[2]);

    /*
     *	Initialise Cheetah
     */
    printf("Setting up Cheetah...\n");
    //static uint32_t ntriggers = 0;
    //static long frameNumber = 0;
    //long    runNumber = 0;
    static cGlobal cheetahGlobal;
    strcpy(cheetahGlobal.configFile, cheetahini);
    sprintf(cheetahGlobal.facility, "PAL");
    cheetahInit(&cheetahGlobal);
    printf("Done cheetahInit\n");

    /*
     *	Open PAL HDF5 file
     *	Read file header and information
     */
    printf("Start HDF5_ReadHeader: %s\n", fname);
    h5_info_t header;
    HDF5_ReadHeader(fname, &header);
    // Gather detector fields and event tags for this run
    printf("Done HDF5_ReadHeader\n");
    //HDF5_Read2dDetectorFields(&header, 0);
    //printf("start readRunInfo\n");
    //HDF5_ReadRunInfo(&header, 0);

    /******************************************
     * Copy header information to cheetahGlobal
     ******************************************/
    sprintf(cheetahGlobal.experimentID, header.experimentID);
    cheetahGlobal.runNumber = header.run_number;

    /*
     * Create a buffer for holding the detector image data
     */
    uint16_t   *buffer = (uint16_t*) calloc(cheetahGlobal.detector[0].pix_nn, sizeof(uint16_t));
    hsize_t dims[2];
    dims[0] = cheetahGlobal.detector[0].asic_ny; // ss
    dims[1] = cheetahGlobal.detector[0].asic_nx; // fs

    time_t	tnow;
    double	dtime, datarate;
    long    detID = 0;

    // Loop through all events found in this run

    for(long eventID=0; eventID<header.nevents; eventID++) {
        /*
         *  Cheetah: Calculate time beteeen processing of data frames
         */
        time(&tnow);
        dtime = difftime(tnow, cheetahGlobal.tlast);
        if(dtime > 1.) {
            datarate = (eventID - cheetahGlobal.lastTimingFrame)/dtime;
            cheetahGlobal.lastTimingFrame = eventID;
            time(&cheetahGlobal.tlast);
            cheetahGlobal.datarate = datarate;
        }

        /*
         *	Read next image
         */
        if (HDF5_ReadImageRaw(&header, eventID, buffer) < 0) continue;

        /*
         *	Cheetah: Create a new eventData structure in which to place all information
         */
        cEventData	*eventData;
        eventData = cheetahNewEvent(&cheetahGlobal);

        /*
         *  Cheetah: Populate event structure with meta-data
         */
        sprintf(eventData->filename, fname);
        eventData->frameNumber = eventID;
        eventData->runNumber = header.run_number; // TODO: get from input.h5
        eventData->nPeaks = 0;
        eventData->pumpLaserCode = header.pumpLaserCode[eventID];
        eventData->pumpLaserDelay = (double) header.pumpLaserDelay[eventID];
        eventData->photonEnergyeV = header.photon_energy_keV[eventID]*1000; // TODO: get from input.h5
        eventData->wavelengthA = 12398 / eventData->photonEnergyeV; // 4.1357E-15 * 2.9979E8 * 1E10 / eV (A)
        eventData->pGlobal = &cheetahGlobal;
        eventData->fiducial = eventID; // must be unique
        sprintf(eventData->eventname, "%07d",eventID);
        //eventData->detectorDistance = header.detectorPosition;
        eventData->detector[detID].detectorZ = header.encoderValue[detID];

        /*
         *  Cheetah: Copy image data into
         *  Raw data is currently hard-coded as UINT16_t, SACLA provides as float, so we have to loose precision :-(
         */
        for(long ii=0; ii<cheetahGlobal.detector[detID].pix_nn; ii++) {
            eventData->detector[detID].data_raw16[ii] = buffer[ii];
        }

        /*
         *	Cheetah: Process this event
         */
        cheetahProcessEventMultithreaded(&cheetahGlobal, eventData);
    }

    // Clean up stale IDs and exit
    HDF5_cleanup(&header);

    time_t endT;
    time(&endT);
    double diff = difftime(endT,startT);
    std::cout << "time taken: " << diff << " second(s)\n";

    /*
     *	Cheetah: Cleanly exit by closing all files, releasing memory, etc.
     */
    cheetahExit(&cheetahGlobal);

    return 0;
}

