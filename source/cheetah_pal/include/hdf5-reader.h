//
//  sacla-hdf5-reader.h
//  cheetah-ab
//
//  Created by Anton Barty on 7/02/2014.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//

#ifndef __hdf5_reader__
#define __hdf5_reader__

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>


/*
 *  Structure for holding SACLA HDF5 metadata
 */
typedef struct {
    // File info
    char    filename[1024];
    hid_t   file_id;

    // Run information
    char    experimentID[1024];
    int     nruns;
    char    **run_string;
    int64_t run_number;
    int64_t *start_tag_number;
    int64_t *end_tag_number;
    float   photon_energy_in_eV;
    float   photon_wavelength_in_nm;

    // Device objects within a run
    unsigned    ndetectors;
    char    **detector_name;
    float encoderValue;
    float detectorPosition;

    // Detector event tags within a run
    unsigned    nevents;
    char    **event_name;
    float   *actual_photon_energy_in_eV; // actual values of each shot

    // PAL hdf5 schema version
    char version[1024];
} h5_info_t;


/*
 *  Prototypes for functions written to read SACLA HDF5 data
 */
int HDF5_ReadHeader(const char*, h5_info_t*);
int HDF5_Read2dDetectorFields(h5_info_t*, long);
//int HDF5_ReadEventTags(h5_info_t*, long);
int HDF5_ReadRunInfo(h5_info_t*, long);
int HDF5_ReadImageRaw(h5_info_t*, long, float*);
int HDF5_cleanup(h5_info_t*);



#endif /* defined(__hdf5_reader__) */
