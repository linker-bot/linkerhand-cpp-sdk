Attribute VB_Name = "PCANBasic"
'  PCANBasic.bas
'
'  ~~~~~~~~~~~~
'
'  PCAN-Basic API
'
'  ~~~~~~~~~~~~
'
'  ------------------------------------------------------------------
'  Author : Keneth Wagner
'  Last change: 2022-07-06
'
'  Language: Visual Basic 6.0
'  ------------------------------------------------------------------
'
'  Copyright (C) 1999-2022  PEAK-System Technik GmbH, Darmstadt
'  more Info at http://www.peak-system.com 
'

'///////////////////////////////////////////////////////////
'/ Value definitions
'///////////////////////////////////////////////////////////

' Currently defined and supported PCAN channels
'
Public Const PCAN_NONEBUS As Long = &H0   ' Undefined/default value for a PCAN bus

Public Const PCAN_ISABUS1 As Long = &H21  ' PCAN-ISA interface, channel 1
Public Const PCAN_ISABUS2 As Long = &H22  ' PCAN-ISA interface, channel 2
Public Const PCAN_ISABUS3 As Long = &H23  ' PCAN-ISA interface, channel 3
Public Const PCAN_ISABUS4 As Long = &H24  ' PCAN-ISA interface, channel 4
Public Const PCAN_ISABUS5 As Long = &H25  ' PCAN-ISA interface, channel 5
Public Const PCAN_ISABUS6 As Long = &H26  ' PCAN-ISA interface, channel 6
Public Const PCAN_ISABUS7 As Long = &H27  ' PCAN-ISA interface, channel 7
Public Const PCAN_ISABUS8 As Long = &H28  ' PCAN-ISA interface, channel 8

Public Const PCAN_DNGBUS1 As Long = &H31  ' PCAN-Dongle/LPT interface, channel 1

Public Const PCAN_PCIBUS1 As Long = &H41  ' PCAN-PCI interface, channel 1
Public Const PCAN_PCIBUS2 As Long = &H42  ' PCAN-PCI interface, channel 2
Public Const PCAN_PCIBUS3 As Long = &H43  ' PCAN-PCI interface, channel 3
Public Const PCAN_PCIBUS4 As Long = &H44  ' PCAN-PCI interface, channel 4
Public Const PCAN_PCIBUS5 As Long = &H45  ' PCAN-PCI interface, channel 5
Public Const PCAN_PCIBUS6 As Long = &H46  ' PCAN-PCI interface, channel 6
Public Const PCAN_PCIBUS7 As Long = &H47  ' PCAN-PCI interface, channel 7
Public Const PCAN_PCIBUS8 As Long = &H48  ' PCAN-PCI interface, channel 8
Public Const PCAN_PCIBUS9 As Long = &H409  ' PCAN-PCI interface, channel 9
Public Const PCAN_PCIBUS10 As Long = &H40A  ' PCAN-PCI interface, channel 10
Public Const PCAN_PCIBUS11 As Long = &H40B  ' PCAN-PCI interface, channel 11
Public Const PCAN_PCIBUS12 As Long = &H40C  ' PCAN-PCI interface, channel 12
Public Const PCAN_PCIBUS13 As Long = &H40D  ' PCAN-PCI interface, channel 13
Public Const PCAN_PCIBUS14 As Long = &H40E  ' PCAN-PCI interface, channel 14
Public Const PCAN_PCIBUS15 As Long = &H40F  ' PCAN-PCI interface, channel 15
Public Const PCAN_PCIBUS16 As Long = &H410  ' PCAN-PCI interface, channel 16

Public Const PCAN_USBBUS1 As Long = &H51  ' PCAN-USB interface, channel 1
Public Const PCAN_USBBUS2 As Long = &H52  ' PCAN-USB interface, channel 2
Public Const PCAN_USBBUS3 As Long = &H53  ' PCAN-USB interface, channel 3
Public Const PCAN_USBBUS4 As Long = &H54  ' PCAN-USB interface, channel 4
Public Const PCAN_USBBUS5 As Long = &H55  ' PCAN-USB interface, channel 5
Public Const PCAN_USBBUS6 As Long = &H56  ' PCAN-USB interface, channel 6
Public Const PCAN_USBBUS7 As Long = &H57  ' PCAN-USB interface, channel 7
Public Const PCAN_USBBUS8 As Long = &H58  ' PCAN-USB interface, channel 8
Public Const PCAN_USBBUS9 As Long = &H509  ' PCAN-USB interface, channel 9
Public Const PCAN_USBBUS10 As Long = &H50A  ' PCAN-USB interface, channel 10
Public Const PCAN_USBBUS11 As Long = &H50B  ' PCAN-USB interface, channel 11
Public Const PCAN_USBBUS12 As Long = &H50C  ' PCAN-USB interface, channel 12
Public Const PCAN_USBBUS13 As Long = &H50D  ' PCAN-USB interface, channel 13
Public Const PCAN_USBBUS14 As Long = &H50E  ' PCAN-USB interface, channel 14
Public Const PCAN_USBBUS15 As Long = &H50F  ' PCAN-USB interface, channel 15
Public Const PCAN_USBBUS16 As Long = &H510  ' PCAN-USB interface, channel 16

