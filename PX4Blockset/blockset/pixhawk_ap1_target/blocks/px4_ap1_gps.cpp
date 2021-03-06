/*
 *   C-MEX S-function for function call.
 *   px4_ap1_gps.cpp
 *
 */

#define S_FUNCTION_NAME                px4_ap1_gps
#define S_FUNCTION_LEVEL               2

#include "simstruc.h"

/* Ensure that this S-Function is compiled with a C++ compiler */
#ifndef __cplusplus
#error This S-Function must be compiled with a C++ compiler. Enter mex('-setup') in the MATLAB Command Window to configure a C++ compiler.
#endif

static void mdlCheckParameters(SimStruct *S)
{
    
}

/* Function: mdlInitializeSizes ===========================================
 * Abstract:
 *    The sizes information is used by Simulink to determine the S-function
 *    block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    int cntPorts = 1;
	int bLatLon = 0;
	int bSpeed = 0;
	int bCourse = 0;
	int bTime = 0;
	int bDate = 0;
	
    int portIdx = 0;

    // /* Number of expected parameters */
    ssSetNumSFcnParams(S, 6);
    
    if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S))
    {
        mdlCheckParameters(S);
        if (ssGetErrorStatus(S) != NULL)
        {
            return;
        }
    }
    
  /*
   * Set the number of work vectors.
   */
    if (!ssSetNumDWork(S, 0))
        return;
    
    
    ssSetNumPWork(S, 0);
    
  /*
   * Set the number of input ports.
   */
    if (!ssSetNumInputPorts(S, 0))
        return;
    
  /*
   * Set the number and data type of output ports.
   */
    
	real_T * par = mxGetPr(ssGetSFcnParam(S, 0));
	bLatLon = (int)(*par) != 0;
	
	par = mxGetPr(ssGetSFcnParam(S, 1));
	bSpeed = (int)(*par) != 0;
	
	par = mxGetPr(ssGetSFcnParam(S, 2));
	bCourse = (int)(*par) != 0;
	
	par = mxGetPr(ssGetSFcnParam(S, 3));
	bTime = (int)(*par) != 0;
	
	par = mxGetPr(ssGetSFcnParam(S, 4));
	bDate = (int)(*par) != 0;
	
	if(bLatLon) {cntPorts += 2;}
	if(bSpeed) 	{cntPorts += 1;}
	if(bCourse) {cntPorts += 1;}
	if(bTime)	{cntPorts += 3;}
	if(bDate) 	{cntPorts += 3;}

    if (!ssSetNumOutputPorts(S, cntPorts))
        return;
    
	// validity
	ssSetOutputPortDataType(S, portIdx, SS_BOOLEAN);
    ssSetOutputPortWidth(S, portIdx, 1);
	portIdx++;
		
	//Latitude + Longitude
    if(bLatLon)
	{
		for(int i = portIdx; i < portIdx + 2; i++)
		{
			ssSetOutputPortDataType(S, i, SS_SINGLE);
			ssSetOutputPortWidth(S, i, 1);
		}
		portIdx = portIdx + 2;
	}
	
	if(bSpeed)
	{
		ssSetOutputPortDataType(S, portIdx, SS_SINGLE);
        ssSetOutputPortWidth(S, portIdx, 1);
		portIdx++;
	}
	
	if(bCourse)
	{
		ssSetOutputPortDataType(S, portIdx, SS_SINGLE);
        ssSetOutputPortWidth(S, portIdx, 1);
		portIdx++;
	}
	
	if(bTime)
	{
		for(int i = portIdx; i < portIdx + 3; i++)
		{
			ssSetOutputPortDataType(S, i, SS_UINT8);
			ssSetOutputPortWidth(S, i, 1);
		}
		portIdx = portIdx + 3;
	}
	
	if(bDate)
	{
		for(int i = portIdx; i < portIdx + 3; i++)
		{
			ssSetOutputPortDataType(S, i, SS_UINT8);
			ssSetOutputPortWidth(S, i, 1);
		}
		portIdx = portIdx + 3;
	}
    
  /*
   * This S-function can be used in referenced model simulating in normal mode.
   */
    ssSetModelReferenceNormalModeSupport(S, MDL_START_AND_MDL_PROCESS_PARAMS_OK);
    
  /*
   * Set the number of sample time.
   */
    ssSetNumSampleTimes(S, 1);
    
  /*
   * All options have the form SS_OPTION_<name> and are documented in
   * matlabroot/simulink/include/simstruc.h. The options should be
   * bitwise or'd together as in
   *   ssSetOptions(S, (SS_OPTION_name1 | SS_OPTION_name2))
   */
    ssSetOptions(S,
    SS_OPTION_USE_TLC_WITH_ACCELERATOR |
    SS_OPTION_CAN_BE_CALLED_CONDITIONALLY |
    SS_OPTION_EXCEPTION_FREE_CODE |
    SS_OPTION_SFUNCTION_INLINED_FOR_RTW |
    SS_OPTION_DISALLOW_CONSTANT_SAMPLE_TIME);
    
}

/* Function: mdlInitializeSampleTimes =====================================
 * Abstract:
 *    This function is used to specify the sample time(s) for your
 *    S-function. You must register the same number of sample times as
 *    specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, FIXED_IN_MINOR_STEP_OFFSET);
    
    #if defined(ssSetModelReferenceSampleTimeDefaultInheritance)
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
    #endif
    
}

/* Function: mdlOutputs ===================================================
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block. Generally outputs are placed in the output vector(s),
 *    ssGetOutputPortSignal.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
    
}

/* Function: mdlTerminate =================================================
 * Abstract:
 *    In this function, you should perform any actions that are necessary
 *    at the termination of a simulation.
 */
static void mdlTerminate(SimStruct *S)
{
    
}

/*
 * Required S-function trailer
 */
#ifdef MATLAB_MEX_FILE
# include "simulink.c"
#else
# include "cg_sfun.h"
#endif
