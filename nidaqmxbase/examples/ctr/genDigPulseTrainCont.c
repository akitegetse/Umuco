/*********************************************************************
*
* ANSI C Example program:
*    genDigPulseTrainCont.c
*
* Example Category:
*    CO
*
* Description:
*    This example demonstrates how to generate a continuous digital
*    pulse train from a Counter Output Channel. The Frequency, and
*    Duty Cycle are all configurable.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to output your signal to on the DAQ device.
*    2. Enter the Frequency and Duty Cycle to define the pulse
*       parameters.
*       Additionally, you can set the Initial Delay (in seconds)
*       which will delay the beginning of the pulse train from the
*       start call; this is currently set to 0.0 in the code.
*    Note: Use the buffPeriodFinite example to verify you are
*          outputting the pulse train on the DAQ device.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Output channel to produce a Pulse in terms
*       of Frequency.
*    3. Call the timing function to configure the duration of the
*       pulse generation.
*    4. Call the Start function to arm the counter and begin the
*       pulse train generation.
*    5. For continuous generation, the counter will continually
*       generate the pulse train until told to stop. This will happen
*       after 10 seconds by default.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will output the pulse train on the output terminal
*    of the counter specified in the Physical Channel I/O control.
*
*    In this example the output will be sent to the default output
*    terminal on ctr0.
*
*    For more information on the default counter input and output
*    terminals for your device, open the NI-DAQmxBase Help, and refer
*    to Counter Signal Connections found under the Device Considerations
*    book in the table of contents.
*
* Recommended Use:
*    Call Configure and Start functions.
*    Call Write function in a loop.
*    Call Stop function at the end.
*
*********************************************************************/

#include <nidaqmxbase/NIDAQmxBase.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define DAQmxErrChk(functionCall) { if( DAQmxFailed(error=(functionCall)) ) { goto Error; } }

static int gRunning=0;

int main(void)
{
    // Task parameters
    int32       error = 0;
    TaskHandle  taskHandle = 0;
    char        errBuff[2048]={'\0'};
    time_t      startTime;

    // Channel parameters
    char        chan[] = "Dev1/ctr0";
    float64     freq = 1.0;
    float64     duty = 0.5;
    float64     delay = 0.0;

    // Data write parameters
    float64     data = 0.0;
    float64     timeout = 10.0;
    bool32      done = 0;


    DAQmxErrChk (DAQmxBaseCreateTask("",&taskHandle));
    DAQmxErrChk (DAQmxBaseCreateCOPulseChanFreq(taskHandle,chan,"",DAQmx_Val_Hz,DAQmx_Val_Low,delay,freq,duty));
    DAQmxErrChk (DAQmxBaseCfgImplicitTiming(taskHandle,DAQmx_Val_ContSamps,1000));
    DAQmxErrChk (DAQmxBaseStartTask(taskHandle));
    gRunning = 1;
    // The loop will quit after 10 seconds
    startTime = time(NULL);
    while( gRunning && !done && time(NULL)<startTime+10 ) {
        DAQmxErrChk (DAQmxBaseIsTaskDone(taskHandle,&done));
        if( !done )
            usleep(100000);
    }

Error:
    if( DAQmxFailed(error) )
        DAQmxBaseGetExtendedErrorInfo(errBuff,2048);
    if( taskHandle!=0 ) {
        DAQmxBaseStopTask(taskHandle);
        DAQmxBaseClearTask(taskHandle);
    }
    if( DAQmxFailed(error) )
		printf ("DAQmxBase Error %ld: %s\n", error, errBuff);
    return 0;
}