Public Const PCAN_PCCBUS1 As Long = &H61  ' PCAN-PC Card interface, channel 1
Public Const PCAN_PCCBUS2 As Long = &H62  ' PCAN-PC Card interface, channel 2

Public Const PCAN_LANBUS1 As Long = &H801  ' PCAN-LAN interface, channel 1
Public Const PCAN_LANBUS2 As Long = &H802  ' PCAN-LAN interface, channel 2
Public Const PCAN_LANBUS3 As Long = &H803  ' PCAN-LAN interface, channel 3
Public Const PCAN_LANBUS4 As Long = &H804  ' PCAN-LAN interface, channel 4
Public Const PCAN_LANBUS5 As Long = &H805  ' PCAN-LAN interface, channel 5
Public Const PCAN_LANBUS6 As Long = &H806  ' PCAN-LAN interface, channel 6
Public Const PCAN_LANBUS7 As Long = &H807  ' PCAN-LAN interface, channel 7
Public Const PCAN_LANBUS8 As Long = &H808  ' PCAN-LAN interface, channel 8
Public Const PCAN_LANBUS9 As Long = &H809  ' PCAN-LAN interface, channel 9
Public Const PCAN_LANBUS10 As Long = &H80A  ' PCAN-LAN interface, channel 10
Public Const PCAN_LANBUS11 As Long = &H80B  ' PCAN-LAN interface, channel 11
Public Const PCAN_LANBUS12 As Long = &H80C  ' PCAN-LAN interface, channel 12
Public Const PCAN_LANBUS13 As Long = &H80D  ' PCAN-LAN interface, channel 13
Public Const PCAN_LANBUS14 As Long = &H80E  ' PCAN-LAN interface, channel 14
Public Const PCAN_LANBUS15 As Long = &H80F  ' PCAN-LAN interface, channel 15
Public Const PCAN_LANBUS16 As Long = &H810  ' PCAN-LAN interface, channel 16

