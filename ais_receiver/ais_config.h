/***************************************************************************************************
*  File:        ais_config.h                                                                       *
*  Authors:     Noemí Miguélez (NM)                                                                *
*               Joan Francesc Muñoz Martin (JFM)                                                   *
*                                                                                                  *
*  Creation:    28-01-2018                                                                         *
*  Description: AisConfiguration file reader and parser (header)                                   *
*                                                                                                  *
*  This file is part of a project developed by Nano-Satellite and Payload Laboratory (NanoSat Lab) *
*  at Technical University of Catalonia - UPC BarcelonaTech.                                       *
*                                                                                                  *
*  AIS configuration file reader. Reads a config file from a path and it sets AISConfiguration     *
*                                                                                                  *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date            Author  Description                                                        *
*  0.1  28-01-2018      <NM>   <First version>                                                     *
*  0.2  30-01-2018      <JFM>  <File path is an input now>                                         *
***************************************************************************************************/

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

#include "jsmn.h"

#define CONFIG_ELEMENT_INT(name) if (strcmp(key, #name) == 0)  ais_config.name = atoi(value)
#define CONFIG_ELEMENT_STR(name) if (strcmp(key, #name) == 0) strcpy(ais_config.name, value);


typedef struct AISConfiguration {
    int total_messages;   //Maximum number of messages to be saved.
} AISConfiguration;


void load_configuration(const char *path, AISConfiguration *ais_config);

#endif
