#ifndef ALHELPERS_H
#define ALHELPERS_H

#include "AL/alext.h"


/* Easy device init/deinit functions. InitAL returns 0 on success. */
int al_init();
void al_close();
void al_list_devices();

/* Helper function to get the name from the format enum. */
const char *al_format_name(ALenum format);

#endif 

