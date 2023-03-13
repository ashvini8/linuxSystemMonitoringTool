#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#define USER_PROCESS 7
#define no_argument 0

/*
    This struct is meant to store the previous iteration's delta total of cpu, delta idle of cpu, the number of displayed in graphics
and the CPU usage percentage
*/
typedef struct cpuStat{
    double total;
    double idle;
    int bars;
    double cpu_perc;
}cpuStat;

/*
    This struct is meant to store the previous iteration's vitual memory used
*/
typedef struct virtualMem{
    double virtualValue;
}virtualMem;

/*
    This function creates, initializes and returns a new cpuStat node
*/
cpuStat *new_cpuStat(){
    cpuStat *new_node = (cpuStat *)calloc(1, sizeof(cpuStat));
    new_node->total = -1;
    new_node->idle = -1;
    new_node->bars = 9;
    new_node->cpu_perc = -1;
    return new_node;
}

/*
    This function creates, initializes and returns a new virtualMem node
*/
virtualMem *new_virtualMem(){
    virtualMem *new_node = (virtualMem *)calloc(1, sizeof(virtualMem));
    new_node->virtualValue = -1;
    return new_node;
}

/*
    Print out the System Name, Machine Name, Version, Release and Architecture
for the System Information Section.
*/
void systemInfo(){

    printf("---------------------------------------\n");
    printf("### System Information ###\n ");
    struct utsname uData;
    uname(&uData);

    //printing out the following fields values
    printf("System Name = %s\n", uData.sysname);
    printf("Machine Name = %s\n", uData.nodename);
    printf("Version = %s\n", uData.version);
    printf("Release = %s\n", uData.release);
    printf("Architecture = %s\n", uData.machine);
    printf("---------------------------------------\n");

}

/*
    Printing out the username, device name of tty and, hostname for remote login or kernel
version for run-level messages for all users one after the other
*/
void userInfo(){

    printf("---------------------------------------\n");
    struct utmp *user_p;
    setutent();
    user_p = getutent();

    //traversing through all users
    while(user_p != NULL){

        //checking if current user is "valid"
        if(user_p->ut_type == USER_PROCESS){
            printf("%s\t %s\t (%s)\n", user_p->ut_user, user_p->ut_line, user_p->ut_host);
        }
        user_p = getutent();
    }
    endutent();


}

/*
    This function takes an integer indicating which iteration the program is on (while printing)
and a pointer to a struct of type cpuStat that contains the previous iterations values.
This function prints the number of cores and prints the CPU usage percentage by calculating the current sample's values
and the previous sample's values.
*/
cpuStat *coresCPU(int ind, cpuStat *prev){
    printf("---------------------------------------\n");

    int numCores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of cores: %d\n", numCores);

    FILE *fp;
    char cpu1[10]; //temp to store the first string "cpu" in the file
    char cpu2[10]; //temp to store the first string "cpu" in the file

    //variables to store the different values in the file
    double user1, nice1, system1, idle1, iowait1, irq1, softirq1;
    double user2, nice2, system2, idle2, iowait2, irq2, softirq2;

    double cpu_perc;

    //values that will hold sample 1 values if ind = 0
    double total_prev0;
    double idle_prev0;

    //sample 1 if this is the first iteration
    if(ind == 0){
        fp = fopen("/proc/stat", "r");
        fscanf(fp, "%s %lf %lf %lf %lf %lf %lf %lf",cpu1, &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1);
        total_prev0 =  user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1;
        idle_prev0 = idle1 + iowait1;
        fclose(fp);
    }

    //getting sample 2
    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%s %lf %lf %lf %lf %lf %lf %lf",cpu2, &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2);
    double total_curr = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2;
    double idle_curr = idle2 + iowait2;
    fclose(fp);

    //if this is the first iteration
    if(ind == 0){
        if(total_curr - total_prev0 == 0){ //if delta total is 0, since we cannot divide by 0, I divided by 0.000001
            cpu_perc = ((total_curr - total_prev0) - (idle_curr - idle_prev0))/(0.000001) * 100;
        }else{
            cpu_perc = ((total_curr - total_prev0) - (idle_curr - idle_prev0))/(total_curr - total_prev0) * 100;
        }
    }
    else{ //other iterations
        if(total_curr - prev->total == 0){
            cpu_perc = ((total_curr - prev->total) - (idle_curr - prev->idle))/(0.000001) * 100;
        }else{
            cpu_perc = ((total_curr - prev->total) - (idle_curr - prev->idle))/(total_curr - prev->total) * 100;
        }
    }
    printf("total cpu use = %.2lf %%\n", cpu_perc);

    //storing the current cpu percentage, delta total and delta idle in the field values of the node pointer
    prev->cpu_perc = cpu_perc;
    prev->total = total_curr;
    prev->idle = idle_curr;

    return prev;
}

