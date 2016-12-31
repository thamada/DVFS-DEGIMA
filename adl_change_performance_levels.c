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
typedef int ( *ADL_OVERDRIVE5_ODPERFORMANCELEVELS_SET) (int, ADLODPerformanceLevels*);

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
  int   cpl_level;
  int   cpl_engine_clock;
  int   cpl_memory_clock;
  int   cpl_core_voltage;
  int   cpl_within_range;
  void *hDLL; // This will be the handle to the .so library
  int   number_adapters;
  int   number_performance_levels;
  

  // Get adapter index handle, performance levels, and performance values
  if (argc < 6) {
    printf("Adapter's ADL index handle must be provided as argument!\n");
    printf("Call program as: PROGRAM <ADL index handle> <perf level> <engine clock> <memory clock> <core voltage>\n");
    return 0;
  } else {
    query_adapter_number = atoi(argv[1]);
    cpl_level            = atoi(argv[2]);
    cpl_engine_clock     = atoi(argv[3]);
    cpl_memory_clock     = atoi(argv[4]);
    cpl_core_voltage     = atoi(argv[5]);
  }

  ADL_MAIN_CONTROL_CREATE                ADL_Main_Control_Create;
  ADL_MAIN_CONTROL_DESTROY               ADL_Main_Control_Destroy;
  ADL_ADAPTER_NUMBEROFADAPTERS_GET       ADL_Adapter_NumberOfAdapters_Get;
  ADL_OVERDRIVE5_ODPARAMETERS_GET        ADL_Overdrive5_ODParameters_Get;
  ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET ADL_Overdrive5_ODPerformanceLevels_Get;
  ADL_OVERDRIVE5_ODPERFORMANCELEVELS_SET ADL_Overdrive5_ODPerformanceLevels_Set;

  // Load ADL library
  hDLL = dlopen("libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
  if (hDLL == NULL) {
    printf("ADL Library was not found.\n");
    return 0;
  }
  
  // Obtain address of functions
  ADL_Main_Control_Create          = (ADL_MAIN_CONTROL_CREATE) dlsym(hDLL, "ADL_Main_Control_Create");
  ADL_Main_Control_Destroy         = (ADL_MAIN_CONTROL_DESTROY) dlsym(hDLL, "ADL_Main_Control_Destroy");
  ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) dlsym(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
  ADL_Overdrive5_ODParameters_Get  = (ADL_OVERDRIVE5_ODPARAMETERS_GET) dlsym(hDLL, "ADL_Overdrive5_ODParameters_Get");
  ADL_Overdrive5_ODPerformanceLevels_Get = (ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET) dlsym(hDLL, "ADL_Overdrive5_ODPerformanceLevels_Get");
  ADL_Overdrive5_ODPerformanceLevels_Set = (ADL_OVERDRIVE5_ODPERFORMANCELEVELS_SET) dlsym(hDLL, "ADL_Overdrive5_ODPerformanceLevels_Set");
  
  // Before using ADL we need to initialize the library
  if ( ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) ) {
    printf("[ADL] Failed to initialize.\n");
    return 0;
  }

  // Get the number of adapters
  ADL_Adapter_NumberOfAdapters_Get(&number_adapters);
  if (query_adapter_number < 0 || query_adapter_number > number_adapters) {
    printf("Not valid adapter number.\n");
    return 0;
  }

  // Get Adapter paramters
  ADLODParameters *lp_adlod_parameters = malloc(sizeof(ADLODParameters));
  status_adl = ADL_Overdrive5_ODParameters_Get(query_adapter_number, lp_adlod_parameters);
  if (ADL_OK == status_adl ) {
    number_performance_levels = lp_adlod_parameters->iNumberOfPerformanceLevels;
    // check that new performance values are within accepted range
    cpl_within_range = 0;
    if (0 <= cpl_level) cpl_within_range++; else printf("Level must be > 0\n");
    if (lp_adlod_parameters->iNumberOfPerformanceLevels > cpl_level) cpl_within_range++; else printf("Level must be < MAX\n");
    if (lp_adlod_parameters->sEngineClock.iMin <= cpl_engine_clock) cpl_within_range++; else printf("Engine clock must be >= MIN\n");
    if (lp_adlod_parameters->sEngineClock.iMax >= cpl_engine_clock) cpl_within_range++; else printf("Engine clock must be <= MAX\n");
    if (lp_adlod_parameters->sMemoryClock.iMin <= cpl_memory_clock) cpl_within_range++; else printf("Memory clock must be >= MIN\n");
    if (lp_adlod_parameters->sMemoryClock.iMax >= cpl_memory_clock) cpl_within_range++; else printf("Memory clock must be <= MAX\n");
    if (lp_adlod_parameters->sVddc.iMin <= cpl_core_voltage) cpl_within_range++; else printf("Core Voltage must be >= MIN\n");
    if (lp_adlod_parameters->sVddc.iMax >= cpl_core_voltage) cpl_within_range++; else printf("Core Voltage must be <= MAX\n");
    if (cpl_within_range < 8) {
      printf("Performance values are not within the adapter's range.\n");
      return 0;
    }
  } else {
    printf("Can't retrieve adapter %d parameters. Error code: %d\n", query_adapter_number, status_adl);
    return 0;
  }

  // Get current performance levels
  // Performance levels correspond to the states of the GPU may be in. Typically: idle, moderate, and busy.
  // Each state had its own engine clock, memory clock, and core voltage.
  ADLODPerformanceLevels *lp_performance_levels = malloc(sizeof(ADLODPerformanceLevels)+sizeof(ADLODPerformanceLevel)*(number_performance_levels-1));
  lp_performance_levels->iSize = sizeof(ADLODPerformanceLevels)+sizeof(ADLODPerformanceLevel)*(number_performance_levels-1);
  // Show the current performance levels
  status_adl = ADL_Overdrive5_ODPerformanceLevels_Get(query_adapter_number, 0, lp_performance_levels);
  if (ADL_OK == status_adl) {
    // Update data structure
    lp_performance_levels->aLevels[cpl_level].iEngineClock = cpl_engine_clock;
    lp_performance_levels->aLevels[cpl_level].iMemoryClock = cpl_memory_clock;
    lp_performance_levels->aLevels[cpl_level].iVddc        = cpl_core_voltage;
    // Set new performance values
    status_adl = ADL_Overdrive5_ODPerformanceLevels_Set(query_adapter_number, lp_performance_levels);
    if (ADL_OK == status_adl) {
      printf("New performance values are set!\n");
    } else {
      printf("Failed to set new performance values. Error code: %d\n", status_adl);
    }
  } else { 
    printf("Can't retrieve adapter %d current performance levels. Error code: %d\n", query_adapter_number, status_adl);
  }

  // release resources
  free(lp_adlod_parameters);

  // Release all ADL global pointers
  ADL_Main_Control_Destroy();

  dlclose(hDLL);

  return 0;
}



