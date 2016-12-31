#include "adl_sdk.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

// Definition of ADL functions
typedef int ( *ADL_MAIN_CONTROL_CREATE) (ADL_MAIN_MALLOC_CALLBACK, int);
typedef int ( *ADL_MAIN_CONTROL_DESTROY) ();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int ( *ADL_OVERDRIVE5_ODPARAMETERS_GET) (int, ADLODParameters*);
typedef int ( *ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET) (int, int, ADLODPerformanceLevels*);
typedef int ( *ADL_OVERDRIVE5_CURRENTACTIVITY_GET) (int, ADLPMActivity*);

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

int main(int argc, char *argv[]) {
  int   i;
  int   status_adl;
  int   query_adapter_number = 0;
  void *hDLL; // This will be the handle to the .so library
  int   number_adapters;
  int   number_performance_levels;

  // Get adapter sdl index handle from command line
  if (argc < 2) {
    printf("Adapter's ADL index handle must be provided as argument!\n");
    return 0;
  } else {
    query_adapter_number = atoi(argv[1]);
  }

  // Load ADL library
  hDLL = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
  if (hDLL == NULL) {
    printf("ADL Library was not found.\n");
    return 0;
  }
  
  ADL_MAIN_CONTROL_CREATE                ADL_Main_Control_Create;
  ADL_MAIN_CONTROL_DESTROY               ADL_Main_Control_Destroy;
  ADL_ADAPTER_NUMBEROFADAPTERS_GET       ADL_Adapter_NumberOfAdapters_Get;
  ADL_OVERDRIVE5_ODPARAMETERS_GET        ADL_Overdrive5_ODParameters_Get;
  ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET ADL_Overdrive5_ODPerformanceLevels_Get;
  ADL_OVERDRIVE5_CURRENTACTIVITY_GET     ADL_Overdrive5_CurrentActivity_Get;

  // Obtain address of functions
  ADL_Main_Control_Create                = (ADL_MAIN_CONTROL_CREATE) dlsym(hDLL, "ADL_Main_Control_Create");
  ADL_Main_Control_Destroy               = (ADL_MAIN_CONTROL_DESTROY) dlsym(hDLL, "ADL_Main_Control_Destroy");
  ADL_Adapter_NumberOfAdapters_Get       = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) dlsym(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
  ADL_Overdrive5_ODParameters_Get        = (ADL_OVERDRIVE5_ODPARAMETERS_GET) dlsym(hDLL, "ADL_Overdrive5_ODParameters_Get");
  ADL_Overdrive5_ODPerformanceLevels_Get = (ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET) dlsym(hDLL, "ADL_Overdrive5_ODPerformanceLevels_Get");
  ADL_Overdrive5_CurrentActivity_Get     = (ADL_OVERDRIVE5_CURRENTACTIVITY_GET) dlsym(hDLL, "ADL_Overdrive5_CurrentActivity_Get");

  // Before using ADL we need to initialize the library
  if ( ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) ) {
    printf("[ADL] Failed to initialize.\n");
    return 0;
  }

  // Get the number of adapters
  ADL_Adapter_NumberOfAdapters_Get(&number_adapters);
  if (query_adapter_number < 0 || query_adapter_number > number_adapters) {
    printf("Not valid adapter number.");
    return 0;
  }

  // Get Adapter paramters
  ADLODParameters *lp_adlod_parameters = malloc(sizeof(ADLODParameters));
  status_adl = ADL_Overdrive5_ODParameters_Get(query_adapter_number, lp_adlod_parameters);
  if (ADL_OK == status_adl ) {
    printf("Adapter parameters\n");
    printf("\tReporting supported : %d\n", lp_adlod_parameters->iActivityReportingSupported);
    printf("\tDiscrete performance supported : %d\n",lp_adlod_parameters->iDiscretePerformanceLevels);
    printf("\tNumber of performance levels: %d\n", lp_adlod_parameters->iNumberOfPerformanceLevels);
    printf("\tEngine clock (min,max,step): (");
    printf("%d", lp_adlod_parameters->sEngineClock.iMin);
    printf(", %d", lp_adlod_parameters->sEngineClock.iMax);
    printf(", %d)\n", lp_adlod_parameters->sEngineClock.iStep);
    printf("\tMemory clock (min,max,step): (");
    printf("%d",lp_adlod_parameters->sMemoryClock.iMin);
    printf(", %d", lp_adlod_parameters->sMemoryClock.iMax);
    printf(", %d)\n", lp_adlod_parameters->sMemoryClock.iStep);
    printf("\tCore voltage (min,max,step): (");
    printf("%d", lp_adlod_parameters->sVddc.iMin);
    printf(", %d", lp_adlod_parameters->sVddc.iMax);
    printf(", %d)\n", lp_adlod_parameters->sVddc.iStep);    
    number_performance_levels = lp_adlod_parameters->iNumberOfPerformanceLevels;
  } else {
    printf("Can't retrieve adaptor %d parameters. Error code: %d\n", query_adapter_number, status_adl);
    return 0;
  }

  // Get Adapter default performance levels
  // Performance levels correspond to the states of the GPU may be in. Typically: idle, moderate, and busy.
  // Each state had its own engine clock, memory clock, and core voltage.
  ADLODPerformanceLevels *lp_performance_levels = malloc(sizeof(ADLODPerformanceLevels)+sizeof(ADLODPerformanceLevel)*(number_performance_levels-1));
  lp_performance_levels->iSize = sizeof(ADLODPerformanceLevels)+sizeof(ADLODPerformanceLevel)*(number_performance_levels-1);
  status_adl = ADL_Overdrive5_ODPerformanceLevels_Get(query_adapter_number, 1, lp_performance_levels);
  if (ADL_OK == status_adl) {
    printf("Performance levels correspond to the states of the GPU may be in. Typically: idle, moderate, and busy.\n");
    printf("Adaptor DEFAULT performance level values:\n");
    for(i = 0; i < number_performance_levels; i++) {
      printf("\tlevel %d ", i); 
      printf("(Engine clock, Memory clock, Core voltage): (");
      printf("%d",lp_performance_levels->aLevels[i].iEngineClock);
      printf(",%d",lp_performance_levels->aLevels[i].iMemoryClock);
      printf(",%d)\n",lp_performance_levels->aLevels[i].iVddc);
    }
  } else { 
    printf("Can't retrieve adaptor %d default performance levels. Error code: %d\n", query_adapter_number, status_adl);
    return 0;
  }

  // Show the current performance levels
  status_adl = ADL_Overdrive5_ODPerformanceLevels_Get(query_adapter_number, 0, lp_performance_levels);
  if (ADL_OK == status_adl) {
    printf("Adaptor CURRENT performance level values:\n");
    for(i = 0; i < number_performance_levels; i++) {
      printf("\tLevel %d ", i); 
      printf("(Engine clock, Memory clock, Core voltage): (");
      printf("%d",lp_performance_levels->aLevels[i].iEngineClock);
      printf(",%d",lp_performance_levels->aLevels[i].iMemoryClock);
      printf(",%d)\n",lp_performance_levels->aLevels[i].iVddc);
    }
  } else { 
    printf("Can't retrieve adaptor %d current performance levels. Error code: %d\n", query_adapter_number, status_adl);
    return 0;
  }

  // Show current performance values
  ADLPMActivity *lp_current_activity = malloc(sizeof(ADLPMActivity));
  status_adl == ADL_Overdrive5_CurrentActivity_Get(query_adapter_number, lp_current_activity);
  if (ADL_OK == status_adl) {
    printf("Adaptor power management activity:\n");
    printf("\tEngine clock: %d\n", lp_current_activity->iEngineClock);
    printf("\tMemory clock: %d\n", lp_current_activity->iMemoryClock);
    printf("\tCore voltage: %d\n", lp_current_activity->iVddc);
    printf("\tActivity Percent: %d\n", lp_current_activity->iActivityPercent);
    printf("\tCurrent perf lvl: %d\n", lp_current_activity->iCurrentPerformanceLevel);
    printf("\tCurrent BUS speed: %d\n", lp_current_activity->iCurrentBusSpeed);
  } else {
    printf("Can't get adaptor power management activity state. Error code: %d", status_adl);
    return 0;
  }

  // release resources
  free(lp_adlod_parameters);
  free(lp_current_activity);

  // Release all ADL global pointers
  ADL_Main_Control_Destroy();

  dlclose(hDLL);

  return 0;
}



