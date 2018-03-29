//
//  sacla-hdf5-reader.cpp
//  cheetah-ab
//
//  Created by Anton Barty on 7/02/2014.
//  Copyright (c) 2014 Anton Barty. All rights reserved.
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "../include/hdf5-reader.h"


/*
 *  Function for parsing SACLA HDF5 metadata
 */
int HDF5_ReadHeader(const char *filename, h5_info_t *result) {
    
    char    h5field[1024];
	hid_t   dataset_id;
	hid_t   dataspace_id;
	hid_t   datatype_id;
	H5T_class_t dataclass;

    // Does the file exist?
    FILE *fp = fopen(filename, "r");
    if(fp) {
        fclose(fp);
    }
    else {
		printf("ERROR: File does not exist %s\n",filename);
		exit(1);
    }
    
    
    // Open PAL HDF5 file and read in header information
    hid_t   file_id;
	printf("Filename: %s\n", filename);
	file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
	if(file_id < 0){
		printf("ERROR: Could not open HDF5 file %s\n",filename);
		exit(1);
	}
    result->file_id = file_id;
    strcpy(result->filename, filename);

    sprintf(h5field, "/file_info/run_number_list");
	H5LTread_dataset(file_id, h5field, H5T_NATIVE_INT, &result->run_number);
    printf("### /file_info/run_number_list: %i\n", result->run_number);

    sprintf(h5field, "/file_info/version");
    H5LTread_dataset_string(file_id, h5field, result->version);
    printf("### /file_info/version: %s\n", result->version);

    sprintf(h5field, "header/frame_num");
	H5LTread_dataset(file_id, h5field, H5T_NATIVE_INT, &result->nevents);
    printf("### /header/frame_num: %i\n", result->nevents);

    sprintf(h5field, "header/photon_energy_eV");
	H5LTread_dataset(file_id, h5field, H5T_NATIVE_FLOAT, &result->photon_energy_in_eV);
    printf("### /header/photon_energy_eV: %f\n", result->photon_energy_in_eV);

    sprintf(h5field, "header/num_detectors");
	H5LTread_dataset(file_id, h5field, H5T_NATIVE_UINT, &result->ndetectors);
    printf("### header/num_detectors: %i\n", result->ndetectors);

    result->detector_name = (char**) calloc(result->ndetectors, sizeof(char*));
    for(unsigned i=0; i<result->ndetectors;i++){
        sprintf(h5field, "header/detector_%d",i);
        result->detector_name[i] = (char*) calloc(1024, sizeof(char));
        H5LTread_dataset_string(file_id, h5field, result->detector_name[i]);
        printf("### /header/detector_%d: %s\n", i, result->detector_name[i]);
    }

    sprintf(h5field, "header/experimentID");
    H5LTread_dataset_string(file_id, h5field, result->experimentID);
    printf("### /header/experimentID: %s\n", result->experimentID);

    sprintf(h5field, "header/EncoderValue");
    H5LTread_dataset(file_id, h5field, H5T_NATIVE_FLOAT, &result->encoderValue);
    printf("### /header/EncoderValue: %f\n", result->encoderValue);

    return 1;
}


/*
 *  Function for collecting names of the 2D detectors saved in this run
 */
int HDF5_Read2dDetectorFields(h5_info_t *result, long run_index) {

    char    h5field[1024];
    char    tempstr[1024];
    long    counter = 0;


	hid_t   group;
	hsize_t	nfields;
	sprintf(h5field, "%s",result->run_string[run_index]);
	group = H5Gopen( result->file_id, h5field, NULL );


	H5Gget_num_objs(group, &nfields);
	printf("Number of fields in %s: %llu\n", h5field, nfields);
    result->detector_name = (char**) calloc(nfields, sizeof(char*));


    printf("Finding 2D detectors\n");
	for(long i=0; i< nfields; i++) {
		ssize_t r;
		r = H5Gget_objname_by_idx( group, i, tempstr, 1024);
		printf("%li : %s", i, tempstr);

        if(strncmp(tempstr,"detector_2d", 10)) {
            printf("\t(not a 2D detector)\n");
            continue;
        }
        printf("\t(2D detector)\n");
        result->detector_name[counter] = (char*) calloc(1024, sizeof(char));
        strcpy(result->detector_name[counter], tempstr);
        counter++;
	}
	H5Gclose(group);

    result->ndetectors = counter;
    printf("Number of unique 2D detectors: %li\n", result->ndetectors);

    return 1;
}

