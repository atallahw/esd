/* for file and terminal I/O */
#include <stdio.h>
/* for string manip */
#include <string.h>
/* for exit() */
#include <stdlib.h>
/* for fabsf() */
#include <math.h>

#define BUFF_SIZE 1024

/*
 * sets first <n> values in <*arr> to <val>
 */
void clear_buffer(float *arr, float val, int n) 
{
	int i;
	for (i = 0; i < n; i++) {
		arr[i] = val;
	}
}

/*
 * Caculates mean of first <n> samples in <*arr>
 */
float calculate_mean(float *arr, int n)
{
	float total;
	int i;

	total = 0.0f;
	for (i = 0; i < n; i++) {
		total += arr[i];
	}

	return total/((float) n);
}

int 
find_peaks_and_troughs(
		float *arr, 	// signal 
		int n_samples, 	// number of samples present in the signal
		float E, 	// threshold for peak detection
		// arrays that will store the indicies of the located
		// peaks and troughs
		float *P, float *T,
		// number of peaks (n_P) and number of troughs (n_T)
		// found in the data set *arr
		int *n_P, int *n_T
		)
{
	int a, b, i, d, _n_P, _n_T;

	i = -1; d = 0; a = 0; b = 0;
	_n_P = 0; _n_T = 0;

	clear_buffer(P, 0.0f, n_samples);
	clear_buffer(T, 0.0f, n_samples);

	while (i != n_samples) {
		i++;
		if (d == 0) {
			if (arr[a] >= (arr[i] + E)) {
				d = 2;
			} else if (arr[i] >= (arr[b] + E)) {
				d = 1;
			}
			if (arr[a] <= arr[i]) {
				a = i;
			} else if (arr[i] <= arr[b]) {
				b = i;
			}
		} else if (d == 1) {
			if (arr[a] <= arr[i]) {
				a = i;
			} else if (arr[a] >= (arr[i] + E)) {
				/*
				 * Peak has been detected.
				 * Add index at detected peak
				 * to array of peak indicies
				 * increment count of peak indicies
				 */
				P[_n_P] = a;
				_n_P++;
				b = i;
				d = 2;
			}
		} else if (d == 2) {
			if (arr[i] <= arr[b]) {
				b = i;
			} else if (arr[i] >= (arr[b] + E)) {
				/*
				 * Trough has been detected.
				 * Add index at detected trough
				 * to array of trough indicies
				 * increment count of trough indicies
				 */
				T[_n_T] = b;
				_n_T++;
				a = i;
				d = 1;
			}
		}
	}

	(*n_P) = _n_P;
	(*n_T) = _n_T;
	return 0;
}