' Represent the PCAN error and status codes
'
Public Const PCAN_ERROR_OK As Long = &H0                 ' No error
Public Const PCAN_ERROR_XMTFULL As Long = &H1            ' Transmit buffer in CAN controller is full
Public Const PCAN_ERROR_OVERRUN As Long = &H2            ' CAN controller was read too late
Public Const PCAN_ERROR_BUSLIGHT As Long = &H4           ' Bus error: an error counter reached the 'light' limit [Not used with the *FD functions]
Public Const PCAN_ERROR_BUSHEAVY As Long = &H8           ' Bus error: an error counter reached the 'heavy' limit
Public Const PCAN_ERROR_BUSWARNING = PCAN_ERROR_BUSHEAVY ' An error counter reached the 'warning' limit [ONLY used with the *FD functions]
Public Const PCAN_ERROR_BUSPASSIVE As Long = &H40000     ' Bus error: the CAN controller is in bus-off state
Public Const PCAN_ERROR_BUSOFF As Long = &H10            ' Bus error: the CAN controller is in bus-off state
Public Const PCAN_ERROR_ANYBUSERR = (PCAN_ERROR_BUSWARNING Or PCAN_ERROR_BUSLIGHT Or PCAN_ERROR_BUSHEAVY Or PCAN_ERROR_BUSOFF Or PCAN_ERROR_BUSPASSIVE)    ' Mask for all bus errors
Public Const PCAN_ERROR_QRCVEMPTY As Long = &H20         ' Receive queue is empty
Public Const PCAN_ERROR_QOVERRUN As Long = &H40          ' Receive queue was read too late
Public Const PCAN_ERROR_QXMTFULL As Long = &H80          ' Transmit queue is full
Public Const PCAN_ERROR_REGTEST As Long = &H100          ' Test of the CAN controller hardware registers failed (no hardware found)
Public Const PCAN_ERROR_NODRIVER As Long = &H200         ' Driver not loaded
Public Const PCAN_ERROR_HWINUSE As Long = &H400          ' Hardware already in use by a Net
Public Const PCAN_ERROR_NETINUSE As Long = &H800         ' A Client is already connected to the Net
Public Const PCAN_ERROR_ILLHW As Long = &H1400           ' Hardware handle is invalid
Public Const PCAN_ERROR_ILLNET As Long = &H1800          ' Net handle is invalid
Public Const PCAN_ERROR_ILLCLIENT As Long = &H1C00       ' Client handle is invalid
Public Const PCAN_ERROR_ILLHANDLE = (PCAN_ERROR_ILLHW Or PCAN_ERROR_ILLNET Or PCAN_ERROR_ILLCLIENT)     ' Mask for all handle errors
Public Const PCAN_ERROR_RESOURCE As Long = &H2000        ' Resource (FIFO, Client, timeout) cannot be created
Public Const PCAN_ERROR_ILLPARAMTYPE As Long = &H4000    ' Invalid parameter
Public Const PCAN_ERROR_ILLPARAMVAL As Long = &H8000     ' Invalid parameter value
Public Const PCAN_ERROR_UNKNOWN As Long = &H10000        ' Unknown error
Public Const PCAN_ERROR_ILLDATA As Long = &H20000        ' Invalid data, function, or action
Public Const PCAN_ERROR_ILLMODE As Long = &H80000        ' Driver object state is wrong for the attempted operation
Public Const PCAN_ERROR_CAUTION As Long = &H2000000      ' An operation was successfully carried out, however, irregularities were registered
Public Const PCAN_ERROR_INITIALIZE As Long = &H4000000   ' Channel is not initialized [Value was changed from &H40000 to &H4000000]
Public Const PCAN_ERROR_ILLOPERATION As Long = &H8000000 ' Invalid operation [Value was changed from &H80000 to &H8000000]

' PCAN devices
'
Public Const PCAN_NONE = &H0        ' Undefined, unknown or not selected PCAN device value
Public Const PCAN_PEAKCAN = &H1     ' PCAN Non-PnP devices. NOT USED WITHIN PCAN-Basic API
Public Const PCAN_ISA = &H2         ' PCAN-ISA, PCAN-PC/104, and PCAN-PC/104-Plus
Public Const PCAN_DNG = &H3         ' PCAN-Dongle
Public Const PCAN_PCI = &H4         ' PCAN-PCI, PCAN-cPCI, PCAN-miniPCI, and PCAN-PCI Express
Public Const PCAN_USB = &H5         ' PCAN-USB and PCAN-USB Pro
Public Const PCAN_PCC = &H6         ' PCAN-PC Card
Public Const PCAN_VIRTUAL = &H7     ' PCAN Virtual hardware. NOT USED WITHIN PCAN-Basic API
Public Const PCAN_LAN = &H8         ' PCAN Gateway devices

