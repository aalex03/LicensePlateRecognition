#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>

typedef enum {
    SETUP,
    WAITFORSIGNAL,
    CHECK
} States;

int serial;

void sendToArduino(const char *message) {
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

    static States sysState = SETUP;
    while (1) {
        switch (sysState) {
            case SETUP:
                printf("System setup\n");
                // Additional setup logic can be added here
                sysState = WAITFORSIGNAL;
                break;
            case WAITFORSIGNAL:
                printf("Waiting for signal from Arduino\n");
                char received_message[256];
                receiveFromArduino(received_message, sizeof(received_message));
                printf("Signal received: %s\n", received_message);
                sysState = CHECK;
                break;
            case CHECK: {
                printf("Running Python script for image processing and checking license plate in the database\n");
                char *licensePlate = executePythonScript();
                int exit_status = checkLicensePlate(licensePlate);

                char response[256];
                if (exit_status == 1) {
                    sprintf(response, "License plate found in the database");
                } else if (exit_status == 0) {
                    sprintf(response, "License plate not found in the database");
                } else {
                    sprintf(response, "Error occurred while checking the license plate in the database");
                }

                sendToArduino(response);
                printf("Response sent to Arduino: %s\n", response);

                free(licensePlate);  // Free the dynamically allocated memory
                sysState = WAITFORSIGNAL;
                break;
            }
        }
    }
}