/*
 *  Function to update event names and photone energy for a run in SACLA HDF5 file
 */
int HDF5_ReadRunInfo(h5_info_t *result, long run_index) {
    char    h5field[1024];
    char    tempstr[1024];
    long    counter = 0;

	hid_t   group;
	hsize_t	nfields;
	sprintf(h5field, "%s/detector_2d_1",result->run_string[run_index]);
	group = H5Gopen(result->file_id, h5field, NULL);

	H5Gget_num_objs(group, &nfields);
	printf("Number of fields in %s: %llu\n", h5field, nfields);
	if (result->event_name != NULL) {
		free(result->event_name);
	}
    result->event_name = (char**) calloc(nfields, sizeof(char*));

    printf("Finding event names\n");
	for(long i=0; i< nfields; i++) {
		ssize_t r;
 		r = H5Gget_objname_by_idx(group, i, tempstr, 1024);
		printf("%li : %s", i, tempstr);

        if(strncmp(tempstr,"tag", 3)) {
            printf("\t(not a new event)\n");
            continue;
        }
        printf("\t(new event)\n");
        result->event_name[counter] = (char*) calloc(1024, sizeof(char));
        strcpy(result->event_name[counter], tempstr);
        counter++;

		herr_t result;
		result = H5LTfind_dataset ( group, tempstr );
		if(result==1) printf("(H5LTfind_dataset=true)\n"); else printf("(H5LTfind_dataset=false)\n");
	}
	H5Gclose(group);

    // get photon energy for all events
	if (result->actual_photon_energy_in_eV != NULL) {
		free(result->actual_photon_energy_in_eV);
	}
    //  TODO: FIXME: Dirty hack! Because some tags are incomplete (i.e., contain
    //  data for some panels alone), len(detector_2d_1) != len(tags) != len(photon_energy).
    //  This happens in runs cancelled midway. + 100 seems enough.
    //  Probably we should look at run_XXX/run_info/tag_number_list.
    result->actual_photon_energy_in_eV = (float*) calloc(nfields + 100, sizeof(float));

	sprintf(h5field, "%s/event_info/bl_3/oh_2",result->run_string[run_index]);
	group = H5Gopen(result->file_id, h5field, NULL );
	H5LTread_dataset_float(group, "photon_energy_in_eV", result->actual_photon_energy_in_eV);
	H5Gclose(group);

    result->nevents = counter;
    printf("Number of unique events: %li\n", result->nevents);

    return 1;
}


/*
 *  Function for reading all <n> 2D detectors into one massive 2D array (for passing to Cheetah or CrystFEL)
 */
int HDF5_ReadImageRaw(h5_info_t *header, long eventID, float *buffer) {

    char    h5field[1024];
    char    h5group[1024];
	hid_t   group;

    printf("Reading event: %d\n",eventID);


    // Open the run group
    sprintf(h5group, "ts-%07d", eventID);
    group = H5Gopen( header->file_id, h5group, NULL );
    if (group<0) {
        printf("%s : H5Gopen failed\n",h5group);
        return -1;
    }

    // Name for this part of the data
    sprintf(h5field, "data");

    // Error check: does this group/field even exist (before we try to read it)?
    herr_t herr;
    herr = H5LTfind_dataset ( group, h5field );
    if(herr!=1) {
        printf("%s/%s : H5LTfind_dataset=false\n",h5group, h5field);
        H5Gclose(group);
        return -1;
    }

    // Read the data set
    H5LTread_dataset_float( group, h5field, buffer);
    H5Gclose(group);


    return 1;
}


/*
 *  Cleanup stale HDF5 references and close the file
 */
int HDF5_cleanup(h5_info_t *header) {

    std::cout << "Cleaning up HDF5 links\n";
	hid_t ids[256];
	long n_ids = H5Fget_obj_ids(header->file_id, H5F_OBJ_ALL, 256, ids);
	for (long i=0; i<n_ids; i++ ) {

		hid_t id;
		H5I_type_t type;

		id = ids[i];
		type = H5Iget_type(id);

		if ( type == H5I_GROUP )
			H5Gclose(id);
		if ( type == H5I_DATASET )
			H5Dclose(id);
		if ( type == H5I_DATASPACE )
			H5Sclose(id);
		if ( type == H5I_DATATYPE )
			H5Dclose(id);
	}

	H5Fclose(header->file_id);

    return 1;
}

