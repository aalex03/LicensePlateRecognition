#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <wiringPi.h>
//#include <wiringSerial.h>

typedef enum {
    SETUP,
    INIT,
    CHECK_CAR,
    CHECK_BARRIER_ENTRY,
    CHECK_BARRIER_EXIT,
    DISPLAY,
    OPENGATE_ENT,
    OPENGATE_EXT,
    COUNTER
} States;

int serial;
char opcode[256],receivedDataArduino[256],sendDataArduino[256];

void sendToArduino(char *message) {
    serialPrintf(serial, "%s", message);
}

void receiveFromArduino(char *buffer, int buffer_size) {
    while (serialDataAvail(serial)) {
        *buffer = serialGetchar(serial);
        buffer++;
        buffer_size--;
        if (buffer_size <= 1) {
            break;
        }
    }
    *buffer = '\0';
}

char *executePythonScript() {
    FILE *fp;
    char *output = (char *)malloc(256 * sizeof(char));  // Dynamically allocate memory for output
    if (output == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }
    char command[256];
    sprintf(command, "python run.py");  // Modify the command as per your script and arguments

    fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to execute Python script");
        exit(1);
    }

    fgets(output, 256, fp);
    pclose(fp);

    // Process the output as needed
    return output;
}

int checkLicensePlate(const char *licensePlate) {
    char command[256];
    sprintf(command, "./database/check_entry %s", licensePlate);

    int status = system(command);
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        return exit_status;
    }
    return -1;  // Error occurred while executing the check_entry executable
}

int main() {
    if (wiringPiSetup() == -1) {
        printf("WiringPi initialization failed.\n");
        return 1;
    }

    serial = serialOpen("/dev/ttyUSB0", 9600);  // Change the port and baud rate as per your setup
    if (serial == -1) {
        printf("Unable to open serial device.\n");
        return 1;
    }

    static States sysState = SETUP,sysStatePrev=SETUP;
    while (1) {
        switch (sysState) {
            case SETUP:
                printf("System SETUP\n");
                sprintf(opcode,"SETUP");
                sendToArduino(opcode);
                sysState = INIT;
                break;
            case INIT:
                printf("System INIT\n");
                sprintf(opcode,"INIT");
                sendToArduino(opcode);
                sysState=CHECK_CAR;
                break;
            case CHECK_CAR:
                printf("System CHECK_CAR\n");
                sprintf(opcode,"CHECK_CAR");
                sendToArduino(opcode);
                receiveFromArduino(receivedDataArduino,sizeof(receivedDataArduino));
                if(receivedDataArduino=="ENT")
                    sysState=CHECK_BARRIER_ENTRY;
                else if(receivedDataArduino=="EXT")
                        sysState=CHECK_BARRIER_EXIT;
                    else
                        sysState=INIT;//Reading fault
                break;
            case CHECK_BARRIER_ENTRY:
                printf("System CHECK_BARRIER_ENTRY\n");
                sprintf(opcode,"CHECK_BARRIER_ENTRY");
                sendToArduino(opcode);
                receiveFromArduino(receivedDataArduino,sizeof(receivedDataArduino));
                if(receivedDataArduino=="TAKE_PHOTO_0")
                    sysState=CHECK_CAR;
                else if(receivedDataArduino=="TAKE_PHOTO_1"){
                        char *licensePlate = executePythonScript();// should take the picture first to process and then run the algorithm?
                        int exit_status = checkLicensePlate(licensePlate);
                        char response[256];
                        if (exit_status == 1) {
                            //sprintf(sendDataArduino,"LICENSE:%s",licensePlate);
                            //sendToArduino(sendDataArduino);
                            sysState=DISPLAY; 
                            sprintf(response, "License plate found in the database");

                        } else if (exit_status == 0) {
                            sysState=CHECK_CAR;
                            sprintf(response, "License plate not found in the database");
                        } else {
                            sysState=CHECK_CAR;
                            sprintf(response, "Error occurred while checking the license plate in the database");
                        }
                        printf("License plate check status: %s\n", response);
                        
                }                            
                break;
            case CHECK_BARRIER_EXIT:
                printf("System CHECK_BARRIER_EXIT\n");
                sprintf(opcode,"CHECK_BARRIER_EXIT");
                sendToArduino(opcode);
                receiveFromArduino(receivedDataArduino,sizeof(receivedDataArduino));
                if(receivedDataArduino=="CAR_EXIT_1")
                    sysState=OPENGATE_EXT;
                    else if(receivedDataArduino=="CAR_EXIT_0")
                    sysState=CHECK_CAR; 
                    else
                    sysState=CHECK_CAR;
                break;
            case DISPLAY:
                printf("System DISPLAY\n");
                sprintf(opcode,"DISPLAY");
                sendToArduino(opcode);
                if(sysStatePrev==CHECK_BARRIER_ENTRY)
                    sysState=OPENGATE_ENT;
                    else if(sysStatePrev==COUNTER)
                        sysState=INIT;
                break;
            case OPENGATE_ENT:
                printf("System OPENGATE_ENT\n");
                sprintf(opcode,"OPENGATE_ENT");
                sendToArduino(opcode);
                receiveFromArduino(receivedDataArduino,sizeof(receivedDataArduino));
                if(receivedDataArduino=="LEFT_PARKING")
                    sysState=INIT;
                    else if (receivedDataArduino=="ENTERED_PARKING")
                        sysState=COUNTER;
                        else
                        sysState=OPENGATE_ENT;//car in fron of barrier, did not enter or leave 
                    break;
            case OPENGATE_EXT:
                printf("System OPENGATE_EXT\n");
                sprintf(opcode,"OPENGATE_EXT");
                sendToArduino(opcode);
                receiveFromArduino(receivedDataArduino,sizeof(receivedDataArduino));
                if(receivedDataArduino=="RETURNED_PARKING")
                    sysState=INIT;
                    else if (receivedDataArduino=="EXITED_PARKING")
                        sysState=COUNTER;
                        else
                        sysState=OPENGATE_EXT;//car in fron of barrier, did not enter or leave 
                break;
            case COUNTER:
                printf("System COUNTER\n");
                sprintf(opcode,"COUNTER");
                sendToArduino(opcode);            
                sysState=DISPLAY;
                break;
            default:
                printf("Error in state machine logic, entered default state\n");
        }
        sysStatePrev=sysState;
    }
}
