#include "stdafx.h"

#include "wht_device.h"
#include "hid.h"
#include "myutils.h"

#define VENDOR_ID	0x40AA
#define PRODUCT_ID	0x9007

WHTDevice::WHTDevice()
	: hDevice(NULL), hWriteEvent(NULL)
{}

WHTDevice::~WHTDevice()
{
	Close();
}

bool WHTDevice::Open()
{
	Close();

	hDevice = NULL;

	GUID guid;
	HDEVINFO info;
	DWORD index, required_size;
	SP_DEVICE_INTERFACE_DATA iface;
	SP_DEVICE_INTERFACE_DETAIL_DATA* details;
	HIDD_ATTRIBUTES attrib;

	HidD_GetHidGuid(&guid);
	info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (info == INVALID_HANDLE_VALUE)
		return false;

	for (index = 0; 1; index++)
	{
		debug(index);

		// get the next HID device
		iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		if (!SetupDiEnumDeviceInterfaces(info, NULL, &guid, index, &iface))
		{
			SetupDiDestroyDeviceInfoList(info);
			break;
		}

		// get the required size for details 
		SetupDiGetDeviceInterfaceDetail(info, &iface, NULL, 0, &required_size, NULL);

		// allocate and clear
		details = (SP_DEVICE_INTERFACE_DETAIL_DATA*) malloc(required_size);
		if (details == NULL)
			continue;

		memset(details, 0, required_size);

		// now get the details
		details->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(info, &iface, details, required_size, NULL, NULL))
		{
			free(details);
			continue;
		}

		// open the HID device
		HANDLE h = CreateFile(details->DevicePath, GENERIC_READ|GENERIC_WRITE,
						FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
						FILE_FLAG_OVERLAPPED, NULL);
		free(details);
		if (h == INVALID_HANDLE_VALUE)
			continue;

		// get the device's attributes
		attrib.Size = sizeof(HIDD_ATTRIBUTES);
		if (!HidD_GetAttributes(h, &attrib))
		{
			CloseHandle(h);
			continue;
		}
		
		// is this the device we need?
		if (attrib.VendorID != VENDOR_ID  ||  attrib.ProductID != PRODUCT_ID)
		{
			CloseHandle(h);
			continue;
		}

		SetupDiDestroyDeviceInfoList(info);
		hDevice = h;
	}

	if (!hDevice)
		return false;

	return true;
}

void WHTDevice::Close()
{
	if (!hDevice)
		return;

	CloseHandle(hDevice);

	hDevice = NULL;
}

bool WHTDevice::GetFeatureReport(uint8_t* buffer, int report_size, uint8_t report_id)
{
	const int buff_size = report_size + 1;
	std::auto_ptr<uint8_t> rcvBuff(new uint8_t[buff_size]);
	rcvBuff.get()[0] = report_id;
	if (HidD_GetFeature(hDevice, buffer, buff_size) == FALSE)
		return false;

	memcpy(buffer, rcvBuff.get() + 1, report_size);
	
	return true;
}

bool WHTDevice::SetFeatureReport(const uint8_t* buffer, int report_size, uint8_t report_id)
{
	// alloc the buffer
	int buff_size = report_size + 1;
	std::auto_ptr<uint8_t> sendBuff(new uint8_t[buff_size]);

	// set the report ID and the data
	sendBuff.get()[0] = report_id;
	memcpy(sendBuff.get() + 1, buffer, report_size);
		
	return HidD_SetFeature(hDevice, sendBuff.get(), buff_size) == TRUE;
}