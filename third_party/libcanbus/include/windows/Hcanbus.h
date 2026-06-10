
#ifndef HCANBUS_H
#define HCANBUS_H

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
#define LIBUSB_CALL WINAPI
#else
#define LIBUSB_CALL
#endif

typedef  struct  _Dev_Info {
	char            HW_Type[32]; //设备型号 字符串
	char            HW_Ser[32];  //设备序列号 字符串 
	char            HW_Ver[32];  //硬件版本 字符串
	char            FW_Ver[32];  //软件版本 字符串
	char            MF_Date[32]; //生产日期 字符串
} Dev_Info, *PDev_Info;

typedef struct _Can_Config {
	unsigned int    Baudrate;
	unsigned short  Pres;        // Pres,Tseg1,Tseg2如果与Baudrate不匹配，动态库会自动按
	unsigned char   Tseg1;       // Baudrate重新计算Pres,Tseg1,Tseg2,SJW并且采样点设置为
	unsigned char   Tseg2;       // 75%左右。一般情况可以只设置波特率，Pres,Tseg1,Tseg2填0
	unsigned char   SJW;
	unsigned char   Config;      //配置信息：0x01接通内部电阻 0x02离线唤醒 0x04自动重传
	unsigned char   Model;       //工作模式：0 正常模式,1 环回模式,2 静默模式,3 静默环回模式
	unsigned char   Reserved;    //保留
}Can_Config, *P_Can_Config;

typedef struct _CanFD_Config {
	unsigned int    NomBaud;     //常规波特率
	unsigned int    DatBaud;     //数据波特率
	unsigned short  NomPre;      // NomPre,NomTseg1,NomTseg2如果与NomBaud不匹配，动态库会自动按
	unsigned char   NomTseg1;    // NomBaud重新计算NomPre,NomTseg1,NomTseg2,NomSJW并且采样点设置为
	unsigned char   NomTseg2;    // 75%左右。这些值可借助波特率计算器计算后设置
	unsigned char   NomSJW;
	unsigned char   DatPre;      // DatPre,DatTseg1,DatTseg2如果与DatBaud不匹配，动态库会自动按
	unsigned char   DatTseg1;    // DatBaud重新计算DatPre,DatTseg1,DatTseg2,DatSJW并且采样点设置为
	unsigned char   DatTseg2;    // 75%左右。这些值可借助波特率计算器计算后设置
	unsigned char   DatSJW;
	unsigned char   Config;      //配置信息：0x01接通内部电阻 0x02离线唤醒 0x04自动重传
	unsigned char   Model;       //工作模式：0 正常模式,1 环回模式,2 静默模式,3 静默环回模式
	unsigned char   Cantype;     //CAN模式： 0 CAN,1 IOS CANFD,2 Non-ISO CANFD
}CanFD_Config, *P_CanFD_Config;

typedef  struct  _Can_Msg {
	unsigned int    ID;          //报文ID
	unsigned int    TimeStamp;   //微秒级时间戳
	unsigned char   FrameType;   //帧类型
	unsigned char   DataLen;     //有效字节数
	unsigned char   ExternFlag;  //扩展帧标识：0标准帧,1扩展帧
	unsigned char   RemoteFlag;  //远程帧标识：0数据帧,1远程帧
	unsigned char   BusSatus;    //总线状态
	unsigned char   ErrSatus;    //错误状态
	unsigned char   TECounter;   //发送错误计数
	unsigned char   RECounter;   //接收错误计数
	unsigned char   Data[8];     //报文数据
}Can_Msg, *P_Can_Msg;