/*
    This function collects the total physical, physically used, total virtual, and virtually used memory (and coverts them to GB)
Then it takes those values and stores them in a string with this structure: (Phys.Used/Tot -- Virtual Used/Tot) which the function
then returns. I use this function to collect Memory when graphics is not called.
*/
char* memoryInfo(){

    struct sysinfo memInfo;
    sysinfo(&memInfo);

    //getting the following values and converting them to GB
    double totalPhysicalGB = (memInfo.totalram )/ (1024.00*1024.00*1024.00);
    double physicalUsedGB = (memInfo.totalram - memInfo.freeram)/ (1024.00*1024.00*1024.00);
    double totalVirtualGB = (memInfo.totalram + memInfo.totalswap)/ (1024.00*1024.00*1024.00);
    double virtualUsedGB = (memInfo.totalram + memInfo.totalswap - memInfo.freeswap - memInfo.freeram)/(1024.00*1024.00*1024.00);

    //string to store the statement in and return
    char *buffer=(char*)malloc(100);

    //(Phys.Used/Tot -- Virtual Used/Tot)
    snprintf(buffer, 100, "%.2lf GB / %.2lf GB -- %.2lf GB / %.2lf GB", physicalUsedGB, totalPhysicalGB, virtualUsedGB, totalVirtualGB);
    return buffer;

}

/*
    This function takes in a pointer to the struct of type virtualMem.
It then collects the total physical, physically used, total virtual, and virtually used memory (and coverts them to GB)
Then it takes those values and stores them in a string with this structure: (Phys.Used/Tot -- Virtual Used/Tot) which the function
then returns. The function also updates the field value of virtualValue to the value of VirtualUsedGB that has just been calculated
I use this function to collect Memory when graphics is called.
*/
char* memoryInfoGraphics(virtualMem *prev){

    struct sysinfo memInfo;
    sysinfo(&memInfo);

    //getting the following values and converting them to GB
    double totalPhysicalGB = (memInfo.totalram )/ (1024.00*1024.00*1024.00);
    double physicalUsedGB = (memInfo.totalram - memInfo.freeram)/ (1024.00*1024.00*1024.00);
    double totalVirtualGB = (memInfo.totalram + memInfo.totalswap)/ (1024.00*1024.00*1024.00);
    double virtualUsedGB = (memInfo.totalram + memInfo.totalswap - memInfo.freeswap - memInfo.freeram)/(1024.00*1024.00*1024.00);

    //string to store the statement in and return
    char *buffer=(char*)malloc(100);

    //(Phys.Used/Tot -- Virtual Used/Tot)
    snprintf(buffer, 100, "%.2lf GB / %.2lf GB -- %.2lf GB / %.2lf GB", physicalUsedGB, totalPhysicalGB, virtualUsedGB, totalVirtualGB);

    //storing the virtual memory used value in the field virtualValue of the node pointer
    prev->virtualValue = virtualUsedGB;

    return buffer;

}