' PCAN parameters
'
Public Const PCAN_DEVICE_ID = &H1                  ' Device identifier parameter
Public Const PCAN_5VOLTS_POWER = &H2               ' 5-Volt power parameter
Public Const PCAN_RECEIVE_EVENT = &H3              ' PCAN receive event handler parameter
Public Const PCAN_MESSAGE_FILTER = &H4             ' PCAN message filter parameter
Public Const PCAN_API_VERSION = &H5                ' PCAN-Basic API version parameter
Public Const PCAN_CHANNEL_VERSION = &H6            ' PCAN device channel version parameter
Public Const PCAN_BUSOFF_AUTORESET = &H7           ' PCAN Reset-On-Busoff parameter
Public Const PCAN_LISTEN_ONLY = &H8                ' PCAN Listen-Only parameter
Public Const PCAN_LOG_LOCATION = &H9               ' Directory path for log files
Public Const PCAN_LOG_STATUS = &HA                 ' Debug-Log activation status
Public Const PCAN_LOG_CONFIGURE = &HB              ' Configuration of the debugged information (LOG_FUNCTION_***)
Public Const PCAN_LOG_TEXT = &HC                   ' Custom insertion of text into the log file
Public Const PCAN_CHANNEL_CONDITION = &HD          ' Availability status of a PCAN-Channel
Public Const PCAN_HARDWARE_NAME = &HE              ' PCAN hardware name parameter
Public Const PCAN_RECEIVE_STATUS = &HF             ' Message reception status of a PCAN-Channel
Public Const PCAN_CONTROLLER_NUMBER = &H10         ' CAN-Controller number of a PCAN-Channel
Public Const PCAN_TRACE_LOCATION = &H11            ' Directory path for PCAN trace files
Public Const PCAN_TRACE_STATUS = &H12              ' CAN tracing activation status
Public Const PCAN_TRACE_SIZE = &H13                ' Configuration of the maximum file size of a CAN trace.
Public Const PCAN_TRACE_CONFIGURE = &H14           ' Configuration of the trace file storing mode (TRACE_FILE_***)
Public Const PCAN_CHANNEL_IDENTIFYING = &H15       ' Physical identification of a USB based PCAN-Channel by blinking its associated LED
Public Const PCAN_CHANNEL_FEATURES = &H16          ' Capabilities of a PCAN device (FEATURE_***)
Public Const PCAN_BITRATE_ADAPTING = &H17          ' Using of an existing bit rate (PCAN-View connected to a channel)
Public Const PCAN_BITRATE_INFO = &H18              ' Configured bit rate as Btr0Btr1 value
Public Const PCAN_BITRATE_INFO_FD = &H19           ' Configured bit rate as TPCANBitrateFD string
Public Const PCAN_BUSSPEED_NOMINAL = &H1A          ' Configured nominal CAN Bus speed as Bits per seconds
Public Const PCAN_BUSSPEED_DATA = &H1B             ' Configured CAN data speed as Bits per seconds
Public Const PCAN_IP_ADDRESS = &H1C                ' Remote address of a LAN channel as string in IPv4 format
Public Const PCAN_LAN_SERVICE_STATUS = &H1D        ' Status of the Virtual PCAN-Gateway Service
Public Const PCAN_ALLOW_STATUS_FRAMES = &H1E       ' Status messages reception status within a PCAN-Channel
Public Const PCAN_ALLOW_RTR_FRAMES = &H1F          ' RTR messages reception status within a PCAN-Channel
Public Const PCAN_ALLOW_ERROR_FRAMES = &H20        ' Error messages reception status within a PCAN-Channel
Public Const PCAN_INTERFRAME_DELAY = &H21          ' Delay, in microseconds, between sending frames
Public Const PCAN_ACCEPTANCE_FILTER_11BIT = &H22   ' Filter over code and mask patterns for 11-Bit messages
Public Const PCAN_ACCEPTANCE_FILTER_29BIT = &H23   ' Filter over code and mask patterns for 29-Bit messages
Public Const PCAN_IO_DIGITAL_CONFIGURATION = &H24  ' Output mode Of 32 digital I/O pin Of a PCAN-USB Chip. 1: Output-Active 0 : Output Inactive
Public Const PCAN_IO_DIGITAL_VALUE = &H25          ' Value assigned To a 32 digital I/O pins Of a PCAN-USB Chip
Public Const PCAN_IO_DIGITAL_SET = &H26            ' Value assigned To a 32 digital I/O pins Of a PCAN-USB Chip - Multiple digital I/O pins To 1 = High
Public Const PCAN_IO_DIGITAL_CLEAR = &H27          ' Clear multiple digital I/O pins To 0
Public Const PCAN_IO_ANALOG_VALUE = &H28           ' Get value Of a Single analog input pin
Public Const PCAN_FIRMWARE_VERSION = &H29          ' Get the version of the firmware used by the device associated With a PCAN-Channel
Public Const PCAN_ATTACHED_CHANNELS_COUNT = &H2A   ' Get the amount of PCAN channels attached to a system
Public Const PCAN_ATTACHED_CHANNELS = &H2B         ' Get information about PCAN channels attached to a system
Public Const PCAN_ALLOW_ECHO_FRAMES = &H2C         ' Echo messages reception status within a PCAN-Channel
Public Const PCAN_DEVICE_PART_NUMBER = &H2D        ' Get the part number associated To a device

' DEPRECATED parameters
'
Public Const PCAN_DEVICE_NUMBER = PCAN_DEVICE_ID   ' Deprecated parameter. Use PCAN_DEVICE_ID instead

