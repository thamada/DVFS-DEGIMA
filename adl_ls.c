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
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) (LPAdapterInfo, int);
typedef int ( *ADL_ADAPTER_ID_GET ) (int, int*);

// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}

int main(int argc, char *argv[]) {
  int i;
  void *hDLL; // This will be the handle to the .so library

  ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
  ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;
  ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;
  ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;
  ADL_ADAPTER_ID_GET               ADL_Adapter_ID_Get;

  LPAdapterInfo lp_adapter_info = NULL;
  int lp_adapter_id;
  int number_adapters;

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
  ADL_Adapter_AdapterInfo_Get      = (ADL_ADAPTER_ADAPTERINFO_GET) dlsym(hDLL, "ADL_Adapter_AdapterInfo_Get");
  ADL_Adapter_ID_Get               = (ADL_ADAPTER_ID_GET) dlsym(hDLL, "ADL_Adapter_ID_Get");

  // Before using ADL we need to initialize the library
  if ( ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) ) {
    printf("[ADL] Failed to initialize.\n");
    return 0;
  }

  // Get the number of adapters
  ADL_Adapter_NumberOfAdapters_Get(&number_adapters);
  printf("[ADL] Number of adapters: %d\n", number_adapters);

  // Show adapter information
  if (number_adapters > 0) {
    // Allocate memory and set as empty char
    lp_adapter_info = malloc (sizeof(AdapterInfo) * number_adapters);
    memset(lp_adapter_info, '\0', sizeof(AdapterInfo) * number_adapters);
    // Get all adapters information
    ADL_Adapter_AdapterInfo_Get(lp_adapter_info, sizeof(AdapterInfo) * number_adapters);
  }

  // For all adapters
  for (i = 0; i < number_adapters; i++) {
    printf("Adapter information:\n");
    printf("\tAdapter number = %d\n", i);
    printf("\tADL index handle = %d\n",lp_adapter_info[i].iAdapterIndex);
    printf("\tDriver number = %d\n",lp_adapter_info[i].iDeviceNumber);
    printf("\tVendor ID = %d\n",lp_adapter_info[i].iVendorID);
    printf("\tAdapter name = %s\n", lp_adapter_info[i].strAdapterName);
    printf("\tAdapter present = %d\n",lp_adapter_info[i].iPresent);
    ADL_Adapter_ID_Get(lp_adapter_info[i].iAdapterIndex, &lp_adapter_id);
    printf("\tAdapter Unique ID = %d\n", lp_adapter_id);
  }

  free(lp_adapter_info);

  // Release all ADL global pointers
  ADL_Main_Control_Destroy();

  dlclose(hDLL);

  return 0;
}



