# System Monitoring Tool (Linux)
**A course project for CSCB09: Software Tools and Systems Programming**

C program that will report different metrics of utilization of a given system as described below

The program takes the following command lines:

```--system``` to indicate that only the system usage should be generated


```--user``` to indicate that only the users usage should be generated


```--graphics``` to include graphical output in the cases where a graphical outcome is possible.

The following graphics will appear for Memory utilization:
```
::::::@  total relative negative change
######*  total relative positive change
```


and for CPU utilization:
```
||||   positive percentage increase
```


```--sequential``` to indicate that the information will be output sequentially without needing to "refresh" the screen (useful if you would like to redirect the output into a file)

 
```--samples=N ```if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions.
If not value is indicated the default value will be 10.


```--tdelay=T``` to indicate how frequently to sample in seconds.
If not value is indicated the default value will be 1 sec.


The last two arguments can also be considered as positional arguments if not flag is indicated in the corresponding order: samples tdelay.


The reported "stats" include:

 - A user usage report on how many users are connected in a given time and how many sessions each user is connected to 
    
- A system usage report on how much utilization of the CPU is being done and how much utilization of memory is being done (report used and free memory).
