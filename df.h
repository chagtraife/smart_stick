#ifndef DF_H
#define DF_H

#include "common.h"
#include "DFRobotDFPlayerMini.h"

#define FPSerial Serial

void df_init();
void alarm();
void askUser();
void add_sdt_success();
void reset_sdt_success();
void stopDF();

#endif