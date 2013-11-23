#include "../include/transceiver.h"

// Hacky way to swap in and out different implementations

#ifdef USE_PIPE_TRNSCVR
#include "transceivers/pipes.cpp"
#else
#include "transceivers/he100.cpp"
#endif
