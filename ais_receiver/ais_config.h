#ifndef __AIS_CONFIG_H__
#define __AIS_CONFIG_H__

#include <stdlib.h>
#include <stdint.h>
#include <fftw3.h>
#include <stdio.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define CONFIG_FILE_SYSTEM      "conf/ais.conf"

#define CONFIG_ELEMENT_INT(name) if (strcmp(key, #name) == 0)  ais_config.name = atoi(value)
#define CONFIG_ELEMENT_STR(name) if (strcmp(key, #name) == 0) strcpy(ais_config.name, value);


typedef struct AISConfiguration {
    int total_messages;   //Maximum number of messages to be saved.
} AISConfiguration;


void load_configuration(void);
extern AISConfiguration ais_config;

#endif
