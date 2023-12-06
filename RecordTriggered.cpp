/// \file
/// \brief record sample
/// \example Record.cpp

//**********************************************************************
// This Demo shows how to use GenICam API(C) to write a simple program.
// Please make sure that the camera and PC are in the same subnet before running the demo.
// If not, you can use camera client software to modify the IP address of camera to the same subnet with PC.
//**********************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "/opt/iCentral/iCentral/include/IMVApi.h"

#include <iostream>
#include <chrono>
#include <libserial/SerialStream.h>

#define sleep(ms) usleep(1000 * ms)

void *frameGrabbingProc(void *pUserData)
{
	const int frames = 500;
	int ret = IMV_OK;
	IMV_HANDLE devHandle = (IMV_HANDLE *)pUserData;
	IMV_Frame frame;
	IMV_RecordFrameInfoParam stRecordFrameInfoParam;

	if (!devHandle)
		return NULL;

	// Record the start time
	auto start_record = std::chrono::high_resolution_clock::now();

	for (unsigned int i = 0; i < frames; i++)
	{
		// Get a frame image
		ret = IMV_GetFrame(devHandle, &frame, 500);
		if (IMV_OK != ret)
		{
			printf("Get frame failed! ErrorCode[%d]\n", ret);
			continue;
		}

		memset(&stRecordFrameInfoParam, 0, sizeof(stRecordFrameInfoParam));
		stRecordFrameInfoParam.pData = frame.pData;
		stRecordFrameInfoParam.nDataLen = frame.frameInfo.size;
		stRecordFrameInfoParam.nPaddingX = frame.frameInfo.paddingX;
		stRecordFrameInfoParam.nPaddingY = frame.frameInfo.paddingY;
		stRecordFrameInfoParam.ePixelFormat = frame.frameInfo.pixelFormat;

		// record one frame
		ret = IMV_InputOneFrame(devHandle, &stRecordFrameInfoParam);
		if (IMV_OK != ret)
			printf("record failed at frame %u! ErrorCode[%d]\n", frames, ret);

		// Free image buffer
		ret = IMV_ReleaseFrame(devHandle, &frame);
		if (IMV_OK != ret)
			printf("Release frame failed! ErrorCode[%d]\n", ret);
	}

	auto end_record = std::chrono::high_resolution_clock::now();									  // Record the end time
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_record - start_record); // Calculate the duration

	// Print the duration in microseconds
	std::cout << "Recorded " << frames << " frames in " << (float)duration.count() / 1000000 << "s, " << (frames * 1000000) / (duration.count()) << "fps" << std::endl;

	return NULL;
}

// ***********BEGIN: These functions are not related to API call and used to display device info***********
static void displayDeviceInfo(IMV_DeviceList deviceInfoList)
{
	IMV_DeviceInfo *pDevInfo = NULL;
	unsigned int cameraIndex = 0;
	char vendorNameCat[11];
	char cameraNameCat[16];

	// Print title line
	printf("\nIdx Type Vendor     Model      S/N             DeviceUserID    IP Address    \n");
	printf("------------------------------------------------------------------------------\n");

	for (cameraIndex = 0; cameraIndex < deviceInfoList.nDevNum; cameraIndex++)
	{
		pDevInfo = &deviceInfoList.pDevInfo[cameraIndex];
		// Camera index in device list, display in 3 characters
		printf("%-3d", cameraIndex + 1);

		// Camera type
		switch (pDevInfo->nCameraType)
		{
		case typeGigeCamera:
			printf(" GigE");
			break;
		case typeU3vCamera:
			printf(" U3V ");
			break;
		case typeCLCamera:
			printf(" CL  ");
			break;
		case typePCIeCamera:
			printf(" PCIe");
			break;
		default:
			printf("     ");
			break;
		}

		// Camera vendor name, display in 10 characters
		if (strlen(pDevInfo->vendorName) > 10)
		{
			memcpy(vendorNameCat, pDevInfo->vendorName, 7);
			vendorNameCat[7] = '\0';
			strcat(vendorNameCat, "...");
			printf(" %-10.10s", vendorNameCat);
		}
		else
			printf(" %-10.10s", pDevInfo->vendorName);

		// Camera model name, display in 10 characters
		printf(" %-10.10s", pDevInfo->modelName);

		// Camera serial number, display in 15 characters
		printf(" %-15.15s", pDevInfo->serialNumber);

		// Camera user id, display in 15 characters
		if (strlen(pDevInfo->cameraName) > 15)
		{
			memcpy(cameraNameCat, pDevInfo->cameraName, 12);
			cameraNameCat[12] = '\0';
			strcat(cameraNameCat, "...");
			printf(" %-15.15s", cameraNameCat);
		}
		else
			printf(" %-15.15s", pDevInfo->cameraName);

		// IP address of GigE camera
		if (pDevInfo->nCameraType == typeGigeCamera)
			printf(" %s", pDevInfo->DeviceSpecificInfo.gigeDeviceInfo.ipAddress);

		printf("\n");
	}

	return;
}

static unsigned int selectDevice(IMV_DeviceList deviceInfoList)
{
	IMV_DeviceInfo *pDevInfo = NULL;
	for (unsigned int i = 0; i < deviceInfoList.nDevNum; i++)
	{
		pDevInfo = &deviceInfoList.pDevInfo[i];
		if (strcmp(pDevInfo->modelName, "Mars640-815um") == 0)
			return i;
	}
	return 0;
}