/*
    This function takes in three integers: samples representing the number of samples to be printed, delay representing the time delay
between each sample in seconds and another indicated if the option of sequential has been chosed by the user (listed respectively).
seqFlag = 1 if 'sequential' has been chosen and 0 if not.
This function is responsible for printing out the Memory usage in kilobytes and if sequential is not chosen, printing out he number of
samples and the delay in seconds.
*/
void firstLine(int samples, int delay, int seqFlag){

    //if not sequential then this line does print
    if(seqFlag == 0){
        printf("Nbr of samples: %d -- every %d secs\n", samples, delay);
    }

    //second line: memory usage
    struct rusage memUsage;
    int r = getrusage(RUSAGE_SELF,&memUsage);
    if(r == 0){
        printf("Memory usage: %ld kilobytes\n",memUsage.ru_maxrss);
    }else{
        printf("Error in getrusage");
    }

    printf("--------------------------------------\n");
}

/*
    This function takes in two integers: the number of samples requested / to print
and another representing what iteration the printing is on. The third parameter is a char** array (an array
of strings) which holds all the memory statements.
The function's responsibility is to print 'ind' amount of statements and print out newline characters
for (samples - 1 - i) amount of times
*/
void printingMemory(int samples, int ind, char** memArr){
    //number of newlines to print after
    int newlines = samples - ind - 1;

    //traversing through and printing the array values ind times
    for(int i = 0; i <= ind; i++){
        printf("%s\n", memArr[i]);
    }
    for(int i = 0; i < newlines; i++){
        printf("\n");
    }
}

/*
    This function takes in one integer representing the number of samples requested. I then creates an array of type char** : an
array of strings. The array has samples number of strings, each that can hold 200 characters. The function then returns this array.
*/
char ** allocateArray(int samples){
    char **arr;

    //array can hold samples number of strings
    arr = malloc(samples * sizeof(char*));

    //each string can hold 200 characters
    for(int i = 0; i < samples; i++){
        arr[i] = malloc(sizeof(char) *200);
    }
    return arr;
}

/*
    This function takes an array of strings (char**) that is to be freed and the number of samples which correlates to how many
strings are stored in the array.
This function then frees the array.
*/
void freeArray(char**arrayToFree, int samples){

    for (int i = 0; i < samples; i++) {
        free(arrayToFree[i]);
    }

    free(arrayToFree);

}

/*
    This function takes in four integers representing if the system flag has been called,
if the user flag, has been called, the number of samples to print, and the delay between them in seconds.
It is then responsible for printing all information that is seen when the sequential and graphics flags are
not used.
*/
void printInfo(int systemFlag, int userFlag, int samples, int delay){

    //creating an array to store the memory statements: (Phys.Used/Tot -- Virtual Used/Tot)
    char**memArr = allocateArray(samples);
    //node to keep track of the previous cpu delta total and delta idle
    cpuStat *prev = new_cpuStat();

    //for loop iterating through samples number of times
    for(int ind = 0; ind < samples; ind++){
        //clears the screen and prints again so it looks like the memory statements and possibly more users are the only
        //thing being added
        system("clear");

        if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            firstLine(samples, delay, 0);
        }
        if(systemFlag == 1|| (systemFlag == 0 && userFlag == 0)){
            //each new memory statement is being added to the array
            *(memArr + ind) = memoryInfo();
            printingMemory(samples, ind, memArr);
        }
        if(userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            userInfo();
        }
        if(systemFlag == 1 ||  (systemFlag == 0 && userFlag == 0)){
            //setting the pointer equal to updated pointer with the new delta total and delta idle values
            prev = coresCPU(ind, prev);
        }
        sleep(delay);
    }
    //this is after the loop because as seen in the demo videos, it always prints at the end
    if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
        systemInfo();
    }
    //frees the array as it's use is now done
    //to prevent memory leaks
    freeArray(memArr, samples);
}

/*
    This function takes two integers representing the number of samples and one representing the nth iteration. In this
case it represents what line it has to prints on + 1. For example if ind is 0, then the statement is printing
in the first line. This function also takes care of printing the correct number of newlines for before and after the statement.
*/
void printingMemorySeq(int samples, int ind){
    //number of newlines to print before statement
    int before = ind;
    //number of newlines to print after statement
    int after = samples - (ind + 1);

    char *seqMemory;

    for(int i = 0; i < ind; i++){
        printf("\n");
    }
    //calling memoryInfo to retreive the memory statement to print
    seqMemory = memoryInfo();
    printf("%s\n", seqMemory);

    for(int i = 0; i < after; i++){
        printf("\n");
    }

}

