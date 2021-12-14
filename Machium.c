#include "Machium.h"
#include "Memory.h"
#include "Register.h"
#include "Breakpoint.h"

machium_command_t machium_exit() {
    MACHIUM_EXIT;
}

void invalid_arg(Machium* machium) {
    printf(ERROR"Invalid argument: \'%s\'\n", machium->args[0]);
}

/*
help menu

machium->args[0] -> help
machium->args[1] -> [command]
*/
machium_command_t m_help(Machium* machium) {
    if (machium->args_count == 1) {
        printf(GOOD"List of commands. Type help [command] for more info:\n");
        printf(YELLOW "write "WHITE"- write to memory\n");
        printf(YELLOW"read "WHITE"- read from memory\n");
        printf(YELLOW"register "WHITE"- read/write registers\n");
        printf(YELLOW"breakpoint "WHITE"- set/remove breakpoints\n");
        printf(YELLOW"watchpoint "WHITE"- set/remove watchpoints\n");
        printf(YELLOW"pause "WHITE"- pauses debug task\n");
        printf(YELLOW"continue "WHITE"- continues debug task\n");
        printf(YELLOW"pid "WHITE"- lists pid or changes the process id\n");
        printf(YELLOW"exit "WHITE"- quits Machium debugger\n");
        return MACHIUM_SUCCESS;
    }
    if (!strcmp(machium->args[1], "help")) {
        printf("Thought I wouldn't program in this edge case?\n");
    }
    else if (!strcmp(machium->args[1], "exit")) {
        printf(YELLOW"[exit/q/quit] "WHITE"- exits Machium debugger\n");
    }
    else if (!strcmp(machium->args[1], "pid")) {
        printf(YELLOW"pid "WHITE"- lists process ID of debug task\n");
        printf(YELLOW"pid [pid] "WHITE"- changes process ID of debug task to [pid]\n");
    }
    else if (!strcmp(machium->args[1], "write")) {
        printf(YELLOW"[write/w] [0xaddress] [0xdata]"WHITE" - writes [0xdata] to [address]\n");
    }
    else if (!strcmp(machium->args[1], "read")) {
        printf(YELLOW"[read/r] [bytes/b] [0xaddress] [size]"WHITE" - reads [size] amount of bytes at [0xaddress]\n");
        printf(YELLOW"[read/r] [value/v] [0xaddress] [size]"WHITE" - reads [size <= 8] value at [0xaddress]\n");
        printf(YELLOW"[read/r] [lines/l] char [0xaddress] [lines]"WHITE" - reads [lines] amount of lines of memory as ASCII at [0xaddress]\n");
        printf(YELLOW"[read/r] [lines/l] bytes [0xaddress] [lines]"WHITE" - reads [lines] amount of lines of memory as bytes at [0xaddress]\n");
    }
    else if (!strcmp(machium->args[1], "register")) {
        printf(YELLOW"[register/reg] write [register] [0xdata]"WHITE" - writes [0xdata] to [register]\n");
        printf(YELLOW"[register/reg] read"WHITE" - prints all registers\n");
        printf("Valid registers -> x0-x28, pc, lr, cpsr, pad\n");
    }
    else if (!strcmp(machium->args[1], "breakpoint")) {
        printf(YELLOW"[breakpoint/br] [set/s] [0xaddress]"WHITE" - sets breakpoint at [0xaddress]\n");
        printf(YELLOW"[breakpoint/br] [remove/r]"WHITE" - removes breakpoint\n");
        printf("Max number of breakpoints is 6!\n");
    }
    else if (!strcmp(machium->args[1], "watchpoint")) {
        printf(YELLOW"[watchpoint/wa] [set/s] [0xaddress]"WHITE" - sets watchpoint at [0xaddress]\n");
        printf(YELLOW"[watchpoint/wa] [remove/r]"WHITE" - removes watchpoint\n");
        printf("Max number of watchpoints is 6!\n");
    }
    else if (!strcmp(machium->args[1], "pause")) {
        printf(YELLOW"[pause/p] "WHITE"- pauses debug task\n");
    }
    else if (!strcmp(machium->args[1], "continue")) {
        printf(YELLOW"[continue/c] "WHITE"- resumes debug task\n");
    }
    else {
        printf(ERROR"Unknown command!\n");
    }

    return MACHIUM_SUCCESS;
}

//pause target task
machium_command_t m_pause(Machium* machium) {
    kern_return_t kret;
    kret = task_suspend(machium->debug_task);
    printf(GOOD"Pausing task...\n");
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Unable to pause debug task!\n");
        return MACHIUM_FAILURE;
    }
    return MACHIUM_SUCCESS;
}

