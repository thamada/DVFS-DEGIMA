#define LINUX 1
#include "adl_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Main Group of APIs
extern int ADL_Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters);
extern int ADL_Main_Control_Destroy();
//General Adapter APIs
extern int ADL_Adapter_NumberOfAdapters_Get(int *lpNumAdapters);
extern int ADL_Adapter_AdapterInfo_Get(LPAdapterInfo lpInfo, int iInputSize);
//Overdrive5 APIs
extern int ADL_Overdrive5_Temperature_Get(int iAdapterIndex, int iThermalControllerIndex, ADLTemperature *lpTemperature);
extern int ADL_Overdrive5_FanSpeedInfo_Get(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedInfo *lpFanSpeedInfo);
extern int ADL_Overdrive5_FanSpeed_Get(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);
extern int ADL_Overdrive5_FanSpeed_Set(int iAdapterIndex, int iThermalControllerIndex, ADLFanSpeedValue *lpFanSpeedValue);

struct tinfo {
	int iAdapterIndex;
	char strUDID[256];
	int iBusNumber;
	int iSlaveIndex;
	int iMasterTemp;
	int iSlaveTemp;
	int iMinPercent;
	int iMaxPercent;
	int iFanSpeed;
	int iTemp;
	int nPrevTemp;
	float fIntegral;
	int iPrevSum;
	int iPrevTemps[60];
};

#define MAX_ADAPTERS 8
int ninfos;
struct tinfo info[MAX_ADAPTERS];

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

