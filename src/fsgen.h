#ifndef __FSGEN_H
#define __FSGEN_H

int generate_rop(FILE* out, int is_bootstrap, const char* firmwareName, const char* deviceName, int pid_len, unsigned int slide);

#endif
