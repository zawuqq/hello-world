HANDLE CDownLoadDlg::OpenMyHIDDevice(int overlapped) {
// HANDLE hidHandle;
	GUID hidGuid;
	HDEVINFO hDevInfo;
	HidD_GetHidGuid(&hidGuid);//�õ�HID·��
	hDevInfo = SetupDiGetClassDevs(&hidGuid,NULL,NULL,(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)); //�õ�һ���豸��Ϣ��ֻ���ص�ǰ���ڵ��豸
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		return INVALID_HANDLE_VALUE;
	}
	SP_DEVICE_INTERFACE_DATA devInfoData;
	devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
	int deviceNo = 0; //�豸�б��е����


	SetLastError(NO_ERROR);
	while (GetLastError() != ERROR_NO_MORE_ITEMS) {
		if (SetupDiEnumInterfaceDevice (hDevInfo,0,&hidGuid,deviceNo,&devInfoData)) { //���������Ǵ��Ѿ���ȡ���豸�ӿ��б���Ϣ�л�ȡ��Ϣ��ʹ�ýṹ����,ÿ����һ�λ����η���һ���ӿ���Ϣ
			ULONG  requiredLength = 0;
//ȡ�ø��豸�ӿڵ�ϸ��(�豸·��)
			SetupDiGetInterfaceDeviceDetail(hDevInfo,// �豸��Ϣ�����
			                                &devInfoData,  // �豸�ӿ���Ϣ
			                                NULL, // �豸�ӿ�ϸ��(�豸·��)
			                                0, // �����������С
			                                &requiredLength,// ���������������С
			                                NULL);  // ���������豸����


// PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(requiredLength);//�����СΪrequiredLength���ڴ��
			PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredLength);//�����СΪrequiredLength���ڴ��
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
				hidHandle = CreateFile(devDetail->DevicePath,//Ҫ�򿪵��ļ��������豸��
				                       GENERIC_READ | GENERIC_WRITE,//���Ϊ GENERIC_READ ��ʾ������豸���ж����ʣ�
//���Ϊ GENERIC_WRITE ��ʾ������豸����д���ʣ������ʹ�ã���
//���Ϊ�㣬��ʾֻ�����ȡ��һ���豸�йص���Ϣ
// 0,
				                       FILE_SHARE_READ | FILE_SHARE_WRITE,//�����FILE_SHARE_READ���򿪲��������ɹ�ֻ����������ʣ�
//�����FILE_SHARE_WRITE ���򿪲��������ɹ�ֻ������д����
				                       NULL, //�������ļ��İ�ȫ����
				                       OPEN_EXISTING,//�ļ������Ѿ����ڣ����豸���Ҫ��
				                       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,//������ļ������ص�����
				                       NULL);  //���ڸ����ļ����
				if(hidHandle==INVALID_HANDLE_VALUE) {
					hidHandle = CreateFile(devDetail->DevicePath,//Ҫ�򿪵��ļ��������豸��
// GENERIC_READ | GENERIC_WRITE,//���Ϊ GENERIC_READ ��ʾ������豸���ж����ʣ�
//���Ϊ GENERIC_WRITE ��ʾ������豸����д���ʣ������ʹ�ã���
//���Ϊ�㣬��ʾֻ�����ȡ��һ���豸�йص���Ϣ
					                       0,
					                       FILE_SHARE_READ | FILE_SHARE_WRITE,//�����FILE_SHARE_READ���򿪲��������ɹ�ֻ����������ʣ�
//�����FILE_SHARE_WRITE ���򿪲��������ɹ�ֻ������д����
					                       NULL, //�������ļ��İ�ȫ����
					                       OPEN_EXISTING,//�ļ������Ѿ����ڣ����豸���Ҫ��
					                       FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,//������ļ������ص�����
					                       NULL);
				}


			} else {
				hidHandle = CreateFile(devDetail->DevicePath,
				                       GENERIC_READ | GENERIC_WRITE,
// 0,
				                       FILE_SHARE_READ | FILE_SHARE_WRITE,
				                       NULL,
				                       OPEN_EXISTING,
				                       0, //��������ļ������ص�����
				                       NULL);
				if(hidHandle==INVALID_HANDLE_VALUE) {
					hidHandle = CreateFile(devDetail->DevicePath,
// GENERIC_READ | GENERIC_WRITE,
					                       0,
					                       FILE_SHARE_READ | FILE_SHARE_WRITE,
					                       NULL,
					                       OPEN_EXISTING,
					                       0, //��������ļ������ص�����
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
			if(!HidD_GetAttributes(hidHandle, &hidAttributes)) { //Ӧ�ó������HID���������س���ID����ƷID��汾��
				free(devDetail);
				CloseHandle(hidHandle);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				return INVALID_HANDLE_VALUE;
			}


			if   (USB_VID  == hidAttributes.VendorID//�Աȳ���ID
			        && USB_PID  == hidAttributes.ProductID//�Աȳ�ƷID
			        && USB_PVN  == hidAttributes.VersionNumber) { //�Ա����ID
				EventObject = CreateEvent(NULL, TRUE, TRUE, _T(""));//����¼�����������豸
//Set the members of the overlapped structure.
				HIDOverlapped.Offset = 0;
				HIDOverlapped.OffsetHigh = 0;
				HIDOverlapped.hEvent = EventObject;
				break;
			} else {
				CloseHandle(hidHandle);//���ü�����1������Ϊ0ʱ��ϵͳɾ���ں˶���
				hidHandle   = INVALID_HANDLE_VALUE;
				EventObject = NULL;
				++deviceNo;
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return hidHandle;
}