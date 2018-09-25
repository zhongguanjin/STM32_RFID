#ifndef _SYN6658_H_
#define _SYN6658_H_

#include "config.h"





extern void Syn6658_Play(char *buf);
extern void Syn6658_Cmd(uint8 cmd);
extern void syn6658_check(void);
extern uint8 get_rspcmd(uint8 dat);
#endif

