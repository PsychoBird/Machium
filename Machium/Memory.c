#include "Memory.h"

/*
m_pid handles the process id of the Debugger

machium->args[0] -> pid
machium->args[1] -> [change pid] (OPTIONAL)
*/
machium_command_t m_pid(Machium* machium) {
    kern_return_t kret;
    pid_t pid;

    if (machium->args_count == 1) {
        printf(GOOD"PID of debugging task: %d\n", machium->pid);
        return MACHIUM_SUCCESS;
    }
    else if (machium->args_count == 2) {
        pid = strtol(machium->args[1], NULL, 0);
        //task_for_pid gets a send right to the task of the process ID indicated by the second argument and stores it in the third argument
        //send rights can be stored in mach_port_t variables
        kret = task_for_pid(mach_task_self(), pid, &machium->debug_task);
        if (pid == 0) {
            printf(ERROR"Machium doesn't support debugging on kernel_task! (task_for_pid(0))\n");
            printf(ERROR"You don't want any unwanted kernel panics, right?\n");
            return MACHIUM_FAILURE;
        }
        if (kret != KERN_SUCCESS) {
            printf(ERROR"Unable to obtain task_for_pid(%d)!\n", pid);
            return MACHIUM_FAILURE;
        }
        else {
            machium->pid = pid;
            printf(GOOD"Changed debugging task to task_for_pid(%d)\n", pid);
        }
    }
    else {
        printf(ERROR"Too many arguments for 'pid', 2 max.\n");
        return MACHIUM_FAILURE;
    }
    return MACHIUM_SUCCESS;
}