int enum_adapters(void)
{
	int iNumAdapters = 0;
	LPAdapterInfo lpAdapterInfo = NULL;
	int i, j, dupe;
	ADLTemperature Temperature;
	ADLFanSpeedInfo FanSpeedInfo;
	ADLFanSpeedValue FanSpeedValue;

        if(ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1) != ADL_OK) {
		fprintf(stderr, "ADL Initialization Error!\n");
		return 1;
	}
        if(ADL_Adapter_NumberOfAdapters_Get(&iNumAdapters) != ADL_OK) {
		fprintf(stderr, "Cannot get the number of adapters!\n");
		return 1;
	}
	if(iNumAdapters <= 0) {
		fprintf(stderr, "No adapters found!\n");
		return 1;
	}
	lpAdapterInfo = malloc(iNumAdapters * sizeof(AdapterInfo));
	memset(lpAdapterInfo, 0, iNumAdapters * sizeof(AdapterInfo));
	if(ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, iNumAdapters * sizeof(AdapterInfo)) != ADL_OK) {
		fprintf(stderr, "Cannot get adapter info!\n");
		return 1;
	}
	ninfos = 0;
	for(i = 0; i < iNumAdapters; i++) {
		if(! lpAdapterInfo[i].iPresent)
			continue;
		//ignore duplicate adapters
		dupe = 0;
		for(j = 0; j < ninfos; j++) {
			if(!strcmp(lpAdapterInfo[i].strUDID, info[j].strUDID)) {
				dupe = 1;
				break;
			}
		}
		if(dupe)
			continue;

		memset(&Temperature, 0, sizeof(ADLTemperature));
		Temperature.iSize = sizeof(ADLTemperature);
		if(ADL_Overdrive5_Temperature_Get(lpAdapterInfo[i].iAdapterIndex, 0, &Temperature) != ADL_OK) {
			fprintf(stderr, "Cannot get temperature!\n");
			return 1;
		}
		memset(&FanSpeedInfo, 0, sizeof(ADLFanSpeedInfo));
		FanSpeedInfo.iSize = sizeof(ADLFanSpeedInfo);
		if(ADL_Overdrive5_FanSpeedInfo_Get(lpAdapterInfo[i].iAdapterIndex, 0, &FanSpeedInfo) != ADL_OK) {
			fprintf(stderr, "Cannot get fan speed info!\n");
			return 1;
		}
		if(!(FanSpeedInfo.iFlags & (ADL_DL_FANCTRL_SUPPORTS_PERCENT_READ | ADL_DL_FANCTRL_SUPPORTS_PERCENT_WRITE))) {
			fprintf(stderr, "Adapter doesn't support fan speed control! %i\n", FanSpeedInfo.iFlags);
			continue;
		}
		memset(&FanSpeedValue, 0, sizeof(ADLFanSpeedValue));
		FanSpeedValue.iSize = sizeof(ADLFanSpeedValue);
		FanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
		if(ADL_Overdrive5_FanSpeed_Get(lpAdapterInfo[i].iAdapterIndex, 0, &FanSpeedValue) != ADL_OK) {
			fprintf(stderr, "Cannot get fan speed!\n");
			return 1;
		}
		info[ninfos].iAdapterIndex = lpAdapterInfo[i].iAdapterIndex;
		strcpy(info[ninfos].strUDID, lpAdapterInfo[i].strUDID);
		info[ninfos].iBusNumber = lpAdapterInfo[i].iBusNumber;
		info[ninfos].iSlaveIndex = -1;
		info[ninfos].iMinPercent = FanSpeedInfo.iMinPercent;
		info[ninfos].iMaxPercent = FanSpeedInfo.iMaxPercent;
		info[ninfos].iFanSpeed = FanSpeedValue.iFanSpeed;
		info[ninfos].iTemp = Temperature.iTemperature;
		info[ninfos].fIntegral = 0.;
		info[ninfos].nPrevTemp = 0;
		info[ninfos].iPrevSum = info[ninfos].iTemp * 60;
		for(j = 0; j < 60; j++)
			info[ninfos].iPrevTemps[j] = info[ninfos].iTemp;

		fprintf(stdout, "%.1f\n",
			(info[ninfos].iTemp) * 0.001 );

		/*
		fprintf(stdout, "Adapter %i: %3i.%01i Celcius\n",
                        ninfos,
                        info[ninfos].iTemp / 1000, (info[ninfos].iTemp % 1000) / 100);
		*/

/*		fprintf(stdout, "Adapter %i: idx %i temp %3i.%01i°C speed %3i%% (%3i%% - %3i%%)\n",
			ninfos,
			info[ninfos].iAdapterIndex,
			info[ninfos].iTemp / 1000, (info[ninfos].iTemp % 1000) / 100,
			info[ninfos].iFanSpeed,
			info[ninfos].iMinPercent,
			info[ninfos].iMaxPercent);
// */
		ninfos += 1;
	}
	if(ninfos == 0) {
		fprintf(stderr, "No adapters found!\n");
		return 1;
	}
	//find slaves
	for(i = 0; i < iNumAdapters; i++) {
		if(! lpAdapterInfo[i].iPresent)
			continue;
		//ignore duplicate adapters
		dupe = 0;
		for(j = 0; j < ninfos; j++) {
			if(!strcmp(lpAdapterInfo[i].strUDID, info[j].strUDID)) {
				dupe = 1;
				break;
			}
		}
		if(dupe)
			continue;
		memset(&FanSpeedInfo, 0, sizeof(ADLFanSpeedInfo));
		FanSpeedInfo.iSize = sizeof(ADLFanSpeedInfo);
		if(ADL_Overdrive5_FanSpeedInfo_Get(lpAdapterInfo[i].iAdapterIndex, 0, &FanSpeedInfo) != ADL_OK) {
			fprintf(stderr, "Cannot get fan speed info!\n");
			return 1;
		}
		if(FanSpeedInfo.iFlags)
			continue;
		for(j = 0; j < ninfos; j++) {
			if(info[j].iSlaveIndex == -1) {
				if((info[j].iBusNumber == lpAdapterInfo[i].iBusNumber + 1) || (info[j].iBusNumber == lpAdapterInfo[i].iBusNumber - 1)) {
					info[j].iSlaveIndex = lpAdapterInfo[i].iAdapterIndex;
					break;
				}
			}
		}
	}
	return 0;
}

static const float targettemp = 77.0;	//deg C
static const float Kp = 1.;
static const float Ki = 0.5;
static const float Kd = 1.;