' PCAN parameter values
'
Public Const PCAN_PARAMETER_OFF = &H0         ' The PCAN parameter is not set (inactive)
Public Const PCAN_PARAMETER_ON = &H1          ' The PCAN parameter is set (active)
Public Const PCAN_FILTER_CLOSE = &H0          ' The PCAN filter is closed. No messages will be received
Public Const PCAN_FILTER_OPEN = &H1           ' The PCAN filter is fully opened. All messages will be received
Public Const PCAN_FILTER_CUSTOM = &H2         ' The PCAN filter is custom configured. Only registered messages will be received
Public Const PCAN_CHANNEL_UNAVAILABLE = &H0   ' The PCAN-Channel handle is illegal, or its associated hardware is not available
Public Const PCAN_CHANNEL_AVAILABLE = &H1     ' The PCAN-Channel handle is available to be connected (PnP Hardware: it means furthermore that the hardware is plugged-in)
Public Const PCAN_CHANNEL_OCCUPIED = &H2      ' The PCAN-Channel handle is valid, and is already being used
Public Const PCAN_CHANNEL_PCANVIEW = (PCAN_CHANNEL_AVAILABLE Or PCAN_CHANNEL_OCCUPIED) ' The PCAN-Channel handle is already being used by a PCAN-View application, but is available to connect

Public Const LOG_FUNCTION_DEFAULT = &H0       ' Logs system exceptions / errors
Public Const LOG_FUNCTION_ENTRY = &H1         ' Logs the entries to the PCAN-Basic API functions
Public Const LOG_FUNCTION_PARAMETERS = &H2    ' Logs the parameters passed to the PCAN-Basic API functions
Public Const LOG_FUNCTION_LEAVE = &H4         ' Logs the exits from the PCAN-Basic API functions
Public Const LOG_FUNCTION_WRITE = &H8         ' Logs the CAN messages passed to the CAN_Write function
Public Const LOG_FUNCTION_READ = &H10         ' Logs the CAN messages received within the CAN_Read function
Public Const LOG_FUNCTION_ALL = &HFFFF        ' Logs all possible information within the PCAN-Basic API functions

Public Const TRACE_FILE_SINGLE = &H0          ' A single file is written until it size reaches PAN_TRACE_SIZE. 
Public Const TRACE_FILE_SEGMENTED = &H1       ' Traced data is distributed in several files with size PAN_TRACE_SIZE.
Public Const TRACE_FILE_DATE = &H2            ' Includes the date into the name of the trace file.
Public Const TRACE_FILE_TIME = &H4            ' Includes the start time into the name of the trace file.
Public Const TRACE_FILE_OVERWRITE = &H80      ' Causes the overwriting of available traces (same name).

Public Const FEATURE_FD_CAPABLE = &H1         ' Device supports flexible data-rate (CAN-FD)
Public Const FEATURE_DELAY_CAPABLE = &H2      ' Device supports a delay between sending frames (FPGA based USB devices)
Public Const FEATURE_IO_CAPABLE = &H4         ' Device supports I/O functionality for electronic circuits (USB-Chip devices)

Public Const SERVICE_STATUS_STOPPED = &H1     ' The service is not running
Public Const SERVICE_STATUS_RUNNING = &H4     ' The service is running

' Other constants
'
Public Const MAX_LENGTH_HARDWARE_NAME = 33    ' Maximum length of the name of a device: 32 characters + terminator
Public Const MAX_LENGTH_VERSION_STRING = 256  ' Maximum length of a version string: 255 characters + terminator

' PCAN message types
'
Public Const PCAN_MESSAGE_STANDARD = &H0      ' The PCAN message is a CAN Standard Frame (11-bit identifier)
Public Const PCAN_MESSAGE_RTR = &H1           ' The PCAN message is a CAN Remote-Transfer-Request Frame
Public Const PCAN_MESSAGE_EXTENDED = &H2      ' The PCAN message is a CAN Extended Frame (29-bit identifier)
Public Const PCAN_MESSAGE_FD = &H4            ' The PCAN message represents a FD frame in terms of CiA Specs
Public Const PCAN_MESSAGE_BRS = &H8           ' The PCAN message represents a FD bit rate switch (CAN data at a higher bit rate)
Public Const PCAN_MESSAGE_ESI = &H10          ' The PCAN message represents a FD error state indicator(CAN FD transmitter was error active)
Public Const PCAN_MESSAGE_ECHO = &H20         ' The PCAN message represents an echo CAN Frame
Public Const PCAN_MESSAGE_ERRFRAME = &H40     ' The PCAN message represents an error frame
Public Const PCAN_MESSAGE_STATUS = &H80       ' The PCAN message represents a PCAN status message