/*
    This function takes in four integers representing if the system flag has been called,
if the user flag, has been called, the number of samples to print, and the delay between them in seconds.
It is then responsible for printing all information that is seen when the sequential flag is chosen and graphics flags is
not.
*/
void printSeq(int systemFlag, int userFlag, int samples, int delay){

    //setting up this node to be able to call coresCPU
    //cannot just send NULL because that would then cause an error since coresCPU accesses prev's fields
    cpuStat *prev = new_cpuStat();

    system("clear");

    for(int ind = 0; ind < samples; ind++){
        //printing what iteration this is
        printf(">>> iteration %d\n", (ind + 1));

        if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            firstLine(samples, delay, 1);
        }
        if(systemFlag == 1|| (systemFlag == 0 && userFlag == 0)){
            printingMemorySeq(samples, ind);
        }
        if(userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            userInfo();
        }
        if(systemFlag == 1 ||  (systemFlag == 0 && userFlag == 0)){
            prev = coresCPU(ind, prev);
        }
        sleep(delay);
    }
    if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
        systemInfo();
    }
    free(prev);
}

/*
    This function takes in one integer: the number representing what iteration is running / what percentage's graphics
we are getting. For example if ind = 0, we are retrieving the graphics for the first CPU percentage. It also takes a pointer pointing
to a node of type cpuStat that contains information from the previous iteration, and two doubles where
prev_perc represents the previous iteration's cpu usage percentage and current iteration's cpu usage percentage.
This function decides how many bars should be displayed for the current cpu percentage by finding the difference between prev_perc and
curr_perc. It then returns the string contains the bars and percentage.
*/
char* addingCPUgraphics(int ind, cpuStat *prev, double prev_perc, double curr_perc){
    //creating a string to store the graphic statement we want and then return
    char *cpuString = (char*)malloc(500);
    int diff_perc;
    char bars[200] = "";
    if(ind == 0){ //if it's the first iteration print out 9 bars
        strcat(bars,"|||||||||");
    }else{
        //calculating the difference
        diff_perc = curr_perc - prev_perc;

        //changing the node's bar values depending on the difference
        prev->bars = prev->bars + diff_perc;

        //adding the according number of bars to the string
        for(int i = 0; i < prev->bars; i++){
            strcat(bars,"|");
        }
    }
    //joining all values together and storing it in cpuString
    snprintf(cpuString, 500, "%s %.2lf", bars, curr_perc);
    return cpuString;
}

/*
    This function takes in two integers: the number of samples requested / to print, and the nth iteration.
In this case ind will represent how many CPU graphic lines in the format: (bars cpu%). The function also takes in the array
of strings to be printed.
*/
void printingCPUgraphics(int samples, int ind, char**CPUgraph){
    int newlines = samples - ind - 1;

    //traversing through and printing the array values ind times
    for(int i = 0; i <= ind; i++){
        printf("%s\n", CPUgraph[i]);
    }
    //printing the according number of newline characters after
    for(int i = 0; i < newlines; i++){
        printf("\n");
    }
}

/*
    This function takes in two integers, samples representing the number of samples to be printed and one representing
the nth iteration. In this case it represents what line it has to prints on + 1. For example if ind is 0, then the
statement is printing in the first line. seqCPUgraphics represents the string to be printed on the (ind + 1)th line.
This function also takes care of the number of newline characters to be printed before and after the statement
*/
void printingCPUgraphicsSeq(int samples, int ind, char *seqCPUgraphics){
    //number of newline characters before statement
    int before = ind;
    //number of newline character after statement
    int after = samples - (ind + 1);

    for(int i = 0; i < ind; i++){
        printf("\n");
    }

    //printing the string
    printf("%s\n", seqCPUgraphics);

    for(int i = 0; i < after; i++){
        printf("\n");
    }

}

