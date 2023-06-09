#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void runPythonScript(const char* scriptPath){
    char command[256];
    sprintf(command,"python3 %s",scriptPath);
    //Open a pipe to execute the Python script
    FILE* pipe = popen(command,"r");
    if(!pipe){
        printf("Error opening pipe to Python script \n");
    }
    char buffer[256];
    int result =0;
     
     //Read the output of the Python script
     if(fgets(buffer,sizeof(buffer),pipe)!=NULL){
        printf("Python script returned %s\n",buffer);
     }
     pclose(pipe);
}
int main(){
    const char* scriptPath="main.py";
    runPythonScript(scriptPath);
    return 0;
}