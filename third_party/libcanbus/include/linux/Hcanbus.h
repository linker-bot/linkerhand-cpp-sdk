#ifndef CANBUS_H
#define CANBUS_H

#include <stdio.h>

#if defined(_WIN32) || defined(__CYGWIN__) || defined(_WIN32_WCE)
#define LIBUSB_CALL __stdcall
#else
#define LIBUSB_CALL
#endif

typedef unsigned int u32;
typedef unsigned char u8;

typedef struct _Dev_Info {
    char HW_Type[32];
    char HW_Ser[32];
    char HW_Ver[32];
    char FW_Ver[32];
    char MF_Date[32];
} Dev_Info, *PDev_Info;

typedef struct _Can_Config {
    unsigned int Baudrate;
    unsigned short Pres;
    unsigned char Tseg1;
    unsigned char Tseg2;
    unsigned char SJW;
    unsigned char Config;
    unsigned char Model;
    unsigned char Reserved;
} Can_Config, *P_Can_Config;

typedef struct _CanFD_Config {
    unsigned int NomBaud;
    unsigned int DatBaud;
    unsigned short NomPre;
    unsigned char NomTseg1;
    unsigned char NomTseg2;
    unsigned char NomSJW;
    unsigned char DatPre;
    unsigned char DatTseg1;
    unsigned char DatTseg2;
    unsigned char DatSJW;
    unsigned char Config;
    unsigned char Model;
    unsigned char Cantype;
} CanFD_Config, *P_CanFD_Config;

typedef struct _Can_Msg {
    unsigned int ID;
    unsigned int TimeStamp;
    unsigned char FrameType;
    unsigned char DataLen;
    unsigned char ExternFlag;
    unsigned char RemoteFlag;
    unsigned char BusSatus;
    unsigned char ErrSatus;
    unsigned char TECounter;
    unsigned char RECounter;
    unsigned char Data[8];
} Can_Msg, *P_Can_Msg;

typedef struct _CanFD_Msg {
    unsigned int ID;
    unsigned int TimeStamp;
    unsigned char FrameType;
    unsigned char DLC;
    unsigned char ExternFlag;
    unsigned char RemoteFlag;
    unsigned char BusSatus;
    unsigned char ErrSatus;
    unsigned char TECounter;
    unsigned char RECounter;
    unsigned char Data[64];
} CanFD_Msg, *P_CanFD_Msg;

typedef struct _Can_Status {
    unsigned char BusSatus;
    unsigned char ErrSatus;
    unsigned char TECounter;
    unsigned char RECounter;
    unsigned int TimeStamp;
} Can_Status, *P_Can_Status;

#ifdef __cplusplus
extern "C" {
#endif

int LIBUSB_CALL CAN_ScanDevice(void);
int LIBUSB_CALL CAN_OpenDevice(u32 devNum, u32 chNum);
int LIBUSB_CALL CAN_CloseDevice(u32 devNum, u32 chNum);

int LIBUSB_CALL CAN_GetDevType(u32 devNum);
int LIBUSB_CALL CAN_GetDevPlck(u32 devNum);
int LIBUSB_CALL CAN_ReadDevInfo(u32 devNum, PDev_Info devinfo);

int LIBUSB_CALL CAN_GetTimeStamp(u32 devNum, u32 chNum, u32 *timestamp);
int LIBUSB_CALL CAN_SetTimeStamp(u32 devNum, u32 chNum, u32 timestamp, char mode);

int LIBUSB_CALL CAN_SetFilter(u32 devNum, u32 chNum, char namber, char type, u32 ftID, u32 ftMask, char enable);
int LIBUSB_CALL CAN_Reset(u32 devNum, u32 chNum);
int LIBUSB_CALL CAN_GetStatus(u32 devNum, u32 chNum, P_Can_Status status);
int LIBUSB_CALL CAN_Init(u32 devNum, u32 chNum, P_Can_Config pInitConfig);
int LIBUSB_CALL CAN_Transmit(u32 devNum, u32 chNum, P_Can_Msg canmsg, u32 items, int timeou);
int LIBUSB_CALL CAN_TransmitRt(u32 devNum, u32 chNum, P_Can_Msg canmsg, u32 items, u32 *txitems, int timeou);
int LIBUSB_CALL CAN_GetReceiveNum(u32 devNum, u32 chNum);
int LIBUSB_CALL CAN_Receive(u32 devNum, u32 chNum, P_Can_Msg canmsg, int Len, int timeou);

int LIBUSB_CALL CANFD_Init(u32 devNum, u32 chNum, P_CanFD_Config pInitConfig);
int LIBUSB_CALL CANFD_Transmit(u32 devNum, u32 chNum, P_CanFD_Msg canmsg, u32 items, int timeout);
int LIBUSB_CALL CANFD_TransmitRt(u32 devNum, u32 chNum, P_CanFD_Msg canmsg, u32 items, u32 *txitems, int timeou);
int LIBUSB_CALL CANFD_GetReceiveNum(u32 devNum, u32 chNum);
int LIBUSB_CALL CANFD_Receive(u32 devNum, u32 chNum, P_CanFD_Msg canmsg, int Len, int timeout);

#ifdef __cplusplus
}
#endif

#endif
