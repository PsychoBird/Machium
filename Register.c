#include "Register.h"


/*
m_register_read handles register reading

machium->args[0] -> register
machium->args[1] -> read
machium->args[2] -> [register]
*/
machium_command_t m_register_read(Machium* machium) {
    kern_return_t kret;

    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;

    arm_thread_state64_t state;
    mach_msg_type_number_t state_count;

    if (machium->args_count < 2) {
        printf(ERROR"Not enough arguments for 'register read', 2 minimum\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 3) {
        printf(ERROR"Too many arguments for 'register read', 3 maximum\n");
        return MACHIUM_FAILURE;
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
    //how did I know to use ARM_THREAD_STATE64? check the xnu kernel source code. I just guessed.
    state_count = ARM_THREAD_STATE64_COUNT;
    kret = thread_get_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t) &state, &state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_get_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //all states here are defined in the xnu kernel in the struct in the typedef arm_thread_state64_t
    if (machium->args[2][0] == '\0' || !strcmp(machium->args[2], "all")) {
        for (int i = 0; i < 30; i++) {
            printf(GREEN "x%d " WHITE "= 0x%llx\n", i, state.__x[i]);
        }
        printf(YELLOW "fp " WHITE "= 0x%llx\n", state.__fp);
        printf(YELLOW "lr " WHITE "= 0x%llx\n", state.__lr);
        printf(YELLOW "sp " WHITE "= 0x%llx\n", state.__sp);
        printf(RED "pc " WHITE "= 0x%llx\n", state.__pc);
        printf(YELLOW "cpsr " WHITE "= 0x%x\n", state.__cpsr);
        printf(YELLOW "pad " WHITE "= 0x%x\n", state.__pad);
    }

    //resume first thread
    thread_resume(thread_list[0]);
    return MACHIUM_SUCCESS;
}

/*
m_register_write handles register reading

machium->args[0] -> register
machium->args[1] -> write
machium->args[2] -> [register]
machium->args[3] -> [value]
*/
machium_command_t m_register_write(Machium* machium) {
    kern_return_t kret;

    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;

    arm_thread_state64_t state;
    mach_msg_type_number_t state_count;

    uint64_t val;

    if (machium->args_count < 4) {
        printf(ERROR"Not enough arguments for 'register write', 4 minimum\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 4) {
        printf(ERROR"Too many arguments for 'register write', 4 maximum\n");
        return MACHIUM_FAILURE;
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
    //how did I know to use ARM_THREAD_STATE64? check the xnu kernel source code. I just guessed.
    state_count = ARM_THREAD_STATE64_COUNT;
    kret = thread_get_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t) &state, &state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not get thread_get_state with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    val = strtol(machium->args[3], NULL, 0);
    printf(GREEN"%s " WHITE " = 0x%llx", machium->args[2], val);

    //this is beyond ugly but there's probably not a better way to do this
    //all states here are defined in the xnu kernel in the struct in the typedef arm_thread_state64_t
    if (!strcmp(machium->args[2], "x0")) state.__x[0] = val;       // x0
    else if (!strcmp(machium->args[2], "x1")) state.__x[1] = val;  // x1
    else if (!strcmp(machium->args[2], "x2")) state.__x[2] = val;  // x2
    else if (!strcmp(machium->args[2], "x3")) state.__x[3] = val;  // x3
    else if (!strcmp(machium->args[2], "x4")) state.__x[4] = val;  // x4
    else if (!strcmp(machium->args[2], "x5")) state.__x[5] = val;  // x5
    else if (!strcmp(machium->args[2], "x6")) state.__x[6] = val;  // x6
    else if (!strcmp(machium->args[2], "x7")) state.__x[7] = val;  // x7
    else if (!strcmp(machium->args[2], "x8")) state.__x[8] = val;  // x8
    else if (!strcmp(machium->args[2], "x9")) state.__x[9] = val;  // x9
    else if (!strcmp(machium->args[2], "x10")) state.__x[10] = val; // x10
    else if (!strcmp(machium->args[2], "x11")) state.__x[11] = val; // x11
    else if (!strcmp(machium->args[2], "x12")) state.__x[12] = val; // x12
    else if (!strcmp(machium->args[2], "x13")) state.__x[13] = val; // x13
    else if (!strcmp(machium->args[2], "x14")) state.__x[14] = val; // x14
    else if (!strcmp(machium->args[2], "x15")) state.__x[15] = val; // x15
    else if (!strcmp(machium->args[2], "x16")) state.__x[16] = val; // x16
    else if (!strcmp(machium->args[2], "x17")) state.__x[17] = val; // x17
    else if (!strcmp(machium->args[2], "x18")) state.__x[18] = val; // x18
    else if (!strcmp(machium->args[2], "x19")) state.__x[19] = val; // x19
    else if (!strcmp(machium->args[2], "x20")) state.__x[20] = val; // x20
    else if (!strcmp(machium->args[2], "x21")) state.__x[21] = val; // x21
    else if (!strcmp(machium->args[2], "x22")) state.__x[22] = val; // x22
    else if (!strcmp(machium->args[2], "x23")) state.__x[23] = val; // x23
    else if (!strcmp(machium->args[2], "x24")) state.__x[24] = val; // x24
    else if (!strcmp(machium->args[2], "x25")) state.__x[25] = val; // x25
    else if (!strcmp(machium->args[2], "x26")) state.__x[26] = val; // x26
    else if (!strcmp(machium->args[2], "x27")) state.__x[27] = val; // x27
    else if (!strcmp(machium->args[2], "x28")) state.__x[28] = val; // x28
    else if (!strcmp(machium->args[2], "lr")) state.__lr = val;     // lr
    else if (!strcmp(machium->args[2], "pc")) state.__pc = val;     // pc
    else if (!strcmp(machium->args[2], "cpsr")) state.__cpsr = val; // cpsr
    else if (!strcmp(machium->args[2], "pc")) state.__pad = val;    // pad
    else { printf(ERROR"Invalid register.\n"); }

    //thread_set_state is basically just thread_get_state but it sets the values we changed
    kret = thread_set_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t)&state, state_count);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Could not call thread_set_state with error: %s\n", mach_error_string(kret));
    }
    //resume first thread
    thread_resume(thread_list[0]);
    return MACHIUM_SUCCESS;
}

/*
m_register handles all register commands

machium->args[0] -> register
machium->args[1] -> [command]
*/
machium_command_t m_register(Machium* machium) {
    if (!strcmp(machium->args[1], "read")) m_register_read(machium);
    else if (!strcmp(machium->args[1], "r")) m_register_read(machium);

    else if (!strcmp(machium->args[1], "write")) m_register_write(machium);
    else if (!strcmp(machium->args[1], "w")) m_register_write(machium);

    else {
        printf(ERROR"Invalid argument for 'register', %s\n", machium->args[1]);
        return MACHIUM_FAILURE;
    }
    return MACHIUM_SUCCESS;
}
