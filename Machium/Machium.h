#ifndef MACHIUM_H
#define MACHIUM_H

#include <stdio.h>
#include <strings.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>


//colors to make Machium more beautiful
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[00m"

#define GOOD GREEN "# " WHITE
#define ERROR RED "# " WHITE
#define WARNING YELLOW "# " WHITE
#define MACHIUM_EXIT\
                    printf(ERROR"Exiting Machium...\n");\
                    exit(0);
#define NAME BLUE "(Machium) " WHITE

#define MACHIUM_FAILURE 0
#define MACHIUM_SUCCESS 1

typedef int8_t machium_command_t;

typedef struct Machium {
    pid_t pid; //process ID of application being debugged
    mach_port_t debug_task; //task port of application being debugged
    char args[5][20]; //command line arguments of user
    uint8_t args_count; //argument count of CLI inputs
} Machium;

//print commands
machium_command_t m_help(Machium* machium);

//pause task threads
machium_command_t m_pause(Machium* machium);

//continue task threads
machium_command_t m_continue(Machium* machium);

//exit machium;
machium_command_t machium_exit();

//handle invalid CLI argument
void invalid_arg(Machium* machium);

//return the function pointer of the machium function about to be called
void* get_machium_command(Machium* machium);

//command line interface for Machium, repeats in infinte loop until debugger exits
void machium_cli(Machium* machium);




#endif /* MACHIUM_H */
