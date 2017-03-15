#include <unistd.h>
#include <mraa/aio.h>
#include <stdio.h>
#include <stdlib.h>
#include "floatfann.h"

int main()
{
    int i, rv, N_SAMPLES = 0;
    int temp0, temp1, temp2, speed;
    uint16_t value0, value1, value2;
    float max;
    fann_type *calc_out;
    fann_type input[3];
    struct fann *ann;
    FILE *data;
    ssize_t read;
    size_t len = 0;
    char *line = NULL;

    data = fopen("log.csv", "r");
    if (data == NULL){
	fprintf(stderr, "Could not open the input file...Please try again\n");
        exit(EXIT_FAILURE);
	}
    
    
    while ((read = getline(&len, &line, data) != -1)) {N_SAMPLES++;}
    N_SAMPLES = N_SAMPLES / 2;
    rewind(data);
      
    ann = fann_create_from_file("TEST.net");
    i = 0;
    while ((read = getline(&len, &line, data)) != -1) {
	
	if (i % 2 == 0){
		rv = sscanf(line, "%lf, %f, %f\n", input[0], input[1], input[2]);
	}
		
	/*	
        temp0 = 0;
        temp1 = 0;
        temp2 = 0;

        input[0] = (float) value0 / 1000;
        input[1] = (float) value1 / 1000;
        input[2] = (float) value2 / 1000;
        */

	max = -100;

	calc_out = fann_run(ann, input);

        for (i = 0; i < 4; i++) {
            if (calc_out[i] > max) {
                max = calc_out[i];
                speed = i;
            }
        }

	printf("Period/Accel/Decel values: %d, %d, %d -> speed is %d\n", value0, value1, value2, location);
        sleep(1);
    }
    fann_destroy(ann);
    return 0;
}
