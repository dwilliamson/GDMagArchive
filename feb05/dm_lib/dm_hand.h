#ifndef INC_DM_HAND_H
#define INC_DM_HAND_H

#include <stdio.h>

extern void dmInit(void);

typedef void * (*dmLoadFromNameFP)(char *name);
typedef void * (*dmLoadFromFileFP)(FILE *f, int len);
typedef void * (*dmPostloadProcessingFP)(void *data, int data_len, void *param);
typedef void   (*dmUnloadFP)(void *data);

extern void dmRegisterFormatHandler(int typecode,
                                    int dependent_typecode,
                                    char *extension,
                                    dmLoadFromNameFP loadFromName,
                                    dmLoadFromFileFP loadFromFile,
                                    dmPostloadProcessingFP postloadProcessing,
                                    dmUnloadFP unload);

#define DM_REGISTER(type, ext, load, postload, unload) \
    dmRegisterFormatHandler(type, 0, ext, NULL, load, postload, unload)

#define DM_REGISTER_FILENAME(type, ext, load, postload, unload) \
    dmRegisterFormatHandler(type, 0, ext, load, NULL, postload, unload)

#define DM_REGISTER_NAMED(type, load, postload, unload) \
    dmRegisterFormatHandler(type, 0, NULL, load, NULL, postload, unload)       

#endif