/*
    This function takes in two integers: the number of samples to print, and the nth iteration that is running.
In this case ind will represent how many memory statements in the format: (Phys.Used/Tot -- Virtual Used/Tot)
is being printed. It also takes two arrays of strings. memArr represents all the memory statements stored and memArrGraphics
represents the corresponding graphics for the memory statements. The function then prints (ind + 1) number of strings up to memArr[ind]
and memArrGraphics[ind]. It also then prints out the corresponding number of newline characters after.
*/
void printingMemoryGraphics(int samples, int ind, char** memArr, char** memArrGraphics){
    int newlines = samples - ind - 1;

    //traversing through and printing the array values ind times
    for(int i = 0; i <= ind; i++){
        printf("%s", memArr[i]);
        printf("%s\n", memArrGraphics[i]);
    }
    //printing the according number of newline characters after
    for(int i = 0; i < newlines; i++){
        printf("\n");
    }
}

/*
    This function takes in two integers: the number of samples to print, and the nth iteration that is running. In this case
ind will represent what line the strings will print at + 1. For example if ind = 0, the string will print at line 1.
 It also takes two char*, strings. seqMemory represents the strings that contains the different memory values in the following
format: (Phys.Used/Tot -- Virtual Used/Tot).
seqMemGraphics represents the corresponding graphics. The function's responsibility is to print them beside each other. The function also prints
the corresponding number of newline characters before and after printing the strings.
*/
void printingMemoryGraphicsSeq(int samples, int ind, char *seqMemory, char *seqMemGraphics){
    //number of newline characters before
    int before = ind;
    //number of newline characters after
    int after = samples - (ind + 1);

    //printing newlines
    for(int i = 0; i < ind; i++){
        printf("\n");
    }
    //printing the strings
    printf("%s %s\n", seqMemory, seqMemGraphics);

    //printing newlines
    for(int i = 0; i < after; i++){
        printf("\n");
    }

}

/*
    This function takes in one integer: the number representing what iteration is running / what line's graphics
we are getting. For example if ind = 0, we are retrieving the graphics for the first memory statement. It also takes two doubles:
prev_virtual representing the previous iterations virtual used memory and curr_virtual representing the current iterations virtual used memory.
This function calculates the difference between the two virtual memories and stores the respective symbols. For example # for a positive change
and : for a negative one. Each symbol also represents a 0.01 change. To note '|o' represents zero+ and '|@' represents zero-
*/
char* addingMemoryGraphics(int ind, double prev_virtual, double curr_virtual){
    char *memoryString = (char*)malloc(500);
    char saveCurr[10];
    char savePrev[10];

    //saving curr_vitual to a string with two decimal places
    snprintf(saveCurr, 10, "%.2lf", curr_virtual);
    //converting the string to a double so that now the double is rounded to two decimal places
    curr_virtual = strtod(saveCurr, NULL);
    //saving prev_vitual to a string with two decimal places
    snprintf(savePrev, 10, "%.2lf", prev_virtual);
     //converting the string to a double so that now the double is rounded to two decimal places
    prev_virtual = strtod(savePrev, NULL);

    //calculating the difference
    double diff_mem = curr_virtual - prev_virtual;
    char saveDiff[10];
    //saving diff_mem to a string with two decimal places
    snprintf(saveDiff, 10, "%.2lf", diff_mem);
    //converting the string to a double so that now the double is rounded to two decimal places
    diff_mem = strtod(saveDiff, NULL);

    //getting the number of symbols to print
    int numSymbols = diff_mem*100.00;

    char symbols[200] = "|";
    if(ind == 0){ //if its the first iteration, there's no memory to compare to, thus the '|o'
        strcat(symbols, "o");
        snprintf(memoryString, 500, "%s %.2lf (%.2lf)", symbols, 0.00, curr_virtual);
    }else{

        //if positive change
        if(diff_mem > 0.00){
            //# numSymbols amount of times
            for(int i = 0; i < numSymbols; i++){
                strcat(symbols,"#");
            }
            snprintf(memoryString, 500, "%s* %.2lf (%.2lf)", symbols, diff_mem, curr_virtual);
        }else if(diff_mem == 0.0){ //if positive zero
            snprintf(memoryString, 500, "%so %.2lf (%.2lf)", symbols, diff_mem, curr_virtual);
        }
        else{ //if negative change or negative zero
            //absolute value of numSymbols so that the for loops runs
            numSymbols = abs(numSymbols);
            for(int i = 0; i < numSymbols; i++){
                strcat(symbols,":");
            }
            snprintf(memoryString, 500, "%s@ %.2lf (%.2lf)", symbols, diff_mem, curr_virtual);
        }
    }
    return memoryString;
}