// ***********END: These functions are not related to API call and used to display device info***********

int main()
{
	static unsigned int video_index = 1;
	int ret = IMV_OK;
	int64_t nWidth = 0;
	int64_t nHeight = 0;
	IMV_HANDLE devHandle = NULL;
	pthread_t threadID = 0;
	IMV_RecordParam stRecordParam;

	// discover camera
	IMV_DeviceList deviceInfoList;
	ret = IMV_EnumDevices(&deviceInfoList, interfaceTypeAll);
	if (IMV_OK != ret)
	{
		printf("Enumeration devices failed! ErrorCode[%d]\n", ret);
		getchar();
		return -1;
	}

	if (deviceInfoList.nDevNum < 1)
	{
		printf("No camera\n");
		getchar();
		return -1;
	}

	// Print camera info (Index, Type, Vendor, Model, Serial number, DeviceUserID, IP Address)
	displayDeviceInfo(deviceInfoList);

	// Select one camera to connect to
	unsigned int cameraIndex = selectDevice(deviceInfoList);

	while (1)
	{
		do
		{
			// Create Device Handle
			ret = IMV_CreateHandle(&devHandle, modeByIndex, (void *)&cameraIndex);
			if (IMV_OK != ret)
			{
				printf("Create devHandle failed! ErrorCode[%d]\n", ret);
				break;
			}

			// Open camera
			ret = IMV_Open(devHandle);
			if (IMV_OK != ret)
			{
				printf("Open camera failed! ErrorCode[%d]\n", ret);
				break;
			}

			// The width and height must be an integer multiple of 2.
			if (!IMV_FeatureIsValid(devHandle, "Width"))
			{
				printf("Width feature is invalid!\n");
				break;
			}

			ret = IMV_GetIntFeatureValue(devHandle, "Width", &nWidth);
			if (IMV_OK != ret)
			{
				printf("Get width feature value failed! ErrorCode[%d]\n", ret);
				break;
			}

			if (!IMV_FeatureIsValid(devHandle, "Height"))
			{
				printf("Height feature is invalid!\n");
				break;
			}

			ret = IMV_GetIntFeatureValue(devHandle, "Height", &nHeight);
			if (IMV_OK != ret)
			{
				printf("Get height feature value failed! ErrorCode[%d]\n", ret);
				break;
			}

			memset(&stRecordParam, 0, sizeof(stRecordParam));

			stRecordParam.nWidth = (unsigned int)nWidth;	 // video image width
			stRecordParam.nHeight = (unsigned int)nHeight;	 // video image height
			stRecordParam.fFameRate = 30;					 // frame rate. Please set to the actual streaming frame rate.
			stRecordParam.nQuality = 20;					 // video quality(1-100). The larger the parameter, the clearer the video, but the larger the volume.
			stRecordParam.recordFormat = typeVideoFormatAVI; // video format

			std::string file_path = "./Video/Record_" + std::to_string(video_index) + ".avi";
			stRecordParam.pRecordFilePath = file_path.c_str(); // please make sure that the path does exist.

			// Receive signal from MCU
			char receivedSignal;
			try
			{
				LibSerial::SerialStream port;
				port.Open("/dev/ttyACM0");							// Replace with your serial port device
				port.SetBaudRate(LibSerial::BaudRate::BAUD_115200); // Adjust baud rate as needed

				port.get(receivedSignal);

				// Process the received signal
				std::cout << "Received Signal: " << receivedSignal << std::endl;
			}
			catch (const std::exception &e)
			{
				// std::cerr << "Exception: " << e.what() << std::endl;
				continue;
			}

			if (receivedSignal != 'N'){
				continue;
				std::cout << "Pass" << std::endl;
			}

			// open the record handle
			ret = IMV_OpenRecord(devHandle, &stRecordParam);
			if (IMV_OK != ret)
			{
				// If opefailed, Run as root
				printf("OpenRecord failed! ErrorCode[%d]\n", ret);
				break;
			}

			// Start grabbing
			ret = IMV_StartGrabbing(devHandle);
			if (IMV_OK != ret)
			{
				printf("Start grabbing failed! ErrorCode[%d]\n", ret);
				// close the record handle
				IMV_CloseRecord(devHandle);
				break;
			}

			// Create frame grabbing thread
			if (pthread_create(&threadID, 0, frameGrabbingProc, (void *)devHandle) != 0)
			{
				printf("Failed to create getFrame thread!\n");
				// close the record handle
				IMV_CloseRecord(devHandle);
				break;
			}
			else video_index++;

			// Waiting exit record thread
			pthread_join(threadID, NULL);

			// Stop grabbing
			ret = IMV_StopGrabbing(devHandle);
			if (IMV_OK != ret)
			{
				printf("Stop grabbing failed! ErrorCode[%d]\n", ret);
				break;
			}

			// Close camera
			ret = IMV_Close(devHandle);
			if (IMV_OK != ret)
			{
				printf("Close camera failed! ErrorCode[%d]\n", ret);
				break;
			}
		} while (false);

		if (devHandle != NULL)
		{
			// Destroy Device Handle
			IMV_DestroyHandle(devHandle);
		}
	}
	return 0;
}