
#ifndef STATUSCODE_H
#define STATUSCODE_H

typedef enum {
    ST_OK = 0,
    ST_NOT_OK = 1, // general not ok, does not necessarily mean error
    ST_NOT_INIT,   // not initialized
    ST_INIT_FAIL,  // initialization failed
    ST_FILE_ERROR,
    ST_MALLOC_ERROR,
    ST_GEN_ERROR
} e_statuscode;

#endif // STATUSCODE_H