/*
    This function is to print the information if the user chose graphics but not sequential. It takes in four integers representing
if the system flag has been called,if the user flag, has been called, the number of samples to print, and the delay between them
in seconds.
*/
void printInfoG(int systemFlag, int userFlag, int samples, int delay){
    //setting up the array of strings
    char**memArr = allocateArray(samples);
    char**memArrGraphics = allocateArray(samples);
    char**CPUgraph = allocateArray(samples);

    //setting up arrays to keep save the previous iteration's values
    double percentages[samples];
    double virtuals[samples];

    //setting up nodes to retreive the previous iteration's values
    cpuStat *prev = new_cpuStat();
    virtualMem *prev_mem = new_virtualMem();

    for(int ind = 0; ind < samples; ind++){
        system("clear");

        if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            firstLine(samples, delay, 0);
        }
        if(systemFlag == 1|| (systemFlag == 0 && userFlag == 0)){
            //setting counter's index of the array equal to the memory statement: (Phys.Used/Tot -- Virtual Used/Tot)
            *(memArr + ind) = memoryInfoGraphics(prev_mem);
            //saving the current virtual value in the array
            virtuals[ind] = prev_mem->virtualValue;

            //saving the graphics string for memory in the array of strings
            if(ind == 0){
                //seperate if statement because virtuals[-1] does not exist so we send 0.0 in its place
                *(memArrGraphics + ind) = addingMemoryGraphics(ind, 0.0, virtuals[ind]);
            }else{
                *(memArrGraphics + ind) = addingMemoryGraphics(ind, virtuals[ind - 1], virtuals[ind]);
            }
            printingMemoryGraphics(samples, ind, memArr, memArrGraphics);
        }
        if(userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            userInfo();
        }
        if(systemFlag == 1 ||  (systemFlag == 0 && userFlag == 0)){
            prev = coresCPU(ind, prev);
            //saving the current percentage value in the array
            percentages[ind] = prev->cpu_perc;

            //saving the graphivs string for CPU in the array of strings
            if(ind != 0){
                *(CPUgraph + ind) = addingCPUgraphics(ind, prev, percentages[ind - 1], percentages[ind]);
            }else{
                *(CPUgraph + ind) = addingCPUgraphics(ind, prev, 0.00, percentages[ind]);
            }
            printingCPUgraphics(samples, ind, CPUgraph);
        }
        sleep(delay);
    }
    if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
        systemInfo();
    }

    //freeing arrays to prevent memory leaks
    freeArray(memArr, samples);
    freeArray(memArrGraphics, samples);
    freeArray(CPUgraph, samples);
}