' LookUp Parameters
'
Public Const LOOKUP_DEVICE_TYPE As String = "devicetype"             ' Lookup channel by Device type (see PCAN devices e.g. PCAN_USB)
Public Const LOOKUP_DEVICE_ID As String = "deviceid"                 ' Lookup channel by device id
Public Const LOOKUP_CONTROLLER_NUMBER As String = "controllernumber" ' Lookup channel by CAN controller 0-based index
Public Const LOOKUP_IP_ADDRESS As String = "ipaddress"               ' Lookup channel by IP address (LAN channels only)

' Frame Type / Initialization Mode
'
Public Const PCAN_MODE_STANDARD = PCAN_MESSAGE_STANDARD
Public Const PCAN_MODE_EXTENDED = PCAN_MESSAGE_EXTENDED

' Baud rate codes = BTR0/BTR1 register values for the CAN controller.
' You can define your own Baud rate with the BTROBTR1 register.
' Take a look at www.peak-system.com for our free software "BAUDTOOL"
' to calculate the BTROBTR1 register for every bit rate and sample point.
'
Public Const PCAN_BAUD_1M = &H14                '   1 MBit/s
Public Const PCAN_BAUD_800K = &H16              ' 800 kBit/s
Public Const PCAN_BAUD_500K = &H1C              ' 500 kBit/s
Public Const PCAN_BAUD_250K = &H11C             ' 250 kBit/s
Public Const PCAN_BAUD_125K = &H31C             ' 125 kBit/s
Public Const PCAN_BAUD_100K = &H432F            ' 100 kBit/s
Public Const PCAN_BAUD_95K = &HC34E             '  95,238 kBit/s
Public Const PCAN_BAUD_83K = &H852B             '  83,333 kBit/s
Public Const PCAN_BAUD_50K = &H472F             '  50 kBit/s
Public Const PCAN_BAUD_47K = &H1414             '  47,619 kBit/s
Public Const PCAN_BAUD_33K = &H8B2F             '  33,333 kBit/s
Public Const PCAN_BAUD_20K = &H532F             '  20 kBit/s
Public Const PCAN_BAUD_10K = &H672F             '  10 kBit/s
Public Const PCAN_BAUD_5K = &H7F7F              '   5 kBit/s

' Represents the configuration for a CAN bit rate
' Note: 
'    * Each parameter and its value must be separated with a '='.
'    * Each pair of parameter/value must be separated using ','. 
'
' Example:
'    f_clock=80000000,nom_brp=10,nom_tseg1=5,nom_tseg2=2,nom_sjw=1,data_brp=4,data_tseg1=7,data_tseg2=2,data_sjw=1
'
Public Const PCAN_BR_CLOCK As String = "f_clock"
Public Const PCAN_BR_CLOCK_MHZ As String = "f_clock_mhz"
Public Const PCAN_BR_NOM_BRP As String = "nom_brp"
Public Const PCAN_BR_NOM_TSEG1 As String = "nom_tseg1"
Public Const PCAN_BR_NOM_TSEG2 As String = "nom_tseg2"
Public Const PCAN_BR_NOM_SJW As String = "nom_sjw"
Public Const PCAN_BR_NOM_SAMPLE As String = "nom_sam"
Public Const PCAN_BR_DATA_BRP As String = "data_brp"
Public Const PCAN_BR_DATA_TSEG1 As String = "data_tseg1"
Public Const PCAN_BR_DATA_TSEG2 As String = "data_tseg2"
Public Const PCAN_BR_DATA_SJW As String = "data_sjw"
Public Const PCAN_BR_DATA_SAMPLE As String = "data_ssp_offset"

' Type of PCAN (Non-PnP) hardware
' 
Public Const PCAN_TYPE_ISA = &H1              ' PCAN-ISA 82C200
Public Const PCAN_TYPE_ISA_SJA = &H9          ' PCAN-ISA SJA1000
Public Const PCAN_TYPE_ISA_PHYTEC = &H4       ' PHYTEC ISA
Public Const PCAN_TYPE_DNG = &H2              ' PCAN-Dongle 82C200
Public Const PCAN_TYPE_DNG_EPP = &H3          ' PCAN-Dongle EPP 82C200
Public Const PCAN_TYPE_DNG_SJA = &H5          ' PCAN-Dongle SJA1000
Public Const PCAN_TYPE_DNG_SJA_EPP = &H6      ' PCAN-Dongle EPP SJA1000

