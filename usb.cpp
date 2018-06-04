HANDLE CDownLoadDlg::OpenMyHIDDevice(int overlapped) {
// HANDLE hidHandle;
	GUID hidGuid;
	HDEVINFO hDevInfo;
	HidD_GetHidGuid(&hidGuid);//得到HID路径    
	hDevInfo = SetupDiGetClassDevs(&hidGuid,NULL,NULL,(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); //得到一类设备信息，只返回当前存在的设备
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return INVALID_HANDLE_VALUE;
	}
	SP_DEVICE_INTERFACE_DATA devInfoData;
	devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
	int deviceNo = 0; //设备列表中的序号


	SetLastError(NO_ERROR);
	while (GetLastError() != ERROR_NO_MORE_ITEMS) {
		if (SetupDiEnumInterfaceDevice (hDevInfo,0,&hidGuid,deviceNo,&devInfoData)) { //函数功能是从已经获取的设备接口列表信息中获取信息并使用结构保存,每调用一次会依次返回一个接口信息
			ULONG  requiredLength = 0;
//取得该设备接口的细节(设备路径)
			SetupDiGetInterfaceDeviceDetail(hDevInfo,// 设备信息集句柄
			                                &devInfoData,  // 设备接口信息
			                                NULL, // 设备接口细节(设备路径)
			                                0, // 输出缓冲区大小
			                                &requiredLength,// 计算输出缓冲区大小
			                                NULL);  // 不需额外的设备描述


// PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(requiredLength);//分配大小为requiredLength的内存块
			PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredLength);//分配大小为requiredLength的内存块
			devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);


			if(!SetupDiGetInterfaceDeviceDetail(hDevInfo,
			                                    &devInfoData,
			                                    devDetail,
			                                    requiredLength,
			                                    &requiredLength,
			                                    NULL)) {
				free(devDetail);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return INVALID_HANDLE_VALUE;
			}




			if (overlapped) {
				hidHandle = CreateFile(devDetail->DevicePath,//要打开的文件的名或设备名
				                       GENERIC_READ | GENERIC_WRITE,//如果为 GENERIC_READ 表示允许对设备进行读访问；
//如果为 GENERIC_WRITE 表示允许对设备进行写访问（可组合使用）；
//如果为零，表示只允许获取与一个设备有关的信息
// 0,
				                       FILE_SHARE_READ | FILE_SHARE_WRITE,//如果是FILE_SHARE_READ随后打开操作对象会成功只有请求读访问；
//如果是FILE_SHARE_WRITE 随后打开操作对象会成功只有请求写访问
				                       NULL, //定义了文件的安全特性
				                       OPEN_EXISTING,//文件必须已经存在，由设备提出要求
				                       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,//允许对文件进行重叠操作
				                       NULL);  //用于复制文件句柄
				if(hidHandle==INVALID_HANDLE_VALUE) {
					hidHandle = CreateFile(devDetail->DevicePath,//要打开的文件的名或设备名
// GENERIC_READ | GENERIC_WRITE,//如果为 GENERIC_READ 表示允许对设备进行读访问；
//如果为 GENERIC_WRITE 表示允许对设备进行写访问（可组合使用）；
//如果为零，表示只允许获取与一个设备有关的信息
					                       0,
					                       FILE_SHARE_READ | FILE_SHARE_WRITE,//如果是FILE_SHARE_READ随后打开操作对象会成功只有请求读访问；
//如果是FILE_SHARE_WRITE 随后打开操作对象会成功只有请求写访问
					                       NULL, //定义了文件的安全特性
					                       OPEN_EXISTING,//文件必须已经存在，由设备提出要求
					                       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,//允许对文件进行重叠操作
					                       NULL);
				}


			} else {
				hidHandle = CreateFile(devDetail->DevicePath,
				                       GENERIC_READ | GENERIC_WRITE,
// 0,
				                       FILE_SHARE_READ | FILE_SHARE_WRITE,
				                       NULL,
				                       OPEN_EXISTING,
				                       0, //不允许对文件进行重叠操作
				                       NULL);
				if(hidHandle==INVALID_HANDLE_VALUE) {
					hidHandle = CreateFile(devDetail->DevicePath,
// GENERIC_READ | GENERIC_WRITE,
					                       0,
					                       FILE_SHARE_READ | FILE_SHARE_WRITE,
					                       NULL,
					                       OPEN_EXISTING,
					                       0, //不允许对文件进行重叠操作
					                       NULL);
				}
			}


// long ly=GetLastError();//debug
			if (hidHandle==INVALID_HANDLE_VALUE) {
				SetupDiDestroyDeviceInfoList(hDevInfo);
				free(devDetail);
				return INVALID_HANDLE_VALUE;
			}


			HIDD_ATTRIBUTES hidAttributes;
// hidAttributes.Size=sizeof(hidAttributes);//LY
			if(!HidD_GetAttributes(hidHandle, &hidAttributes)) { //应用程序调用HID函数，传回厂商ID，产品ID与版本号
				free(devDetail);
				CloseHandle(hidHandle);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return INVALID_HANDLE_VALUE;
			}


			if   (USB_VID  == hidAttributes.VendorID//对比厂家ID
			        && USB_PID  == hidAttributes.ProductID//对比厂品ID
			        && USB_PVN  == hidAttributes.VersionNumber) { //对比软件ID
				EventObject = CreateEvent(NULL, TRUE, TRUE, _T(""));//获得事件句柄，监听设备
//Set the members of the overlapped structure.
				HIDOverlapped.Offset = 0;
				HIDOverlapped.OffsetHigh = 0;
				HIDOverlapped.hEvent = EventObject;
				break;
			} else {
				CloseHandle(hidHandle);//引用计数减1，当变为0时，系统删除内核对象
				hidHandle   = INVALID_HANDLE_VALUE;
				EventObject = NULL;
				++deviceNo;
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return hidHandle;
}
