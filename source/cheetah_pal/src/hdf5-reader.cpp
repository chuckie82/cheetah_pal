//
//  hdf5-reader.cpp
//
//

#include <iostream>
#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cstddef>

#include "../include/hdf5-reader.h"

unsigned getRunNumber(const std::string& str)
{
  std::size_t found = str.find_last_of("/\\");
  std::size_t places = 7;
  char runNum[1024];
  strncpy( runNum, str.substr(found+1).c_str(), places );
  return atoi(runNum);
}

/*
 *  Function for parsing PAL HDF5 metadata
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
    } else {
        printf("ERROR: File does not exist %s\n",filename);
        exit(1);
    }

    // Open PAL HDF5 file and read in header information
    hid_t   file_id;
    file_id = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
    if(file_id < 0){
        printf("ERROR: Could not open HDF5 file %s\n",filename);
        exit(1);
    }
    result->file_id = file_id;
    strcpy(result->filename, filename);

    // Get run number
    result->run_number = getRunNumber(filename);

    sprintf(h5field, "/R%04d/header/file_version", result->run_number);
    herr_t      status;
    status = H5LTread_dataset_string(file_id, h5field, result->version);

    // Get number of events from length of photon energy array
    sprintf(h5field, "/R%04d/scan_dat/photon_energy", result->run_number);
    hid_t dataset = H5Dopen2(file_id, h5field, H5P_DEFAULT);
    /*
     * Get datatype and dataspace handles and then query
     * dataset class, order, size, rank and dimensions.
     */
    hid_t datatype  = H5Dget_type(dataset);     /* datatype handle */
    H5T_class_t t_class     = H5Tget_class(datatype);
    if (t_class == H5T_FLOAT) printf("Data set has FLOAT type \n");
    H5T_order_t order     = H5Tget_order(datatype);
    if (order == H5T_ORDER_LE) printf("Little endian order \n");
    size_t size  = H5Tget_size(datatype);
    hsize_t dims_out[2];
    hid_t dataspace = H5Dget_space(dataset);    /* dataspace handle */
    int rank      = H5Sget_simple_extent_ndims(dataspace);
    int status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
    printf("rank %d, dimensions %lu \n", rank,
           (unsigned long)(dims_out[0]));
    result->nevents = dims_out[0];

    // Get list of photon energies
    // FIXME: seems to cause bus error
    result->photon_energy_keV = (float*) calloc( result->nevents, sizeof(float));
    sprintf(h5field, "/R%04d/scan_dat/photon_energy", result->run_number);
    H5LTread_dataset_float(file_id, h5field, result->photon_energy_keV);

    // Get list of pump laser code
    result->pumpLaserCode = (int*) calloc( result->nevents, sizeof(int) );
    sprintf(h5field, "/R%04d/scan_dat/laser_on", result->run_number);
    H5LTread_dataset_int(file_id, h5field, result->pumpLaserCode);

    // Get list of pump laser delay
    result->pumpLaserDelay = (float*) calloc( result->nevents, sizeof(float) );
    sprintf(h5field, "/R%04d/scan_dat/laser_delay", result->run_number);
    H5LTread_dataset_float(file_id, h5field, result->pumpLaserDelay);

    // Get number of detectors
    sprintf(h5field, "/R%04d/header/detector_num", result->run_number);
    H5LTread_dataset(file_id, h5field, H5T_NATIVE_UINT, &result->ndetectors);

    // Get detector names
    // FIXME
    result->detector_name = (char**) calloc(result->ndetectors, sizeof(char*));
    for(unsigned i=0; i<result->ndetectors; i++){
        sprintf(h5field, "/R%04d/header/detector_%d_name",result->run_number, i);
        result->detector_name[i] = (char*) calloc(1024, sizeof(char));
        H5LTread_dataset_string(file_id, h5field, result->detector_name[i]);
    }

    // Get experiment ID
    // FIXME
    sprintf(h5field, "/R%04d/header/exp_id", result->run_number);
    H5LTread_dataset_string(file_id, h5field, result->experimentID);

    for(unsigned i=0; i<result->ndetectors;i++){
        sprintf(h5field, "/R%04d/header/detector_%d_distance", result->run_number, i);
        H5LTread_dataset(file_id, h5field, H5T_NATIVE_FLOAT, &result->encoderValue[i]);
    }

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
int HDF5_ReadImageRaw(h5_info_t *header, long eventID, uint16_t *buffer) {

    hid_t	dataset;         /* handles */
    hid_t	datatype, dataspace;
    hid_t	memspace;
    H5T_class_t t_class;                 /* data type class */
    H5T_order_t order;                 /* data order */
    size_t	size;                  /*
                                        * size of the data element
                                        * stored in file
                                        */
    hsize_t     dimsm[3];              /* memory space dimensions */
    hsize_t     dims_out[3];           /* dataset dimensions */

    hsize_t	 count[3];              /* size of the hyperslab in the file */
    hsize_t	 offset[3];             /* hyperslab offset in the file */
    hsize_t	 count_out[3];          /* size of the hyperslab in memory */
    hsize_t	 offset_out[3];         /* hyperslab offset in memory */
    int          status, status_n, rank;
    int RANK_OUT = 3;

    // FIXME: how to figure out which detector gets used for hit finding
    char    h5field[1024];
    sprintf(h5field, "/R%04d/scan_dat/%s_data", header->run_number, header->detector_name[0]);
    dataset = H5Dopen2( header->file_id, h5field, H5P_DEFAULT);

    /*
     * Get datatype and dataspace handles and then query
     * dataset class, order, size, rank and dimensions.
     */
    datatype  = H5Dget_type(dataset);     /* datatype handle */
    t_class     = H5Tget_class(datatype);
    if (t_class == H5T_NATIVE_USHORT) printf("Data set has UINT16 type \n");
    order     = H5Tget_order(datatype);
    if (order == H5T_ORDER_LE) printf("Little endian order \n");

    size  = H5Tget_size(datatype);
    dataspace = H5Dget_space(dataset);    /* dataspace handle */
    rank      = H5Sget_simple_extent_ndims(dataspace);
    status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);

    /*
     * Define hyperslab in the dataset.
     */
    offset[0] = eventID;
    offset[1] = 0;
    offset[2] = 0;
    count[0]  = 1;
    count[1]  = dims_out[1];
    count[2]  = dims_out[2];
    status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL,
                                 count, NULL);

    /*
     * Define the memory dataspace.
     */
    dimsm[0] = dims_out[0];
    dimsm[1] = dims_out[1];
    dimsm[2] = dims_out[2];
    memspace = H5Screate_simple(RANK_OUT,dimsm,NULL);

    /*
     * Define memory hyperslab.
     */
    offset_out[0] = 0;
    offset_out[1] = 0;
    offset_out[2] = 0;
    count_out[0]  = 1;
    count_out[1]  = dims_out[1];
    count_out[2]  = dims_out[2];
    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL,
                                 count_out, NULL);

    /*
     * Read data from hyperslab in the file into the hyperslab in
     * memory and display.
     */
    status = H5Dread(dataset, H5T_NATIVE_USHORT, memspace, dataspace,
                     H5P_DEFAULT, buffer);

    /*
     * Close/release resources.
     */
    H5Tclose(datatype);
    H5Dclose(dataset);
    H5Sclose(dataspace);
    H5Sclose(memspace);

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