int main(int argc, char **argv)
{
	/* Generic variables */
	int i, idx;
	int rv;
	/* Variables for reading file line by line */
	char *ifile_name, *ofile_pt_name, *ofile_st_name;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int N_SAMPLES;

	/* Variables for storing the data and storing the return values */
	double *t,*t_b, *t_a; 
	float *x_ac, *y_ac, *z_ac, *x_gy, *y_gy, *z_gy; 	// variables for data collected from input file
	float pk_threshold;	// pk-threshold value
       	/* Variables for peak-trough detection */	
	float *P_i; 	// indicies of each peak found by peak detection
	float *T_i; 	// indicies of each trough found by trough detection
	float *S_i; 	// indicies of the start of each stride
	int n_P; 	// number of peaks
	int n_T; 	// number of troughs
	int n_S; 	// number of strides

	/*
	 * set this to 0 so that the code can function without
	 * having to actually performing stride detection
	 * from peaks and troughs
	 */
	n_S = 0; 
	
	/*
	 * Check if the user entered the correct command line arguments
	 * Usage: 
	 * ./extract_stride_data <ifile_name> <output_peaks> <output_strides>
	 * 				<threshold_value_float>
	 * Or 
	 * ./extract_stride_data
	 */
	if (argc != 5) {
		ifile_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
		memset(ifile_name, 0, BUFF_SIZE);
		snprintf(ifile_name, 
				BUFF_SIZE, 
				"data.csv"
			);
		ofile_pt_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
		memset(ofile_pt_name, 0, BUFF_SIZE);
		snprintf(ofile_pt_name, BUFF_SIZE, "acceleration_output.csv");
		ofile_st_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
		memset(ofile_st_name, 0, BUFF_SIZE);
		snprintf(ofile_st_name, BUFF_SIZE, "acceleration_strides.csv");
		pk_threshold = 6.7;
	} else {
		ifile_name = argv[1];
		ofile_pt_name = argv[2];
		ofile_st_name = argv[3];
		pk_threshold = atof(argv[4]);
	}

	printf("Arguments used:\n\t%s=%s\n\t%s=%s\n\t%s=%s\n\t%s=%f\n",
			"ifile_name", ifile_name,
			"ofile_peak_trough_name", ofile_pt_name,
			"ofile_stride_name", ofile_st_name,
			"peak_threshold", pk_threshold
	      );

	/* open the input file */
	printf("Attempting to read from file \'%s\'.\n", ifile_name);
	fp = fopen(ifile_name, "r");
	if (fp == NULL) {
		fprintf(stderr, 
				"Failed to read from file \'%s\'.\n", 
				ifile_name
		       );
		exit(EXIT_FAILURE);
	}

	/* count the number of lines in the file */
	read = getline(&line, &len, fp); //discard header of file
	N_SAMPLES = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		N_SAMPLES++;
	}

	/* go back to the start of the file so that the data can be read */
	rewind(fp);
	read = getline(&line, &len, fp); //discard header of file

	/* start reading the data from the file into the data structures */
	i = 0;
	t =   (double *) malloc(sizeof(double) * N_SAMPLES);
	t_b = (double *) malloc(sizeof(double) * N_SAMPLES);
	t_a = (double *) malloc(sizeof(double) * N_SAMPLES);
	x_ac = (float *) malloc(sizeof(float) * N_SAMPLES);
	y_ac = (float *) malloc(sizeof(float) * N_SAMPLES);
	z_ac = (float *) malloc(sizeof(float) * N_SAMPLES);
	x_gy = (float *) malloc(sizeof(float) * N_SAMPLES);
	y_gy = (float *) malloc(sizeof(float) * N_SAMPLES);
	z_gy = (float *) malloc(sizeof(float) * N_SAMPLES);
	
	while ((read = getline(&line, &len, fp)) != -1) {
		/* parse the data */
		rv = sscanf(line, "%lf,%lf,%f,%f,%f,%f,%f,%f\n", &t_b[i], &t_a[i], &x_ac[i], &y_ac[i],&z_ac[i],&x_gy[i], &y_gy[i], &z_gy[i]);
		if (rv != 8) {
			fprintf(stderr,
					"%s %d \'%s\'. %s.\n",
					"Failed to read line",
					i,
					line,
					"Exiting"
			       );
			exit(EXIT_FAILURE);
		}
		i++;
	}
	fclose(fp);

	/* 
	 * From selected thresholds, 
	 * find indicies of peaks
	 * find indicies of troughs
	 */
	int ii;
	for ( ii = 0; ii < N_SAMPLES; ii++)
	{
		t[ii] = (t_b[ii] + t_a[ii])/2.0;


	}
	P_i = (float *) malloc(sizeof(float) * N_SAMPLES);
	T_i = (float *) malloc(sizeof(float) * N_SAMPLES);
	rv = find_peaks_and_troughs(
			x_ac,                               //x
			N_SAMPLES, 
			pk_threshold, 
			P_i, T_i, 
			&n_P, &n_T);
	if (rv < 0) {
		fprintf(stderr, "find_peaks_and_troughs failed\n");
		exit(EXIT_FAILURE);
	}

	/* DO NOT MODIFY ANYTHING BEFORE THIS LINE */

	/* 
	 * Insert your algorithm to convert from a series of peak-trough
	 * indicies, to a series of indicies that indicate the start
	 * of a stride.
	 */
	S_i = (float *) malloc(sizeof(float) * n_P);
	float *S_min;
	S_min = (float *) malloc(sizeof(float) * n_T);    //for min (troughs)

        int a , b , c;
	a = 0; b = 1; n_S = 0; c = 0; 
	while( b < n_P)
	{
		if((P_i[b] - P_i[a]) > 300)
		{
			S_i[n_S] = P_i[a];
		        S_min[n_S] = T_i[a];	  // troughs
			n_S++;
			a=a+c+1;              
			b=b+1;
			c=0;
			/*checking for boundary conditions: if the last stride*/ 
			if((b == n_P) && ((P_i[b-1] - P_i[a-1])>300))
		        {
				S_i[n_S] = P_i[b-1];	
				S_min[n_S] = T_i[b-1];
				n_S++;
			}	
		}
		else{
			b++;
			c++;
                        /*checking for boundary conditions: if the last stride*/
			if((b == n_P) && ((P_i[a] - P_i[a-1])>300))
			{		
				S_i[n_S] = P_i[a];
                        	S_min[n_S] = T_i[a];
				n_S++;
			}	
		}
	
	}

	
	/* DO NOT MODIFY ANYTHING AFTER THIS LINE */

	/* open the output file to write the peak and trough data */
	printf("Attempting to write to file \'%s\'.\n", ofile_pt_name);
	fp = fopen(ofile_pt_name, "w");
	if (fp == NULL) {
		fprintf(stderr, 
				"Failed to write to file \'%s\'.\n", 
				ofile_pt_name
		       );
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "P_i,P_t,P_x,T_i,T_t,T_p\n");            //x
	for (i = 0; i < n_P || i < n_T; i++) {
		/* Only peak data if there is peak data to write */
		if (i < n_P) {
			idx = (int) P_i[i];
			fprintf(fp, "%d,%20.10lf,%lf,",
					idx,
					t[idx],
					x_ac[idx]               //x
			       );
		} else {
			fprintf(fp, ",,,");
		}
		/* Only trough data if there is trough data to write */
		if (i < n_T) {
			idx = (int) T_i[i];
			fprintf(fp, "%d,%20.10lf,%lf\n",
					idx,
					t[idx],
					x_ac[idx]                //x
			       );
		} else {
			fprintf(fp, ",,\n");
		}
	}
	fclose(fp);

	/* open the output file to write the stride data */
	printf("Attempting to write to file \'%s\'.\n", ofile_st_name);
	fp = fopen(ofile_st_name, "w");
	if (fp == NULL) {
		fprintf(stderr, 
				"Failed to write to file \'%s\'.\n", 
				ofile_st_name
		       );
		exit(EXIT_FAILURE);
	}
        int idx_min;  
	double period;
	int idx_next;
	fprintf(fp, "S_i,S_t,S_x,S_i,S_t,S_min,period\n");           //x
	for (i = 0; i < n_S; i++) {
		idx = (int) S_i[i];
		idx_min = (int) S_min[i];
		idx_next = (int) S_i[i+1];
		((i+1)!=n_S)? period = t[idx_next]- t[idx]: period;
		fprintf(fp, "%d,%20.10lf,%f,%d,%20.10lf,%f,%lf\n",
				idx,
				t[idx],
				x_ac[idx],			//x
				idx_min,
				t[idx_min],
                                x_ac[idx_min],                  //x
				period
		       );
	}
	fclose(fp);

	char * train_file_name = "train_set.txt";
        /* open the training file for the neutal network */
	printf("Attempting to write to file \'%s\'.\n", train_file_name);
	fp = fopen(train_file_name, "a");
	if (fp == NULL) {
		fprintf(stderr, 
				"Failed to write to file \'%s\'.\n", 
				train_file_name
		       );
		exit(EXIT_FAILURE);
	}
        
	for (i = 0; i < n_S; i++) {
		idx = (int) S_i[i];
		idx_min = (int) S_min[i];
		idx_next = (int) S_i[i+1];
		((i+1)!=n_S)? period = t[idx_next]- t[idx]: period;
		fprintf(fp, "%lf %f %f\n",
				period/10.0,
				x_ac[idx]/10.0,			//x
				x_ac[idx_min]/10.0              //x
				);
		fprintf(fp,"%d %d %d %d\n",-1,-1,-1,1);
	}
	fclose(fp);


	printf("extract_stride_data completed successfuly. Exiting.\n");

	return 0;

}
