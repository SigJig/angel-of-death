
#ifndef STATUSCODE_H
#define STATUSCODE_H

typedef enum {
    ST_OK = 0,
    ST_NOT_OK, // general not ok, does not necessarily mean error
    ST_FILE_ERROR,
    ST_INIT_FAIL, // initialization failed
    ST_NOT_INIT, // not initialized
    ST_GEN_ERROR
} e_status;

#endif // STATUSCODE_H