' Represents a PCAN message
'
Public Type TPCANMsg
    ID As Long                    ' 11/29-bit message identifier
    MsgType As Byte               ' Type of the message
    LEN As Byte                   ' Data Length Code of the message (0..8)
    DATA(7) As Byte               ' Data of the message (DATA[0]..DATA[7])
End Type

' Represents a timestamp of a received PCAN message
' Total Microseconds = micros + 1000 * millis + 0x100000000 * 1000 * millis_overflow
'
Public Type TPCANTimestamp
    millis As Long                ' base-value: milliseconds: 0.. 2^32-1
    millis_overflow As Integer    ' roll-arounds of millis
    micros As Integer             ' microseconds: 0..999
End Type

' Represents a PCAN message from a FD capable hardware
'
Public Type TPCANMsgFD
    ID As Long                    ' 11/29-bit message identifier
    MsgType As Byte               ' Type of the message
    DLC As Byte                   ' Data Length Code of the message (0..15)
    DATA(63) As Byte              ' Data of the message (DATA[0]..DATA[63])
End Type

' Describes an available PCAN channel
'
Public Type TPCANChannelInformation
    channel_handle As Integer                       ' PCAN channel handle   
    device_type As Byte                             ' Kind Of PCAN device
    controller_number As Byte                       ' CAN-Controller number
    device_features As Long                         ' Device capabilities flag (see FEATURE_*)
    device_name(MAX_LENGTH_HARDWARE_NAME) As Byte   ' Device name
    device_id As Long                               ' Device number
    channel_condition As Long                       ' Availability status of a PCAN-Channel      
End Type

''' <summary>
''' Initializes a PCAN Channel
''' </summary>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="Btr0Btr1">"The speed for the communication (BTR0BTR1 code)"</param>
''' <param name="HwType">"Non-PnP: The type of hardware and operation mode"</param>
''' <param name="IOPort">"Non-PnP: The I/O address for the parallel port"</param>
''' <param name="Interrupt">"Non-PnP: Interrupt number of the parallel port"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_Initialize Lib "PCANBasic.DLL" _
    (ByVal channel As Integer, _
    ByVal Btr0Btr1 As Integer, _
    Optional ByVal HwType As Byte = 0, _
    Optional ByVal IOPort As Long = 0, _
    Optional ByVal Interrupt As Integer = 0) As Long


''' <summary>
''' Initializes a FD capable PCAN Channel  
''' </summary>
''' <param name="Channel">"The handle of a FD capable PCAN Channel"</param>
''' <param name="BitrateFD">"The speed for the communication (FD bit rate string)"</param>
''' <remarks>See PCAN_BR_* values
''' * parameter and values ust be separated by '='
''' * Couples of Parameter/value must be separated by ','
''' * Following Parameter must be filled out: f_clock, data_brp, data_sjw, data_tseg1, data_tseg2,
'''   nom_brp, nom_sjw, nom_tseg1, nom_tseg2.
''' * Following Parameters are optional (not used yet): data_ssp_offset, nom_sam
'''</remarks>
''' <example>f_clock=80000000,nom_brp=10,nom_tseg1=5,nom_tseg2=2,nom_sjw=1,data_brp=4,data_tseg1=7,data_tseg2=2,data_sjw=1</example>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_InitializeFD Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByVal BitrateFD As String) As Long


''' <summary>
''' Uninitializes one or all PCAN Channels initialized by CAN_Initialize
''' </summary>
''' <remarks>Giving the TPCANHandle value "PCAN_NONEBUS", 
''' uninitialize all initialized channels</remarks>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_Uninitialize Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer) As Long


''' <summary>
''' Resets the receive and transmit queues of the PCAN Channel  
''' </summary>
''' <remarks>
''' A reset of the CAN controller is not performed.
''' </remarks>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_Reset Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer) As Long


