#ifndef BREAKPOINT_H
#define BREAKPOINT_H

#include "Machium.h"

//these value were found through the ARM manual.
//I don't feel like explaining the siginificance of these but 481 the end value of some shifted bits and im too lazy to put it in C code
#define BREAKPOINT_ENABLE 481
#define BREAKPOINT_DISABLE 0

//start the mach exception server which will handle hardware breakpoint exceptions
//this just prevents crashes for now
kern_return_t start_exception_server(Machium* machium);

//handle breakpoints
machium_command_t m_breakpoint(Machium* machium);

//handle watchpoints
machium_command_t m_watchpoint(Machium* machium);

#endif /* BREAKPOINT_H */
