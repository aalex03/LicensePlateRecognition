#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>

typedef enum
{
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
char opcode[256], receivedDataArduino[256], sendDataArduino[256];

void sendToArduino(char *message)
{
    serialPrintf(serial, "%s", message);
}

void setSysState(States *sysState, States *prevState, States newState)
{
    *prevState = *sysState;
    *sysState = newState;
}

void receiveFromArduino(char *buffer, int buffer_size)
{
    int index = 0;

    while (buffer_size > 1)
    {
        // Check if data is available
        while (!serialDataAvail(serial))
        {
            // Wait for data to be received
            puts("No data on serial");
            sleep(1);
        }

        // Read the character from serial
        char data = serialGetchar(serial);
        printf("%c", data);
        // Store the character in the buffer
        *buffer = data;
        buffer++;
        buffer_size--;
        index++;

        // Break the loop if a newline character is received
        if (data == '\n')
        {
            break;
        }
    }
    *(buffer - 2) = '\0';
}

char *executePythonScript()
{
    FILE *fp;
    char *output = (char *)malloc(256 * sizeof(char)); // Dynamically allocate memory for output
    if (output == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }
    char command[256];
    sprintf(command, "python run.py"); // Modify the command as per your script and arguments

    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("Failed to execute Python script");
        exit(1);
    }

    fgets(output, 256, fp);
    pclose(fp);

    // Process the output as needed
    return output;
}

int checkLicensePlate(const char *licensePlate)
{
    char command[256];
    char licensePlateCopy[256];
    strcpy(licensePlateCopy, licensePlate);

    char *token = strtok(licensePlateCopy, "\n"); // Split the string by newline character

    while (token != NULL)
    {
        sprintf(command, "./database/check_entry %s", token);

        int status = system(command);
        if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            printf("License Plate: %s, Exit Status: %d\n", token, exit_status);
            // Add your desired logic based on the exit_status
            if (exit_status == 1)
            {
                return exit_status;
            }
        }

        token = strtok(NULL, "\n"); // Get the next token
    }

    return 0;
}