typedef  struct  _CanFD_Msg {
	unsigned int    ID;          //报文ID
	unsigned int    TimeStamp;   //微秒级时间戳
	unsigned char   FrameType;   //帧类型
	unsigned char   DLC;         //DLC不等于数据长度。最大值15，对于数据长度64
	unsigned char   ExternFlag;  //扩展帧标识：0标准帧,1扩展帧
	unsigned char   RemoteFlag;  //远程帧标识：0数据帧,1远程帧
	unsigned char   BusSatus;    //总线状态
	unsigned char   ErrSatus;    //错误状态
	unsigned char   TECounter;   //发送错误计数
	unsigned char   RECounter;   //接收错误计数
	unsigned char   Data[64];    //报文数据
}CanFD_Msg, *P_CanFD_Msg;

typedef struct _Can_Status {
	unsigned char   BusSatus;    //总线状态
	unsigned char   ErrSatus;    //错误状态
	unsigned char   TECounter;   //发送错误计数
	unsigned char   RECounter;   //接收错误计数
	unsigned int    TimeStamp;   //产生状态时的时间戳
}Can_Status, *P_Can_Status;

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall   Reg_HotPlug_Func(void(*pfunc)(void));   //热拔插函数 
int __stdcall   CAN_ScanDevice(void);                   //扫描CAN,CANFD设备

int __stdcall   CAN_OpenDevice(unsigned int devNum);    //打开CAN,CANFD设备
int __stdcall   CAN_CloseDevice(unsigned int devNum);   //关闭CAN,CANFD设备

int __stdcall   CAN_GetDevType(unsigned int devNum);     //获取CAN,CANFD类型 0：常规CAN，1：CANFD
int __stdcall   CAN_GetDevPlck(unsigned int devNum);     //获取CAN,CANFD主频
int __stdcall   CAN_ReadDevInfo(unsigned int devNum, PDev_Info devinfo);  //读取CAN,CANFD设备信息

int __stdcall   CAN_GetTimeStamp(unsigned int devNum,unsigned int *timestamp); //获取CAN,CANFD时间戳
int __stdcall   CAN_SetTimeStamp(unsigned int devNum,unsigned int timestamp,char mode);//设置CAN,CANFD时间戳  mode 0：设置当前通道，1设置同硬件所有通道

int __stdcall   CAN_SetFilter(unsigned int devNum,char namber, char type, unsigned int ftID, unsigned int ftMask, char enable); //设置CAN,CANFD硬件屏蔽
int __stdcall   CAN_Reset(unsigned int devNum);         //复位CAN,CANFD设备
int __stdcall   CAN_GetStatus(unsigned int devNum,P_Can_Status status);        //获取CAN,CANFD设备状态

int __stdcall   CAN_Init(unsigned int devNum,P_Can_Config pInitConfig);   //初始化CAN设备
int __stdcall   CAN_Transmit(unsigned int devNum, P_Can_Msg canmsg, unsigned int items, int timeou);  //发送CAN报文
int __stdcall   CAN_TransmitRt(unsigned int devNum, P_Can_Msg canmsg, unsigned int items, unsigned int *txitems, int timeou);   //定时发送CAN报文
int __stdcall   CAN_GetReceiveNum(unsigned int devNum); //获取接收缓冲区中接收到但尚未被读取的帧数量
int __stdcall   CAN_Receive(unsigned int devNum,P_Can_Msg canmsg, int Len, int timeou);  //接收CAN报文

int __stdcall   CANFD_Init(unsigned int devNum, P_CanFD_Config pInitConfig);
int __stdcall   CANFD_Transmit(unsigned int devNum, P_CanFD_Msg canmsg, unsigned int items, int timeout);
int __stdcall   CANFD_TransmitRt(unsigned int devNum, P_CanFD_Msg canmsg, unsigned int items, unsigned int *txitems, int timeou);   //定时发送CAN报文
int __stdcall   CANFD_GetReceiveNum(unsigned int devNum);//获取接收缓冲区中接收到但尚未被读取的帧数量
int __stdcall   CANFD_Receive(unsigned int devNum, P_CanFD_Msg canmsg, unsigned int Len, int timeout); //接收CANFD报文
#ifdef __cplusplus
}
#endif 

#endif



