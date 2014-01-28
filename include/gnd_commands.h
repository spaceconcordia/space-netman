#ifndef CDH_GND_H_
#define CDH_GND_H_

//  This file describes how command frames are built and sent to the satellite

#include <inttypes.h>
#include <boolstd.h>
#include "gnd_cdh"

// These functions will generate the proper bytes to be transceived depending on the command

bool cmd_get_time();
bool cmd_set_time(uint32 time);

// This function retrieves n bytes from a log file for a given subsystem
// TODO: Get log command currently consumes n bytes from a log file. We want to obtain the whole file, not just the logs
bool cmd_get_ss_log(subsystem_t subystem, size_t bytes);

bool cmd_reboot(uint32_t time, bool force);
bool cmd_upload(char* destination_path, char* filename);
bool cmd_kill_process(char* process_name, pid_t pid, subsystem_t ss);
bool cmd_start_process(subsystem_t ss);
bool cmd_decode();
bool cmd_tx_test();

// This function will notify the satellite to validate and execute the last received command
bool cmd_execute();

#endif