//resume target task
machium_command_t m_continue(Machium* machium) {
    kern_return_t kret;
    kret = task_resume(machium->debug_task); //unpauses task
    printf(GOOD"Resuming task...\n");
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Unable to resume debug task!\n");
        return MACHIUM_FAILURE;
    }
    return MACHIUM_SUCCESS;
}


//returns function pointer of Machium command
void* get_machium_command(Machium* machium) {
    //machium_exit
    if (!strcmp(machium->args[0], "exit")) return machium_exit;
    else if (!strcmp(machium->args[0], "quit")) return machium_exit;
    else if (!strcmp(machium->args[0], "q")) return machium_exit;

    //m_pid
    else if (!strcmp(machium->args[0], "pid")) return m_pid;

    //m_write
    else if (!strcmp(machium->args[0], "write")) return m_write;
    else if (!strcmp(machium->args[0], "w")) return m_write;

    //m_read
    else if (!strcmp(machium->args[0], "read")) return m_read;
    else if (!strcmp(machium->args[0], "r")) return m_read;

    //m_register
    else if (!strcmp(machium->args[0], "register")) return m_register;
    else if (!strcmp(machium->args[0], "reg")) return m_register;

    //m_pause
    else if (!strcmp(machium->args[0], "pause")) return m_pause;
    else if (!strcmp(machium->args[0], "p")) return m_pause;

    //m_continue
    else if (!strcmp(machium->args[0], "continue")) return m_continue;
    else if (!strcmp(machium->args[0], "c")) return m_continue;

    else if (!strcmp(machium->args[0], "breakpoint")) return m_breakpoint;
    else if (!strcmp(machium->args[0], "br")) return m_breakpoint;

    else if (!strcmp(machium->args[0], "watchpoint")) return m_watchpoint;
    else if (!strcmp(machium->args[0], "wa")) return m_watchpoint;

    //m_help
    else if (!strcmp(machium->args[0], "help")) return m_help;
    return &invalid_arg;
}

//command line interface
void machium_cli(Machium* machium) {
    char input[40]; //store direct input
    uint8_t args_index; //store index of each arguments

    machium_command_t* (*machium_call)(); //call function returned by get_machium_command

    printf(GOOD"For a list of commands, type 'help'\n");

    while (1) {
        memset(machium->args, 0, sizeof(char)*5*20); //fix end-of-line for arguments

        printf(NAME);
        fgets(input, 40, stdin); //get user input

        machium->args_count = 0; //reset arg values
        args_index = 0;

        for (int input_index = 0; input_index < strlen(input) + 1; input_index++) {
            if (input[input_index] == ' ' || input[input_index] == '\n') { //see if theres a space indicating a new argument or end of line
                machium->args_count++; //increase argument count if true
                if (machium->args_count > 4) {
                    break; //don't store more than 5 arguments
                }
                args_index = 0; //reset index of args
                input_index++;
            }
            machium->args[machium->args_count][args_index] = input[input_index]; //store input
            if (args_index < 20) {
                args_index++; //buffer overflows aren't cool.
            }
        }
        machium_call = get_machium_command(machium);
        machium_call(machium); //get command and call function for it
    }
}


int main(int argc, char *argv[]) {
    Machium* machium;
    kern_return_t kret; //hold debug task (obtained via tfp());

    machium = (Machium*) malloc(sizeof(struct Machium));

    printf(YELLOW "# " WHITE "Welcome to Machium Debugger!\n" WHITE);

    //test if we're running as root
    if (geteuid() && getuid()) {
        printf(ERROR"Run Machium as root!\n");
        machium_exit();
    }

    if (argc == 1) {
        //get pid to attach
        printf(GOOD"PID to attach: ");
        scanf("%d", &machium->pid);
        getchar();
    }
    else if (argc == 2) {
        machium->pid = strtol(argv[1], NULL, 0);
    }

    if (machium->pid == 0) {
        printf(ERROR"Machium doesn't support debugging on kernel_task! (task_for_pid(0))\n");
        printf(ERROR"You don't want any unwanted kernel panics, right?\n");
        machium_exit();
    }


    //task_for_pid gets a send right to the task of the process ID indicated by the second argument and stores it in the third argument
    //send rights can be stored in mach_port_t variables
    kret = task_for_pid(mach_task_self(), machium->pid, &machium->debug_task);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Couldn't obtain task_for_pid(%d)!\n", machium->pid);
        printf(ERROR"Do you have proper entitlements?\n");
        machium_exit();
    }
    else {
        printf(GOOD"Obtained task_for_pid(%d)\n", machium->pid);
    }
    machium_cli(machium); //start CLI
    return 0;
}