int main()
{
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi initialization failed.\n");
        return 1;
    }

    serial = serialOpen("/dev/ttyUSB0", 9600); // Change the port and baud rate as per your setup
    if (serial == -1)
    {
        printf("Unable to open serial device.\n");
        return 1;
    }

    static States sysState = SETUP, sysStatePrev = SETUP;
    while (1)
    {
        sleep(1);
        switch (sysState)
        {
        case SETUP:
            printf("System SETUP\n");
            sprintf(opcode, "SETUP");
            sendToArduino(opcode);
            setSysState(&sysState, &sysStatePrev, INIT);
            break;
        case INIT:
            printf("System INIT\n");
            sprintf(opcode, "INIT");
            sendToArduino(opcode);
            sleep(2);
            setSysState(&sysState, &sysStatePrev, CHECK_CAR);
            break;
        case CHECK_CAR:
            printf("System CHECK_CAR\n");
            sprintf(opcode, "CHECK_CAR");
            sendToArduino(opcode);
            receiveFromArduino(receivedDataArduino, sizeof(receivedDataArduino));
            printf("Received data from Arduino: %s\n", receivedDataArduino);
            if (strcmp(receivedDataArduino, "ENT") == 0)
                setSysState(&sysState, &sysStatePrev, CHECK_BARRIER_ENTRY);
            else if (strcmp(receivedDataArduino, "EXT") == 0)
                setSysState(&sysState, &sysStatePrev, CHECK_BARRIER_EXIT);
            else if(strcmp(receivedDataArduino,"NO_CAR_DETECTED")==0)
                setSysState(&sysState, &sysStatePrev, CHECK_CAR); // Car not present at barriers
			else
				setSysState(&sysState, &sysStatePrev, CHECK_CAR);//Fault at communication, retrying
            break;
        case CHECK_BARRIER_ENTRY:
            printf("System CHECK_BARRIER_ENTRY\n");
            sprintf(opcode, "CHECK_BARRIER_ENTRY");
            sendToArduino(opcode);
            receiveFromArduino(receivedDataArduino, sizeof(receivedDataArduino));
            if (strcmp(receivedDataArduino, "TAKE_PHOTO_0") == 0)
                setSysState(&sysState, &sysStatePrev, CHECK_CAR);
            else if (strcmp(receivedDataArduino, "TAKE_PHOTO_1") == 0)
            {
                char *licensePlate = executePythonScript(); // should take the picture first to process and then run the algorithm?
                int exit_status = checkLicensePlate(licensePlate);
                char response[256];
                if (exit_status == 0)
                {
                    setSysState(&sysState, &sysStatePrev, DISPLAY);
                    sprintf(response, "License plate found in the database");
                }
                else if (exit_status == 1)
                {
                    setSysState(&sysState, &sysStatePrev, CHECK_CAR);
                    sprintf(response, "License plate not found in the database");
                }
                else
                {
                    setSysState(&sysState, &sysStatePrev, CHECK_CAR);
                    sprintf(response, "Error occurred while checking the license plate in the database");
                }
                printf("    License plate check status: %s\n", response);
            }
            break;
        case CHECK_BARRIER_EXIT:
            printf("System CHECK_BARRIER_EXIT\n");
            sprintf(opcode, "CHECK_BARRIER_EXIT");
            sendToArduino(opcode);
            sleep(2);
            receiveFromArduino(receivedDataArduino, sizeof(receivedDataArduino));
            if (strcmp(receivedDataArduino, "CAR_EXIT_1"))
                setSysState(&sysState, &sysStatePrev, OPENGATE_EXT);
            else if (strcmp(receivedDataArduino, "CAR_EXIT_0"))
                setSysState(&sysState, &sysStatePrev, CHECK_CAR);
            else
                setSysState(&sysState, &sysStatePrev, CHECK_CAR);
            break;
        case DISPLAY:
            printf("System DISPLAY\n");
            sprintf(opcode, "DISPLAY");
            sendToArduino(opcode);
            sleep(2);
            if (sysStatePrev == CHECK_BARRIER_ENTRY)
                setSysState(&sysState, &sysStatePrev, OPENGATE_ENT);
            else if (sysStatePrev == COUNTER)
                setSysState(&sysState, &sysStatePrev, INIT);
            break;
        case OPENGATE_ENT:
            printf("System OPENGATE_ENT\n");
            sprintf(opcode, "OPENGATE_ENT");
            sendToArduino(opcode);
            sleep(2);
            receiveFromArduino(receivedDataArduino, sizeof(receivedDataArduino));
            if (strcmp(receivedDataArduino, "LEFT_PARKING")==0)
                setSysState(&sysState, &sysStatePrev, INIT);
            else if (strcmp(receivedDataArduino, "ENTERED_PARKING")==0)
                setSysState(&sysState, &sysStatePrev, COUNTER);
            else
                setSysState(&sysState, &sysStatePrev, OPENGATE_ENT); // car in fron of barrier, did not enter or leave
            break;
        case OPENGATE_EXT:
            printf("System OPENGATE_EXT\n");
            sprintf(opcode, "OPENGATE_EXT");
            sendToArduino(opcode);
            sleep(2);
            receiveFromArduino(receivedDataArduino, sizeof(receivedDataArduino));
            if (strcmp(receivedDataArduino, "RETURNED_PARKING")==0)
                setSysState(&sysState, &sysStatePrev, INIT);
            else if (strcmp(receivedDataArduino, "EXITED_PARKING")==0)
                setSysState(&sysState, &sysStatePrev, COUNTER);
            else
                setSysState(&sysState, &sysStatePrev, OPENGATE_EXT); // car in fron of barrier, did not enter or leave
            break;
        case COUNTER:
            printf("System COUNTER\n");
            sprintf(opcode, "COUNTER");
            sendToArduino(opcode);
            sleep(2);
            setSysState(&sysState, &sysStatePrev, DISPLAY);
            break;
        default:
            printf("Error in state machine logic, entered default state\n");
        }
    }
}