int calc_new_fanspeed(int idx)
{
	float curtemp, avgtemp;
	float Sp, Si, Sd, Ss;
	int newfanspeed;
	curtemp = (float)(info[idx].iTemp) / 1000.0;
	info[idx].fIntegral += curtemp - targettemp;
	avgtemp = (float)(info[idx].iPrevSum) / (1000.0 * 60);
	Sp = (curtemp - targettemp) * Kp;
	Si = info[idx].fIntegral * Ki;
	Sd = (curtemp - avgtemp) * Kd;
	Ss = Sp + Si + Sd;
	newfanspeed = 50 + Ss;
	if(newfanspeed < info[idx].iMinPercent)
		newfanspeed = info[idx].iMinPercent;
	if(newfanspeed > info[idx].iMaxPercent)
		newfanspeed = info[idx].iMaxPercent;
	if(curtemp > targettemp + 5.0) {
		newfanspeed = info[idx].iMaxPercent;
		info[idx].fIntegral = 100.;
	}
	return newfanspeed;
}

int main(int argc, char **argv)
{
	int i, tfanspeed;
	ADLTemperature Temperature;
	ADLFanSpeedValue FanSpeedValue;
	if(enum_adapters())
		return 1;
	memset(&Temperature, 0, sizeof(ADLTemperature));
	Temperature.iSize = sizeof(ADLTemperature);
	memset(&FanSpeedValue, 0, sizeof(ADLFanSpeedValue));
	FanSpeedValue.iSize = sizeof(ADLFanSpeedValue);
	FanSpeedValue.iSpeedType = ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
	FanSpeedValue.iFlags = ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED;

	while(0) { // regulates the GPU temp to stay under target
		for(i = 0; i < ninfos; i++) {
			//get current temp
			if(ADL_Overdrive5_Temperature_Get(info[i].iAdapterIndex, 0, &Temperature) != ADL_OK) {
				fprintf(stderr, "Cannot get temperature!\n");
				return 1;
			}
			info[i].iMasterTemp = Temperature.iTemperature;
			if(info[i].iSlaveIndex != -1) {
				if(ADL_Overdrive5_Temperature_Get(info[i].iSlaveIndex, 0, &Temperature) != ADL_OK) {
					fprintf(stderr, "Cannot get temperature!\n");
					return 1;
				}
				info[i].iSlaveTemp = Temperature.iTemperature;
				if(Temperature.iTemperature < info[i].iMasterTemp) {
					Temperature.iTemperature = info[i].iMasterTemp;
				}
			}
			info[i].iPrevSum -= info[i].iPrevTemps[info[i].nPrevTemp];
			info[i].iPrevSum += info[i].iTemp;
			info[i].iPrevTemps[info[i].nPrevTemp] = info[i].iTemp;
			info[i].iTemp = Temperature.iTemperature;
			tfanspeed = calc_new_fanspeed(i);
			if(tfanspeed != info[i].iFanSpeed) {
				info[i].iFanSpeed = tfanspeed;
				FanSpeedValue.iFanSpeed = tfanspeed;
				if(ADL_Overdrive5_FanSpeed_Set(info[i].iAdapterIndex, 0, &FanSpeedValue) != ADL_OK) {
					fprintf(stderr, "Cannot set fan speed!\n");
					return 1;
				}
			}
			info[i].nPrevTemp = (info[i].nPrevTemp + 1) % 60;
		}
		//print status line
		for(i = 0; i < ninfos; i++) {
			if(i)
				fprintf(stdout, "   ");
			if(info[i].iSlaveIndex != -1) {
				fprintf(stdout, "%i:%3i.%01i C:%3i.%01i C:%3i%%",
					i,
					info[i].iMasterTemp / 1000, (info[i].iMasterTemp % 1000) / 100,
					info[i].iSlaveTemp / 1000, (info[i].iSlaveTemp % 1000) / 100,
					info[i].iFanSpeed);
			} else {
				fprintf(stdout, "%i:%3i.%01i C:%3i%%",
					i,
					info[i].iMasterTemp / 1000, (info[i].iMasterTemp % 1000) / 100,
					info[i].iFanSpeed);
			}
		}
		fprintf(stdout, "\n");
		fflush(stdout);
		sleep(1);
	}

//	printf("done\n");
	ADL_Main_Control_Destroy();
	return 0;
}