/*
    This function is to print the information if the user chose graphics and sequential. It takes in four integers representing
if the system flag has been called,if the user flag, has been called, the number of samples to print, and the delay between them
in seconds.
*/
void printSeqG(int systemFlag, int userFlag, int samples, int delay){
    cpuStat *prev = new_cpuStat();
    double percentages[samples];
    double virtuals[samples];
    virtualMem *prev_mem = new_virtualMem();

    //string declarations
    char *seqCPUgraphics;
    char *seqMemGraphics;
    char *seqMemory;

    system("clear");

    for(int ind = 0; ind < samples; ind++){
        printf(">>> iteration %d\n", (ind + 1));
        if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            firstLine(samples, delay, 1);
        }
        if(systemFlag == 1|| (systemFlag == 0 && userFlag == 0)){
            //setting string to memory statemnt
            seqMemory = memoryInfoGraphics(prev_mem);
            //saving the current virtual used memory in array
            virtuals[ind] = prev_mem->virtualValue;

            //setting string to memory graphics
            if(ind == 0){
                //if statement to prevent virtuals[-1] so we send 0.0 in its place
                seqMemGraphics = addingMemoryGraphics(ind, 0.0, virtuals[ind]);
            }else{
                seqMemGraphics = addingMemoryGraphics(ind, virtuals[ind - 1], virtuals[ind]);
            }

            printingMemoryGraphicsSeq(samples, ind, seqMemory, seqMemGraphics);

        }
        if(userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
            userInfo();
        }
        if(systemFlag == 1 ||  (systemFlag == 0 && userFlag == 0)){
            prev = coresCPU(ind, prev);
            //saveing the current cpu percentage in array
            percentages[ind] = prev->cpu_perc;
            if(ind != 0){
                seqCPUgraphics = addingCPUgraphics(ind, prev, percentages[ind - 1], percentages[ind]);
            }else{
                //if statement to prevent virtuals[-1] so we send 0.0 in its place
                seqCPUgraphics = addingCPUgraphics(ind, prev, 0.00, percentages[ind]);
            }
            printingCPUgraphicsSeq(samples, ind, seqCPUgraphics);
        }
        sleep(delay);
    }
    if(systemFlag == 1 || userFlag == 1 || (systemFlag == 0 && userFlag == 0)){
        systemInfo();
    }
    //freeing pointers to prevent memory leaks
    free(seqCPUgraphics);
    free(seqMemGraphics);
    free(seqMemory);
}

/*
    In the main function, I focused on getting the command line arguments and then sending the program to it's
respective printing functions.
I used getopt_long to be able to get the different flags no matter what order the user inputs it in.
Using getopt_long also helped me get the positional arguments as well (by using optind).
*/
int main(int argc, char **argv){

    //setting samples and tdelay to the default values
    //so that if the user doesn't specify a change they will be equal to the following
    int samples = 10;
    int delay = 1;

    //getopt
    //setting the flags to = 0
    int option_index = 0;
    int systemFlag = 0;
    int userFlag = 0;
    int seqFlag = 0;
    int tdelFlag = 0;
    int sampFlag = 0;
    int graphFlag = 0;
    static struct option long_options[] = {
        {"system", no_argument, NULL, 's'},
        {"user", no_argument, NULL, 'u'},
        {"sequential", no_argument, NULL, 'q'},
        {"tdelay", optional_argument, NULL, 't'},
        {"samples", optional_argument, NULL, 'n'},
        {"graphics", no_argument, NULL, 'g'},
        {NULL, 0, NULL, 0}
    };
    //setting c to 0 so it at least does through the while loop once
    int c = 0;
    //getopt cases and setting flag values and getting number valeus
    while(c != -1){
        c = getopt_long(argc, argv, "suqt::n::", long_options, NULL);
        switch(c){
            case 's':
                systemFlag = 1;
                break;
            case 'u':
                userFlag = 1;
                break;
            case 'q':
                seqFlag = 1;
                break;
            case 't':
                tdelFlag = 1;
                if(optarg){
                    delay = atoi(optarg);
                }
                break;
            case 'n':
                sampFlag = 1;
                if(optarg){
                    samples = atoi(optarg);
                }
                break;
            case 'g':
                graphFlag = 1;
                break;
        }
    }
    //positional arguments
    int param = optind;
    if (optind < argc) {
        while (param < argc){
            if(param == optind){
                samples = atoi(argv[param]);
            }
            if(param == (optind + 1)){
                delay = atoi(argv[param]);

            }
            param++;
        }
    }

    //if graphics isn't chosen it'll send to either regular printing or regular sequential printing
    if(graphFlag == 0){
        if(seqFlag == 0){
            printInfo(systemFlag, userFlag, samples, delay);
        }else{
            printSeq(systemFlag, userFlag, samples, delay);
        }
    }else{
        //if graphics is chosen it'll send to either graphics printing or sequential graphics printing
        if(seqFlag == 0){
            printInfoG(systemFlag, userFlag, samples, delay);
        }else{
            printSeqG(systemFlag, userFlag, samples, delay);
        }

    }

    return 0;
}
