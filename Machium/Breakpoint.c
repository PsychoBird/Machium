#include "Breakpoint.h"

uint8_t br_count = 0; //breakpoint count
uint8_t wa_count = 0; //watchpoing count
bool started_exception_server = false;

/*
starts exception server to catch breakpoints / watchpoints
if we don't do this the remote process just crashes
*/
kern_return_t start_exception_server(Machium* machium) {
    mach_port_t server;
    kern_return_t kret;

    if (started_exception_server)
        return KERN_FAILURE; //only run this once

    //allocate mach port with a receive right for our remote task
    kret = mach_port_allocate(machium->debug_task, MACH_PORT_RIGHT_RECEIVE, &server);

    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not start exception server with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }

    //let's add a nice send right to the mach port
    mach_port_options_t options = { .flags = MPO_INSERT_SEND_RIGHT }; //thanks @s1guza for this line!
    kret = mach_port_construct(mach_task_self(), &options, 0, &server);


    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not insert rights to server with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }

    //this makes our exception server an ARM64 exception handler. currently only supporting breakpoints!
    kret = task_set_exception_ports(machium->debug_task, EXC_MASK_BREAKPOINT, server, EXCEPTION_STATE, ARM_THREAD_STATE64);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not set task exception port with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }
    started_exception_server = true;
    return KERN_SUCCESS;
}

/*
sets a hardware breakpoint
the max amount of hardware breakpoints is 6

machium->args[0] -> breakpoint
machium->args[1] -> [option]
machium->args[2] -> [address]
*/
machium_command_t m_breakpoint(Machium* machium) {
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;

    kern_return_t kret;

    arm_debug_state64_t state;
    mach_msg_type_number_t state_count;

    uint64_t address;

    if (machium->args_count < 2) {
        printf(ERROR"Not enough arguments for 'breakpoint', 2 minimum\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 3) {
        printf(ERROR"Too many arguments for 'breakpoint', 3 maximum\n");
        return MACHIUM_FAILURE;
    }

    //handle mach exceptions
    if (!started_exception_server) {
        kret = start_exception_server(machium);
        if (kret != KERN_SUCCESS) {
            printf(ERROR"Could not start breakpoint exception server!\n");
        }
    }


    //task_threads gets an array of active threads for the task indicated by the first argument
    kret = task_threads(machium->debug_task, &thread_list, &thread_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get task_threads with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //suspend thread 0 which will be our active app
    //if we don't do this, we'll just be returning register values in this function
    thread_suspend(thread_list[0]);

    //get all states for the first thread in the array
    //ARM_DEBUG_STATE64 is found nowhere on the internet.
    //I was able to find this through some help from some iOS researchers and digging through the XNU source code.
    //Although the XNU source code says there's 16 breakpoint / watchpoint registers, only 6 are supported by the ARM hardware.
    //Thanks Apple.
    state_count = ARM_DEBUG_STATE64_COUNT;
    kret = thread_get_state(thread_list[0], ARM_DEBUG_STATE64, (thread_state_t) &state, &state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_get_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    address = strtol(machium->args[2], NULL, 0);

    if (br_count == 5) {
        printf(ERROR"Max amount of hardware breakpoint registers used!\n");
        return MACHIUM_FAILURE;
    }

    if (!strcmp(machium->args[1], "remove") || !strcmp(machium->args[1], "r")) {
        if (br_count == 0) {
            printf(ERROR"No breakpoints enabled!\n");
            return MACHIUM_FAILURE;
        }
        br_count--;
        state.__bvr[br_count] = 0; //remove address
        state.__bcr[br_count] = BREAKPOINT_DISABLE; //disable breakpoint by setting state to 0
        printf(GOOD"Removing breakpoint %d\n", br_count);
    }

    if (!strcmp(machium->args[1], "set") || !strcmp(machium->args[1], "s")) {
        state.__bvr[br_count] = address; //set to the address where we want to set our breakpoint
        state.__bcr[br_count] = BREAKPOINT_ENABLE; //enable breakpoint at a hardware level
        printf(GOOD"Setting breakpoint %d at address 0x%llx\n", br_count, address);
        br_count++;
    }

    //thread_set_state is basically just thread_get_state but it sets the values we changed
    kret = thread_set_state(thread_list[0], ARM_DEBUG_STATE64, (thread_state_t)&state, state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_set_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //resume thread 0
    thread_resume(thread_list[0]);

    return MACHIUM_SUCCESS;
}

/*
sets a hardware watchpoint. basically the same as the m_breakpoint code
the max amount of hardware watchpoints is 6

machium->args[0] -> watchpoint
machium->args[1] -> [option]
machium->args[2] -> [address]
*/
machium_command_t m_watchpoint(Machium* machium) {
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;

    kern_return_t kret;

    arm_debug_state64_t state;
    mach_msg_type_number_t state_count;

    uint64_t address;

    if (machium->args_count < 2) {
        printf(ERROR"Not enough arguments for 'watchpoint', 2 minimum\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 3) {
        printf(ERROR"Too many arguments for 'watchpoint', 3 maximum\n");
        return MACHIUM_FAILURE;
    }

    //handle mach exceptions
    if (!started_exception_server) {
        kret = start_exception_server(machium);
        if (kret != KERN_SUCCESS) {
            printf(ERROR"Could not start breakpoint exception server!\n");
        }
    }

    //task_threads gets an array of active threads for the task indicated by the first argument
    kret = task_threads(machium->debug_task, &thread_list, &thread_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get task_threads with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //suspend thread 0 which will be our active app
    //if we don't do this, we'll just be returning register values in this function
    thread_suspend(thread_list[0]);

    //get all states for the first thread in the array
    //ARM_DEBUG_STATE64 is found nowhere on the internet.
    //I was able to find this through some help from some iOS researchers and digging through the XNU source code.
    //Although the XNU source code says there's 16 breakpoint / watchpoint registers, only 6 are supported by the ARM hardware.
    //Thanks Apple.
    state_count = ARM_DEBUG_STATE64_COUNT;
    kret = thread_get_state(thread_list[0], ARM_DEBUG_STATE64, (thread_state_t) &state, &state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_get_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    address = strtol(machium->args[2], NULL, 0);

    if (wa_count == 5) {
        printf(ERROR"Max amount of hardware watchpoint registers used!\n");
        return MACHIUM_FAILURE;
    }

    if (!strcmp(machium->args[1], "remove") || !strcmp(machium->args[1], "r")) {
        if (wa_count == 0) {
            printf(ERROR"No watchpoints enabled!\n");
            return MACHIUM_FAILURE;
        }
        wa_count--;
        state.__bvr[wa_count] = 0; //remove address
        state.__bcr[wa_count] = BREAKPOINT_DISABLE; //disable breakpoint and continue execution
        printf(GOOD"Removing watchpoint %d\n", wa_count);

    }

    if (!strcmp(machium->args[1], "set") || !strcmp(machium->args[1], "s")) {
        state.__bvr[wa_count] = address; // address of the watchpoint
        state.__bcr[wa_count] = BREAKPOINT_ENABLE; //literally the same as above. enables a hardware watchpoint
        printf(GOOD"Setting watchpoint %d at address 0x%llx\n", wa_count, address);
        wa_count++;
    }

    //thread_set_state is basically just thread_get_state but it sets the values we changed
    kret = thread_set_state(thread_list[0], ARM_DEBUG_STATE64, (thread_state_t)&state, state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_set_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //resumes thread 0
    thread_resume(thread_list[0]);

    return MACHIUM_SUCCESS;
}