''' <summary>
''' Gets the current status of a PCAN Channel 
''' </summary>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_GetStatus Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer) As Long


''' <summary>
''' Reads a CAN message from the receive queue of a PCAN Channel 
''' </summary>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="MessageBuffer">"A TPCANMsg structure buffer to store the CAN message"</param>
''' <param name="TimestampBuffer">"A TPCANTimestamp structure buffer to get 
''' the reception time of the message. If this value is not desired, this parameter
''' should be passed as NULL"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_Read Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByRef MessageBuffer As TPCANMsg, _
    ByRef TimestampBuffer As TPCANTimestamp) As Long


''' <summary>
''' Reads a CAN message from the receive queue of a FD capable PCAN Channel 
''' </summary>
''' <param name="Channel">"The handle of a FD capable PCAN Channel"</param>
''' <param name="MessageBuffer">"A TPCANMsgFD structure buffer to store the CAN message"</param>
''' <param name="TimestampBuffer">"A TPCANTimestampFD buffer to get 
''' the reception time of the message. If this value is not desired, this parameter
''' should be passed as NULL"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_ReadFD Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer,
    ByRef MessageBuffer As TPCANMsgFD,
    ByRef TimestampBuffer As Long) As Long


''' <summary>
''' Transmits a CAN message over a FD capable PCAN Channel
''' </summary>
''' <param name="Channel">"The handle of a FD capable PCAN Channel"</param>
''' <param name="MessageBuffer">"A TPCANMsgFD buffer with the message to be sent"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_WriteFD Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByRef MessageBuffer As TPCANMsgFD) As Long


''' <summary>
''' Transmits a CAN message 
''' </summary>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="MessageBuffer">"A TPCANMsg buffer with the message to be sent"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_Write Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByRef MessageBuffer As TPCANMsg) As Long


''' <summary>
''' Configures the reception filter. 
''' </summary>
''' <remarks>The message filter will be expanded with every call to 
''' this function. If it is desired to reset the filter, please use 
''' the CAN_SetValue function</remarks>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="FromID">"The lowest CAN ID to be received"</param>
''' <param name="ToID">"The highest CAN ID to be received"</param>
''' <param name="Mode">"Message type, Standard (11-bit identifier) or 
''' Extended (29-bit identifier)"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_FilterMessages Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByVal FromID As Long, _
    ByVal ToID As Long, _
    ByVal Mode As Byte) As Long


''' <summary>
''' Retrieves a PCAN Channel value
''' </summary>
''' <remarks>Parameters can be present or not according with the kind 
''' of Hardware (PCAN Channel) being used. If a parameter is not available,
''' a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="Parameter">"The TPCANParameter parameter to get"</param>
''' <param name="Buffer">"Buffer for the parameter value"</param>
''' <param name="BufferLength">"Size in bytes of the buffer"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_GetValue Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByVal Parameter As Byte, _
    ByRef Buffer As Any, _
    ByVal BufferLength As Long) As Long


''' <summary>
''' Configures or sets a PCAN Channel value 
''' </summary>
''' <remarks>Parameters can be present or not according with the kind 
''' of Hardware (PCAN Channel) being used. If a parameter is not available,
''' a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
''' <param name="Channel">"The handle of a PCAN Channel"</param>
''' <param name="Parameter">"The TPCANParameter parameter to set"</param>
''' <param name="Buffer">"Buffer with the value to be set"</param>
''' <param name="BufferLength">"Size in bytes of the buffer"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_SetValue Lib "PCANBasic.DLL" _
    (ByVal Channel As Integer, _
    ByVal Parameter As Byte, _
    ByRef Buffer As Any, _
    ByVal BufferLength As Long) As Long


''' <summary>
''' Returns a descriptive text of a given TPCANStatus error 
''' code, in any desired language
''' </summary>
''' <remarks>The current languages available for translation are: 
''' Neutral (0x00), German (0x07), English (0x09), Spanish (0x0A),
''' Italian (0x10) and French (0x0C)</remarks>
''' <param name="Error">"A TPCANStatus error code"</param>
''' <param name="Language">"Indicates a 'Primary language ID'"</param>
''' <param name="Buffer">"Buffer for a null terminated char array"</param>
''' <returns>"A TPCANStatus error code"</returns>
Public Declare Function CAN_GetErrorText Lib "PCANBasic.DLL" _
    (ByVal ErrorCode As Long,
    ByVal Language As Integer,
    ByVal Buffer As String) As Long

''' <summary>
''' Finds a PCAN-Basic channel that matches with the given parameters
''' </summary>
''' <param name="Parameters">A comma separated string contained pairs of 
''' parameter-name/value to be matched within a PCAN-Basic channel</param>
''' <param name="FoundChannel">Buffer for returning the PCAN-Basic channel, 
''' when found</param>
''' <returns>A TPCANStatus error code</returns>
Public Declare Function CAN_LookUpChannel Lib "PCANBasic.DLL" _
    (ByVal Parameters As String,
    ByRef FoundChannel As Integer) As Long