/*
read a certain number of bytes out at a memory address

machium->args[0] -> read
machium->args[1] -> bytes
machium->args[2] -> [address]
machium->args[3] -> [size]
*/
machium_command_t m_read_bytes(Machium* machium) {
    kern_return_t kret;
    uint64_t address;
    size_t size;
    uint8_t* read_out;

    if (machium->args_count < 4) {
        printf(ERROR"Not enough arguments for 'read bytes', 4 required\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 4) {
        printf(ERROR"Too many arguments for 'read bytes', 4 required\n");
        return MACHIUM_FAILURE;
    }

    address = (uint64_t) strtol(machium->args[2], NULL, 0);
    size = (size_t) strtol(machium->args[3], NULL, 0);

    read_out = (uint8_t*) malloc(size); //create readout buffer

    printf(GOOD"Reading %zu bytes from memory address 0x%llx...\n", size, address);

    //reads [size] data from [address] in the debug task and stores it in read_out
    kret = vm_read_overwrite(machium->debug_task, address, size, (vm_offset_t) read_out, &size);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Failed to read bytes from memory!\nError: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    //print out read data
    printf("0x");
    for (int i = 0; i < size; i++)
        printf("%x", (uint8_t) read_out[i]);
    printf("\n");

    free(read_out); //free readout buffer

    return MACHIUM_SUCCESS;
}

/*
read lines of memory at a given memory address

machium->args[0] -> read
machium->args[1] -> lines
machium->args[2] -> [type]
machium->args[3] -> [address]
machium->args[4] -> [lines]
*/
machium_command_t m_read_lines(Machium* machium) {
    kern_return_t kret;

    uint64_t address;
    uint8_t alignment_value;

    int total_lines;
    bool is_reading_char;
    char* read_out;
    size_t size;

    if (machium->args_count < 5) {
        printf(ERROR"Not enough arguments for 'read lines', 5 required\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 5) {
        printf(ERROR"Too many arguments for 'read lines', 5 required\n");
        return MACHIUM_FAILURE;
    }

    address = (uint64_t) strtol(machium->args[3], NULL, 0);
    total_lines = (int) strtol(machium->args[4], NULL, 0);

    if (total_lines > 20) {
        total_lines = 20; //lazy way to stop memory corruption
        printf(WARNING"Max lines to print is 20!\n");
    }

    printf(GOOD"Reading %d %s lines from memory address 0x%llx...\n", total_lines, machium->args[2], address);

    //the amount of bytes we're reading
    size = 16 * total_lines;

    read_out = (char*) malloc(size); //create readout buffer

    //align our bytes to 0xf so we can readout evenly at a line value
    alignment_value = address % 16;
    address = address - alignment_value;

    //reads [size] data from [address] in the debug task and stores it in read_out
    kret = vm_read_overwrite(machium->debug_task, address, size, (vm_offset_t) read_out, &size);

    if (kret != KERN_SUCCESS) {
        printf(ERROR"Failed to read from memory!\nError: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    printf(GREEN "0x%llx " WHITE "| ", address + alignment_value);

    if (!strcmp(machium->args[2], "char")) {
        is_reading_char = true;
        printf(YELLOW "0 1 2 3 4 5 6 7 8 9 A B C D E F \n");
    } else if (!strcmp(machium->args[2], "bytes")) {
        is_reading_char = false;
        printf(YELLOW "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F \n");
    }

    //print all of the lines being read
    for (int read_lines = 0; read_lines < total_lines; read_lines++) {
        //create starter of new line
        printf(BLUE "0x%llx " WHITE "| ", address);
        for (int i = (16 * read_lines); i < (16 * (read_lines + 1)); i++) {
            if (!is_reading_char) {
                //make sure the readout is aligned
                if ((uint8_t) read_out[i] <= 0xf) {
                    printf("0%x ", (uint8_t) read_out[i]);
                } else {
                    printf("%x ", (uint8_t) read_out[i]);
                }
            }
            else {
                //only read out valid ascii characters
                if ((uint8_t) read_out[i] >= 33 && (uint8_t) read_out[i] <= 126) {
                    printf("%c ", read_out[i]);
                } else {
                    printf(RED"? "WHITE);
                }
            }
        }

        printf("\n");
        address += 16; //new line starts
    }

    free(read_out); //free readout

    return MACHIUM_SUCCESS;
}

/*
reads value from memory address

machium->arg[0] -> read
machium->arg[1] -> value
machium->arg[2] -> [address]
machium->arg[3] -> [size] (MAX 8)
*/
machium_command_t m_read_value(Machium* machium) {
    kern_return_t kret;
    uint64_t address;
    size_t size;
    uint64_t read_out;

    if (machium->args_count < 4) {
        printf(ERROR"Not enough arguments for 'read value', 4 required\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 4) {
        printf(ERROR"Too many arguments for 'read value', 4 required\n");
        return MACHIUM_FAILURE;
    }

    read_out = 0;

    address = (uint64_t) strtol(machium->args[2], NULL, 0);
    size = (size_t) strtol(machium->args[3], NULL, 0);

    if (size > 8) {
        size = 8; //sizeof(vm_offset_t) == 8
        printf(WARNING"Max read out size is 8!\n");
    }

    printf(GOOD"Reading size %zu value from memory address 0x%llx...\n", size, address);

    //reads [size] data from [address] in the debug task and stores it in read_out
    kret = vm_read_overwrite(machium->debug_task, address, size, (uint64_t) &read_out, &size);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Failed to read value from memory!\nError: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }
    printf("0x%llx\n", read_out);

    return MACHIUM_SUCCESS;
}

/*
handle read command

machium->args[0] -> read
machium->args[1] -> [command]
*/
machium_command_t m_read(Machium* machium) {
    if (!strcmp(machium->args[1], "bytes")) m_read_bytes(machium);
    else if (!strcmp(machium->args[1], "b")) m_read_bytes(machium);

    else if (!strcmp(machium->args[1], "lines")) m_read_lines(machium);
    else if (!strcmp(machium->args[1], "l")) m_read_lines(machium);

    else if (!strcmp(machium->args[1], "value")) m_read_value(machium);
    else if (!strcmp(machium->args[1], "v")) m_read_value(machium);
    else {
        printf(ERROR"Invalid argument for 'read', %s\n", machium->args[1]);
        return MACHIUM_FAILURE;
    }

    return MACHIUM_SUCCESS;
}


/*
handle read command

machium->args[0] -> write
machium->args[1] -> [address]
machium->args[2] -> [data]
*/
machium_command_t m_write(Machium* machium) {
    kern_return_t kret;
    vm_address_t address;
    size_t size;
    vm_address_t data;

    vm_address_t region_address;
    vm_size_t region_size;
    vm_region_flavor_t flavor;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t count;
    mach_port_t object;

    vm_prot_t prot;


    if (machium->args_count < 3) {
        printf(ERROR"Not enough arguments for 'write', 3 required\n");
        return MACHIUM_FAILURE;
    }
    else if (machium->args_count > 3) {
        printf(ERROR"Too many arguments for 'write', 3 required\n");
        return MACHIUM_FAILURE;
    }

    address = (vm_address_t) strtol(machium->args[1], NULL, 0);
    data = (vm_offset_t) strtol(machium->args[2], NULL, 0);
    size = sizeof(data);

    region_address = address;
    flavor = VM_REGION_BASIC_INFO_64;
    count = VM_REGION_BASIC_INFO_COUNT_64;

    //im not explaining the use of vm_region_64 but i literally just use this to get the protection at memory address region_address
    kret = vm_region_64(machium->debug_task, &region_address, &region_size, flavor, (vm_region_info_64_t) &info, &count, &object);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"vm_region_64 failure with error: %s\n", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }
    prot = info.protection; //save protection

    //vm_protect for vm_write-ing data to memory
    kret = vm_protect(machium->debug_task, address, region_size, false, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"vm_protect failure with error: %s", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    printf(GOOD"Writing %lx to memory address 0x%lx...\n", data, address);

    //write [data] of [size] to [address]
    kret = vm_write(machium->debug_task, address, (vm_offset_t) data, size);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Failed to write value to memory!\nError: %s\n", mach_error_string(kret));
    }

    //restore original protections
    kret = vm_protect(machium->debug_task, address, region_size, false, prot);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"vm_protect failure with error: %s", mach_error_string(kret));
        return MACHIUM_FAILURE;
    }

    printf(GOOD"Successfully wrote data!\n");

    return MACHIUM_SUCCESS;
}
