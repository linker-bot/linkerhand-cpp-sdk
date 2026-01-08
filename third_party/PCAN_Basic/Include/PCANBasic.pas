//  PCANBasic.pas
//
//  ~~~~~~~~~~~~
//
//  PCAN-Basic API
//
//  ~~~~~~~~~~~~
//
//  ------------------------------------------------------------------
//  Author : Keneth Wagner
//  Last change: 2022-07-06
//
//  Language: Object Pascal
//  ------------------------------------------------------------------
//
//  Copyright (C) 1999-2022  PEAK-System Technik GmbH, Darmstadt
//  more Info at http://www.peak-system.com
//
unit PCANBasic;

interface
Const
    // Other constants
    //
    MAX_LENGTH_HARDWARE_NAME = 33;     // Maximum length of the name of a device: 32 characters + terminator
    MAX_LENGTH_VERSION_STRING = 256;     // Maximum length of a version string: 255 characters + terminator

type
  TPCANHandle = Word;
  PTPCANHandle = ^Word;
  TPCANBitrateFD = PAnsiChar;
  TPCANTimestampFD = UInt64;
  PUInt64 = ^UInt64;

{$Z4}
  /// <summary>
  /// Represents a PCAN status/error code
  /// </summary>
  TPCANStatus = (
    /// <summary>
    /// No error
    /// </summary>
    PCAN_ERROR_OK = $00000,
    /// <summary>
    /// Transmit buffer in CAN controller is full
    /// </summary>
    PCAN_ERROR_XMTFULL = $00001,
    /// <summary>
    /// CAN controller was read too late
    /// </summary>
    PCAN_ERROR_OVERRUN = $00002,
    /// <summary>
    /// Bus error: an error counter reached the 'light' limit
    /// </summary>
    PCAN_ERROR_BUSLIGHT = $00004,
    /// <summary>
    /// Bus error: an error counter reached the 'heavy' limit
    /// </summary>
    PCAN_ERROR_BUSHEAVY = $00008,
    /// <summary>
    /// Bus error: an error counter reached the 'warning' limit
    /// </summary>
    PCAN_ERROR_BUSWARNING = LongWord(PCAN_ERROR_BUSHEAVY),
    /// <summary>
    /// Bus error: the CAN controller is error passive
    /// </summary>
    PCAN_ERROR_BUSPASSIVE = $40000,
    /// <summary>
    /// Bus error: the CAN controller is in bus-off state
    /// </summary>
    PCAN_ERROR_BUSOFF = $00010,
    /// <summary>
    /// Mask for all bus errors
    /// </summary>
    PCAN_ERROR_ANYBUSERR = LongWord(PCAN_ERROR_BUSWARNING) Or LongWord(PCAN_ERROR_BUSLIGHT) Or LongWord(PCAN_ERROR_BUSHEAVY) Or LongWord(PCAN_ERROR_BUSOFF) Or LongWord(PCAN_ERROR_BUSPASSIVE),
    /// <summary>
    /// Receive queue is empty
    /// </summary>
    PCAN_ERROR_QRCVEMPTY = $00020,
    /// <summary>
    /// Receive queue was read too late
    /// </summary>
    PCAN_ERROR_QOVERRUN = $00040,
    /// <summary>
    /// Transmit queue is full
    /// </summary>
    PCAN_ERROR_QXMTFULL = $00080,
    /// <summary>
    /// Test of the CAN controller hardware registers failed (no hardware found)
    /// </summary>
    PCAN_ERROR_REGTEST = $00100,
    /// <summary>
    /// Driver not loaded
    /// </summary>
    PCAN_ERROR_NODRIVER = $00200,
    /// <summary>
    /// Hardware already in use by a Net
    /// </summary>
    PCAN_ERROR_HWINUSE = $00400,
    /// <summary>
    /// A Client is already connected to the Net
    /// </summary>
    PCAN_ERROR_NETINUSE = $00800,
    /// <summary>
    /// Hardware handle is invalid
    /// </summary>
    PCAN_ERROR_ILLHW = $01400,
    /// <summary>
    /// Net handle is invalid
    /// </summary>
    PCAN_ERROR_ILLNET = $01800,
    /// <summary>
    /// Client handle is invalid
    /// </summary>
    PCAN_ERROR_ILLCLIENT = $01C00,
    /// <summary>
    /// Mask for all handle errors
    /// </summary>
    PCAN_ERROR_ILLHANDLE = LongWord(PCAN_ERROR_ILLHW) Or LongWord(PCAN_ERROR_ILLNET) Or LongWord(PCAN_ERROR_ILLCLIENT),
    /// <summary>
    /// Resource (FIFO, Client, timeout) cannot be created
    /// </summary>
    PCAN_ERROR_RESOURCE = $02000,
    /// <summary>
    /// Invalid parameter
    /// </summary>
    PCAN_ERROR_ILLPARAMTYPE = $04000,
    /// <summary>
    /// Invalid parameter value
    /// </summary>
    PCAN_ERROR_ILLPARAMVAL = $08000,
    /// <summary>
    /// Unknown error
    /// </summary>
    PCAN_ERROR_UNKNOWN = $10000,
    /// <summary>
    /// Invalid data, function, or action
    /// </summary>
    PCAN_ERROR_ILLDATA = $20000,
    /// <summary>
    /// Driver object state is wrong for the attempted operation
    /// </summary>
    PCAN_ERROR_ILLMODE = $80000,
    /// <summary>
    /// An operation was successfully carried out, however, irregularities were registered
    /// </summary>
    PCAN_ERROR_CAUTION = $2000000,
    /// <summary>
    /// Channel is not initialized
    /// <remarks>Value was changed from 0x40000 to 0x4000000</remarks>
    /// </summary>
    PCAN_ERROR_INITIALIZE = $4000000,
    /// <summary>
    /// Invalid operation
    /// <remarks>Value was changed from 0x80000 to 0x8000000</remarks>
    /// </summary>
    PCAN_ERROR_ILLOPERATION = $8000000
  );

{$Z1}
  /// <summary>
  /// Represents a PCAN device
  /// </summary>
  TPCANDevice = (
    /// <summary>
    /// Undefined, unknown or not selected PCAN device value
    /// </summary>
    PCAN_NONE = 0,
    /// <summary>
    /// PCAN Non-PnP devices. NOT USED WITHIN PCAN-Basic API
    /// </summary>
    PCAN_PEAKCAN = 1,
    /// <summary>
    /// PCAN-ISA, PCAN-PC/104, and PCAN-PC/104-Plus
    /// </summary>
    PCAN_ISA = 2,
    /// <summary>
    /// PCAN-Dongle
    /// </summary>
    PCAN_DNG = 3,
    /// <summary>
    /// PCAN-PCI, PCAN-cPCI, PCAN-miniPCI, and PCAN-PCI Express
    /// </summary>
    PCAN_PCI = 4,
    /// <summary>
    /// PCAN-USB and PCAN-USB Pro
    /// </summary>
    PCAN_USB = 5,
    /// <summary>
    /// PCAN-PC Card
    /// </summary>
    PCAN_PCC = 6,
    /// <summary>
    /// PCAN Virtual hardware. NOT USED WITHIN PCAN-Basic API
    /// </summary>
    PCAN_VIRTUAL = 7,
    /// <summary>
    /// PCAN Gateway devices
    /// </summary>
    PCAN_LAN = 8
  );

  /// <summary>
  /// Represents a PCAN parameter to be read or set
  /// </summary>
  TPCANParameter = (
    /// <summary>
    /// Device identifier parameter
    /// </summary>
    PCAN_DEVICE_ID = 1,
    /// <summary>
    /// DEPRECATED. Use PCAN_DEVICE_ID instead
    /// </summary>
    PCAN_DEVICE_NUMBER = PCAN_DEVICE_ID,
    /// <summary>
    /// 5-Volt power parameter
    /// </summary>
    PCAN_5VOLTS_POWER = 2,
    /// <summary>
    /// PCAN receive event handler parameter
    /// </summary>
    PCAN_RECEIVE_EVENT = 3,
    /// <summary>
    /// PCAN message filter parameter
    /// </summary>
    PCAN_MESSAGE_FILTER = 4,
    /// <summary>
    /// PCAN-Basic API version parameter
    /// </summary>
    PCAN_API_VERSION = 5,
    /// <summary>
    /// PCAN device channel version parameter
    /// </summary>
    PCAN_CHANNEL_VERSION = 6,
    /// <summary>
    /// PCAN Reset-On-Busoff parameter
    /// </summary>
    PCAN_BUSOFF_AUTORESET = 7,
    /// <summary>
    /// PCAN Listen-Only parameter
    /// </summary>
    PCAN_LISTEN_ONLY = 8,
    /// <summary>
    /// Directory path for log files
    /// </summary>
    PCAN_LOG_LOCATION = 9,
    /// <summary>
    /// Debug-Log activation status
    /// </summary>
    PCAN_LOG_STATUS = 10,
    /// <summary>
    /// Configuration of the debugged information (LOG_FUNCTION_***)
    /// </summary>
    PCAN_LOG_CONFIGURE = 11,
    /// <summary>
    /// Custom insertion of text into the log file
    /// </summary>
    PCAN_LOG_TEXT = 12,
    /// <summary>
    /// Availability status of a PCAN-Channel
    /// </summary>
    PCAN_CHANNEL_CONDITION = 13,
    /// <summary>
    /// PCAN hardware name parameter
    /// </summary>
    PCAN_HARDWARE_NAME = 14,
    /// <summary>
    /// Message reception status of a PCAN-Channel
    /// </summary>
    PCAN_RECEIVE_STATUS = 15,
    /// <summary>
    /// CAN-Controller number of a PCAN-Channel
    /// </summary>
    PCAN_CONTROLLER_NUMBER = 16,
    /// <summary>
    /// Directory path for PCAN trace files
    /// </summary>
    PCAN_TRACE_LOCATION = 17,
    /// <summary>
    /// CAN tracing activation status
    /// </summary>
    PCAN_TRACE_STATUS = 18,
    /// <summary>
    /// Configuration of the maximum file size of a CAN trace
    /// </summary>
    PCAN_TRACE_SIZE = 19,
    /// <summary>
    /// Configuration of the trace file storing mode (TRACE_FILE_***)
    /// </summary>
    PCAN_TRACE_CONFIGURE = 20,
    /// <summary>
    /// Physical identification of a USB based PCAN-Channel by blinking its associated LED
    /// </summary>
    PCAN_CHANNEL_IDENTIFYING = 21,
    /// <summary>
    /// Capabilities of a PCAN device (FEATURE_***)
    /// </summary>
    PCAN_CHANNEL_FEATURES = 22,
    /// <summary>
    /// Using of an existing bit rate (PCAN-View connected to a channel)
    /// </summary>
    PCAN_BITRATE_ADAPTING = 23,
    /// <summary>
    /// Configured bit rate as Btr0Btr1 value
    /// </summary>
    PCAN_BITRATE_INFO = 24,
    /// <summary>
    /// Configured bit rate as TPCANBitrateFD string
    /// </summary>
    PCAN_BITRATE_INFO_FD = 25,
    /// <summary>
    /// Configured nominal CAN Bus speed as Bits per seconds
    /// </summary>
    PCAN_BUSSPEED_NOMINAL = 26,
    /// <summary>
    /// Configured CAN data speed as Bits per seconds
    /// </summary>
    PCAN_BUSSPEED_DATA = 27,
    /// <summary>
    /// Remote address of a LAN channel as string in IPv4 format
    /// </summary>
    PCAN_IP_ADDRESS = 28,
    /// <summary>
    /// Status of the Virtual PCAN-Gateway Service
    /// </summary>
    PCAN_LAN_SERVICE_STATUS = 29,
    /// <summary>
    /// Status messages reception status within a PCAN-Channel
    /// </summary>
    PCAN_ALLOW_STATUS_FRAMES = 30,
    /// <summary>
    /// RTR messages reception status within a PCAN-Channel
    /// </summary>
    PCAN_ALLOW_RTR_FRAMES = 31,
    /// <summary>
    /// Error messages reception status within a PCAN-Channel
    /// </summary>
    PCAN_ALLOW_ERROR_FRAMES = 32,
    /// <summary>
    /// Delay, in microseconds, between sending frames
    /// </summary>
    PCAN_INTERFRAME_DELAY = 33,
    /// <summary>
    /// Filter over code and mask patterns for 11-Bit messages
    /// </summary>
    PCAN_ACCEPTANCE_FILTER_11BIT = 34,
    /// <summary>
    /// Filter over code and mask patterns for 29-Bit messages
    /// </summary>
    PCAN_ACCEPTANCE_FILTER_29BIT = 35,
    /// <summary>
    /// Output mode of 32 digital I/O pin of a PCAN-USB Chip. 1: Output-Active 0 : Output Inactive
    /// </summary>
    PCAN_IO_DIGITAL_CONFIGURATION = 36,
    /// <summary>
    /// Value assigned to a 32 digital I/O pins of a PCAN-USB Chip
    /// </summary>
    PCAN_IO_DIGITAL_VALUE = 37,
    /// <summary>
    /// Value assigned to a 32 digital I/O pins of a PCAN-USB Chip - Multiple digital I/O pins to 1 = High
    /// </summary>
    PCAN_IO_DIGITAL_SET = 38,
    /// <summary>
    /// Clear multiple digital I/O pins to 0
    /// </summary>
    PCAN_IO_DIGITAL_CLEAR = 39,
    /// <summary>
    /// Get value of a single analog input pin
    /// </summary>
    PCAN_IO_ANALOG_VALUE = 40,
    /// <summary>
    /// Get the version of the firmware used by the device associated with a PCAN-Channel
    /// </summary>
    PCAN_FIRMWARE_VERSION = 41,
    /// <summary>
    /// Get the amount of PCAN channels attached to a system
    /// </summary>
    PCAN_ATTACHED_CHANNELS_COUNT = 42,
    /// <summary>
    /// Get information about PCAN channels attached to a system
    /// </summary>
    PCAN_ATTACHED_CHANNELS = 43,
    /// <summary>
    /// Echo messages reception status within a PCAN-Channel
    /// </summary>
    PCAN_ALLOW_ECHO_FRAMES = 44,
    /// <summary>
    /// Get the part number associated to a device
    /// </summary>
    PCAN_DEVICE_PART_NUMBER = 45
  );

  /// <summary>
  /// Represents the type of a PCAN message
  /// </summary>
  TPCANMessageType = (
    /// <summary>
    /// The PCAN message is a CAN Standard Frame (11-bit identifier)
    /// </summary>
    PCAN_MESSAGE_STANDARD = $00,
    /// <summary>
    /// The PCAN message is a CAN Remote-Transfer-Request Frame
    /// </summary>
    PCAN_MESSAGE_RTR = $01,
    /// <summary>
    /// The PCAN message is a CAN Extended Frame (29-bit identifier)
    /// </summary>
    PCAN_MESSAGE_EXTENDED = $02,
    /// <summary>
    /// The PCAN message represents a FD frame in terms of CiA Specs
    /// </summary>
    PCAN_MESSAGE_FD = $04,
    /// <summary>
    /// The PCAN message represents a FD bit rate switch (CAN data at a higher bit rate)
    /// </summary>
    PCAN_MESSAGE_BRS = $08,
    /// <summary>
    /// The PCAN message represents a FD error state indicator(CAN FD transmitter was error active)
    /// </summary>
    PCAN_MESSAGE_ESI = $10,
    /// <summary>
    /// The PCAN message represents an echo CAN Frame
    /// </summary>
    PCAN_MESSAGE_ECHO = $20,
    /// <summary>
    /// The PCAN message represents an error frame
    /// </summary>
    PCAN_MESSAGE_ERRFRAME = $40,
    /// <summary>
    /// The PCAN message represents a PCAN status message
    /// </summary>
    PCAN_MESSAGE_STATUS = $80
  );

  /// <summary>
  /// Represents a PCAN filter mode
  /// </summary>
  TPCANMode = (
    /// <summary>
    /// Mode is Standard (11-bit identifier)
    /// </summary>
    PCAN_MODE_STANDARD = Byte(PCAN_MESSAGE_STANDARD),
    /// <summary>
    /// Mode is Extended (29-bit identifier)
    /// </summary>
    PCAN_MODE_EXTENDED = Byte(PCAN_MESSAGE_EXTENDED)
  );

{$Z2}
  /// <summary>
  /// Represents a PCAN Baud rate register value
  /// </summary>
  TPCANBaudrate = (
    /// <summary>
    /// 1 MBit/s
    /// </summary>
    PCAN_BAUD_1M = $0014,
    /// <summary>
    /// 800 kBit/s
    /// </summary>
    PCAN_BAUD_800K = $0016,
    /// <summary>
    /// 500 kBit/s
    /// </summary>
    PCAN_BAUD_500K = $001C,
    /// <summary>
    /// 250 kBit/s
    /// </summary>
    PCAN_BAUD_250K = $011C,
    /// <summary>
    /// 125 kBit/s
    /// </summary>
    PCAN_BAUD_125K = $031C,
    /// <summary>
    /// 100 kBit/s
    /// </summary>
    PCAN_BAUD_100K = $432F,
    /// <summary>
    /// 95,238 kBit/s
    /// </summary>
    PCAN_BAUD_95K = $C34E,
    /// <summary>
    /// 83,333 kBit/s
    /// </summary>
    PCAN_BAUD_83K = $852B,
    /// <summary>
    /// 50 kBit/s
    /// </summary>
    PCAN_BAUD_50K = $472F,
    /// <summary>
    /// 47,619 kBit/s
    /// </summary>
    PCAN_BAUD_47K = $1414,
    /// <summary>
    /// 33,333 kBit/s
    /// </summary>
    PCAN_BAUD_33K = $8B2F,
    /// <summary>
    /// 20 kBit/s
    /// </summary>
    PCAN_BAUD_20K = $532F,
    /// <summary>
    /// 10 kBit/s
    /// </summary>
    PCAN_BAUD_10K = $672F,
    /// <summary>
    /// 5 kBit/s
    /// </summary>
    PCAN_BAUD_5K = $7F7F
  );

{$Z1}
  /// <summary>
  /// Represents the type of PCAN (Non-PnP) hardware to be initialized
  /// </summary>
  TPCANType = (
    /// <summary>
    /// PCAN-ISA 82C200
    /// </summary>
    PCAN_TYPE_ISA = $01,
    /// <summary>
    /// PCAN-ISA SJA1000
    /// </summary>
    PCAN_TYPE_ISA_SJA = $09,
    /// <summary>
    /// PHYTEC ISA
    /// </summary>
    PCAN_TYPE_ISA_PHYTEC = $04,
    /// <summary>
    /// PCAN-Dongle 82C200
    /// </summary>
    PCAN_TYPE_DNG = $02,
    /// <summary>
    /// PCAN-Dongle EPP 82C200
    /// </summary>
    PCAN_TYPE_DNG_EPP = $03,
    /// <summary>
    /// PCAN-Dongle SJA1000
    /// </summary>
    PCAN_TYPE_DNG_SJA = $05,
    /// <summary>
    /// PCAN-Dongle EPP SJA1000
    /// </summary>
    PCAN_TYPE_DNG_SJA_EPP = $06
  );

  /// <summary>
  /// Represents a PCAN message
  /// </summary>
  TPCANMsg = record
    /// <summary>
    /// 11/29-bit message identifier
    /// </summary>
    ID: Longword;
    /// <summary>
    /// Type of the message
    /// </summary>
    MSGTYPE: TPCANMessageType;
    /// <summary>
    /// Data Length Code of the message (0..8)
    /// </summary>
    LEN: Byte;
    /// <summary>
    /// Data of the message (DATA[0]..DATA[7])
    /// </summary>
    DATA: array[0..7] of Byte;
  end;
  PTPCANMsg = ^TPCANMsg;

  /// <summary>
  /// Represents a timestamp of a received PCAN message.
  /// Total Microseconds = micros + 1000 * millis + 0x100000000 * 1000 * millis_overflow
  /// </summary>
  TPCANTimestamp = record
    /// <summary>
    /// Base-value: milliseconds: 0.. 2^32-1
    /// </summary>
    millis: Longword;
    /// <summary>
    /// Roll-arounds of millis
    /// </summary>
    millis_overflow: Word;
    /// <summary>
    /// Microseconds: 0..999
    /// </summary>
    micros: Word;
  end;
  PTPCANTimestamp = ^TPCANTimestamp;

  /// <summary>
  /// Represents a PCAN message from a FD capable hardware
  /// </summary>
  TPCANMsgFD = record
    /// <summary>
    /// 11/29-bit message identifier
    /// </summary>
    ID: Longword;
    /// <summary>
    /// Type of the message
    /// </summary>
    MSGTYPE: TPCANMessageType;
    /// <summary>
    /// Data Length Code of the message (0..15)
    /// </summary>
    DLC: Byte;
    /// <summary>
    /// Data of the message (DATA[0]..DATA[63])
    /// </summary>
    DATA: array[0..63] of Byte;
  end;
  PTPCANMsgFD = ^TPCANMsgFD;


  /// <summary>
  /// Describes an available PCAN channel
  /// </summary>
  TPCANChannelInformation = record
    /// <summary>
    /// PCAN channel handle
    /// </summary>
    channel_handle: TPCANHandle;
    /// <summary>
    /// Kind of PCAN device
    /// </summary>
    device_type: TPCANDevice;
    /// <summary>
    /// CAN-Controller number
    /// </summary>
    controller_number: Byte;
    /// <summary>
    /// Device capabilities flag (see FEATURE_*)
    /// </summary>
    device_features: Longword;
    /// <summary>
    /// Device name
    /// </summary>
    device_name: array [0..MAX_LENGTH_HARDWARE_NAME-1] of AnsiChar;
    /// <summary>
    /// Device number
    /// </summary>
    device_id: Longword;
    /// <summary>
    /// Availability status of a PCAN-Channel
    /// </summary>
    channel_condition: Longword;
  end;
  PTPCANChannelInformation = ^TPCANChannelInformation;

  /// <summary>
  /// PCAN-Basic API class implementation
  /// </summary>
  TPCANBasic = class
  public
    class var
      /// <summary>
      /// Undefined/default value for a PCAN bus
      /// </summary>
      const PCAN_NONEBUS: TPCANHandle = $00;

      /// <summary>
      /// PCAN-ISA interface, channel 1
      /// </summary>
      const PCAN_ISABUS1: TPCANHandle = $21;
      /// <summary>
      /// PCAN-ISA interface, channel 2
      /// </summary>
      const PCAN_ISABUS2: TPCANHandle = $22;
      /// <summary>
      /// PCAN-ISA interface, channel 3
      /// </summary>
      const PCAN_ISABUS3: TPCANHandle = $23;
      /// <summary>
      /// PCAN-ISA interface, channel 4
      /// </summary>
      const PCAN_ISABUS4: TPCANHandle = $24;
      /// <summary>
      /// PCAN-ISA interface, channel 5
      /// </summary>
      const PCAN_ISABUS5: TPCANHandle = $25;
      /// <summary>
      /// PCAN-ISA interface, channel 6
      /// </summary>
      const PCAN_ISABUS6: TPCANHandle = $26;
      /// <summary>
      /// PCAN-ISA interface, channel 7
      /// </summary>
      const PCAN_ISABUS7: TPCANHandle = $27;
      /// <summary>
      /// PCAN-ISA interface, channel 8
      /// </summary>
      const PCAN_ISABUS8: TPCANHandle = $28;

      /// <summary>
      /// PPCAN-Dongle/LPT interface, channel 1
      /// </summary>
      const PCAN_DNGBUS1: TPCANHandle = $31;

      /// <summary>
      /// PCAN-PCI interface, channel 1
      /// </summary>
      const PCAN_PCIBUS1: TPCANHandle = $41;
      /// <summary>
      /// PCAN-PCI interface, channel 2
      /// </summary>
      const PCAN_PCIBUS2: TPCANHandle = $42;
      /// <summary>
      /// PCAN-PCI interface, channel 3
      /// </summary>
      const PCAN_PCIBUS3: TPCANHandle = $43;
      /// <summary>
      /// PCAN-PCI interface, channel 4
      /// </summary>
      const PCAN_PCIBUS4: TPCANHandle = $44;
      /// <summary>
      /// PCAN-PCI interface, channel 5
      /// </summary>
      const PCAN_PCIBUS5: TPCANHandle = $45;
      /// <summary>
      /// PCAN-PCI interface, channel 6
      /// </summary>
      const PCAN_PCIBUS6: TPCANHandle = $46;
      /// <summary>
      /// PCAN-PCI interface, channel 7
      /// </summary>
      const PCAN_PCIBUS7: TPCANHandle = $47;
      /// <summary>
      /// PCAN-PCI interface, channel 8
      /// </summary>
      const PCAN_PCIBUS8: TPCANHandle = $48;
      /// <summary>
      /// PCAN-PCI interface, channel 9
      /// </summary>
      const PCAN_PCIBUS9: TPCANHandle = $409;
      /// <summary>
      /// PCAN-PCI interface, channel 10
      /// </summary>
      const PCAN_PCIBUS10: TPCANHandle = $40A;
      /// <summary>
      /// PCAN-PCI interface, channel 11
      /// </summary>
      const PCAN_PCIBUS11: TPCANHandle = $40B;
      /// <summary>
      /// PCAN-PCI interface, channel 12
      /// </summary>
      const PCAN_PCIBUS12: TPCANHandle = $40C;
      /// <summary>
      /// PCAN-PCI interface, channel 13
      /// </summary>
      const PCAN_PCIBUS13: TPCANHandle = $40D;
      /// <summary>
      /// PCAN-PCI interface, channel 14
      /// </summary>
      const PCAN_PCIBUS14: TPCANHandle = $40E;
      /// <summary>
      /// PCAN-PCI interface, channel 15
      /// </summary>
      const PCAN_PCIBUS15: TPCANHandle = $40F;
      /// <summary>
      /// PCAN-PCI interface, channel 16
      /// </summary>
      const PCAN_PCIBUS16: TPCANHandle = $410;

      /// <summary>
      /// PCAN-USB interface, channel 1
      /// </summary>
      const PCAN_USBBUS1: TPCANHandle = $51;
      /// <summary>
      /// PCAN-USB interface, channel 2
      /// </summary>
      const PCAN_USBBUS2: TPCANHandle = $52;
      /// <summary>
      /// PCAN-USB interface, channel 3
      /// </summary>
      const PCAN_USBBUS3: TPCANHandle = $53;
      /// <summary>
      /// PCAN-USB interface, channel 4
      /// </summary>
      const PCAN_USBBUS4: TPCANHandle = $54;
      /// <summary>
      /// PCAN-USB interface, channel 5
      /// </summary>
      const PCAN_USBBUS5: TPCANHandle = $55;
      /// <summary>
      /// PCAN-USB interface, channel 6
      /// </summary>
      const PCAN_USBBUS6: TPCANHandle = $56;
      /// <summary>
      /// PCAN-USB interface, channel 7
      /// </summary>
      const PCAN_USBBUS7: TPCANHandle = $57;
      /// <summary>
      /// PCAN-USB interface, channel 8
      /// </summary>
      const PCAN_USBBUS8: TPCANHandle = $58;
      /// <summary>
      /// PCAN-USB interface, channel 9
      /// </summary>
      const PCAN_USBBUS9: TPCANHandle = $509;
      /// <summary>
      /// PCAN-USB interface, channel 10
      /// </summary>
      const PCAN_USBBUS10: TPCANHandle = $50A;
      /// <summary>
      /// PCAN-USB interface, channel 11
      /// </summary>
      const PCAN_USBBUS11: TPCANHandle = $50B;
      /// <summary>
      /// PCAN-USB interface, channel 12
      /// </summary>
      const PCAN_USBBUS12: TPCANHandle = $50C;
      /// <summary>
      /// PCAN-USB interface, channel 13
      /// </summary>
      const PCAN_USBBUS13: TPCANHandle = $50D;
      /// <summary>
      /// PCAN-USB interface, channel 14
      /// </summary>
      const PCAN_USBBUS14: TPCANHandle = $50E;
      /// <summary>
      /// PCAN-USB interface, channel 15
      /// </summary>
      const PCAN_USBBUS15: TPCANHandle = $50F;
      /// <summary>
      /// PCAN-USB interface, channel 16
      /// </summary>
      const PCAN_USBBUS16: TPCANHandle = $510;

      /// <summary>
      /// PCAN-PC Card interface, channel 1
      /// </summary>
      const PCAN_PCCBUS1: TPCANHandle = $61;
      /// <summary>
      /// PCAN-PC Card interface, channel 2
      /// </summary>
      const PCAN_PCCBUS2: TPCANHandle = $62;

      /// <summary>
      /// PCAN-LAN interface, channel 1
      /// </summary>
      const PCAN_LANBUS1: TPCANHandle = $801;
      /// <summary>
      /// PCAN-LAN interface, channel 2
      /// </summary>
      const PCAN_LANBUS2: TPCANHandle = $802;
      /// <summary>
      /// PCAN-LAN interface, channel 3
      /// </summary>
      const PCAN_LANBUS3: TPCANHandle = $803;
      /// <summary>
      /// PCAN-LAN interface, channel 4
      /// </summary>
      const PCAN_LANBUS4: TPCANHandle = $804;
      /// <summary>
      /// PCAN-LAN interface, channel 5
      /// </summary>
      const PCAN_LANBUS5: TPCANHandle = $805;
      /// <summary>
      /// PCAN-LAN interface, channel 6
      /// </summary>
      const PCAN_LANBUS6: TPCANHandle = $806;
      /// <summary>
      /// PCAN-LAN interface, channel 7
      /// </summary>
      const PCAN_LANBUS7: TPCANHandle = $807;
      /// <summary>
      /// PCAN-LAN interface, channel 8
      /// </summary>
      const PCAN_LANBUS8: TPCANHandle = $808;
      /// <summary>
      /// PCAN-LAN interface, channel 9
      /// </summary>
      const PCAN_LANBUS9: TPCANHandle = $809;
      /// <summary>
      /// PCAN-LAN interface, channel 10
      /// </summary>
      const PCAN_LANBUS10: TPCANHandle = $80A;
      /// <summary>
      /// PCAN-LAN interface, channel 11
      /// </summary>
      const PCAN_LANBUS11: TPCANHandle = $80B;
      /// <summary>
      /// PCAN-LAN interface, channel 12
      /// </summary>
      const PCAN_LANBUS12: TPCANHandle = $80C;
      /// <summary>
      /// PCAN-LAN interface, channel 13
      /// </summary>
      const PCAN_LANBUS13: TPCANHandle = $80D;
      /// <summary>
      /// PCAN-LAN interface, channel 14
      /// </summary>
      const PCAN_LANBUS14: TPCANHandle = $80E;
      /// <summary>
      /// PCAN-LAN interface, channel 15
      /// </summary>
      const PCAN_LANBUS15: TPCANHandle = $80F;
      /// <summary>
      /// PCAN-LAN interface, channel 16
      /// </summary>
      const PCAN_LANBUS16: TPCANHandle = $810;

      /// <summary>
      /// Clock frequency in Herz (80000000, 60000000, 40000000, 30000000, 24000000, 20000000)
      /// </summary>
      const PCAN_BR_CLOCK: String = 'f_clock';
      /// <summary>
      /// Clock frequency in Megaherz (80, 60, 40, 30, 24, 20)
      /// </summary>
      const PCAN_BR_CLOCK_MHZ: String = 'f_clock_mhz';
      /// <summary>
      /// Clock prescaler for nominal time quantum
      /// </summary>
      const PCAN_BR_NOM_BRP: String = 'nom_brp';
      /// <summary>
      /// TSEG1 segment for nominal bit rate in time quanta
      /// </summary>
      const PCAN_BR_NOM_TSEG1: String = 'nom_tseg1';
      /// <summary>
      /// TSEG2 segment for nominal bit rate in time quanta
      /// </summary>
      const PCAN_BR_NOM_TSEG2: String = 'nom_tseg2';
      /// <summary>
      /// Synchronization Jump Width for nominal bit rate in time quanta
      /// </summary>
      const PCAN_BR_NOM_SJW: String = 'nom_sjw';
      /// <summary>
      /// Sample point for nominal bit rate
      /// </summary>
      const PCAN_BR_NOM_SAMPLE: String = 'nom_sam';
      /// <summary>
      /// Clock prescaler for highspeed data time quantum
      /// </summary>
      const PCAN_BR_DATA_BRP: String = 'data_brp';
      /// <summary>
      /// TSEG1 segment for fast data bit rate in time quanta
      /// </summary>
      const PCAN_BR_DATA_TSEG1: String = 'data_tseg1';
      /// <summary>
      /// TSEG2 segment for fast data bit rate in time quanta
      /// </summary>
      const PCAN_BR_DATA_TSEG2: String = 'data_tseg2';
      /// <summary>
      /// Synchronization Jump Width for highspeed data bit rate in time quanta
      /// </summary>
      const PCAN_BR_DATA_SJW: String = 'data_sjw';
      /// <summary>
      /// Secondary sample point delay for highspeed data bit rate in cyles
      /// </summary>
      const PCAN_BR_DATA_SAMPLE: String = 'data_ssp_offset';

      /// <summary>
      /// The PCAN parameter is not set (inactive)
      /// </summary>
      const PCAN_PARAMETER_OFF: Integer = 0;
      /// <summary>
      /// The PCAN parameter is set (active)
      /// </summary>
      const PCAN_PARAMETER_ON: Integer = 1;
      /// <summary>
      /// The PCAN filter is closed. No messages will be received
      /// </summary>
      const PCAN_FILTER_CLOSE: Integer = 0;
      /// <summary>
      /// The PCAN filter is fully opened. All messages will be received
      /// </summary>
      const PCAN_FILTER_OPEN: Integer = 1;
      /// <summary>
      /// The PCAN filter is custom configured. Only registered
      /// messages will be received
      /// </summary>
      const PCAN_FILTER_CUSTOM: Integer = 2;
      /// <summary>
      /// The PCAN-Channel handle is illegal, or its associated hardware is not available
      /// </summary>
      const PCAN_CHANNEL_UNAVAILABLE: Integer = 0;
      /// <summary>
      /// The PCAN-Channel handle is available to be connected (PnP Hardware: it means furthermore that the hardware is plugged-in)
      /// </summary>
      const PCAN_CHANNEL_AVAILABLE: Integer = 1;
      /// <summary>
      /// The PCAN-Channel handle is valid, and is already being used
      /// </summary>
      const PCAN_CHANNEL_OCCUPIED: Integer = 2;
      /// <summary>
      /// The PCAN-Channel handle is already being used by a PCAN-View application, but is available to connect
      /// </summary>
      const PCAN_CHANNEL_PCANVIEW: Integer = 3;

      /// <summary>
      /// Logs system exceptions / errors
      /// </summary>
      const LOG_FUNCTION_DEFAULT: Integer = $00;
      /// <summary>
      /// Logs the entries to the PCAN-Basic API functions
      /// </summary>
      const LOG_FUNCTION_ENTRY: Integer = $01;
      /// <summary>
      /// Logs the parameters passed to the PCAN-Basic API functions
      /// </summary>
      const LOG_FUNCTION_PARAMETERS: Integer = $02;
      /// <summary>
      /// Logs the exits from the PCAN-Basic API functions
      /// </summary>
      const LOG_FUNCTION_LEAVE: Integer = $04;
      /// <summary>
      /// Logs the CAN messages passed to the CAN_Write function
      /// </summary>
      const LOG_FUNCTION_WRITE: Integer = $08;
      /// <summary>
      /// Logs the CAN messages received within the CAN_Read function
      /// </summary>
      const LOG_FUNCTION_READ: Integer = $10;
      /// <summary>
      /// Logs all possible information within the PCAN-Basic API functions
      /// </summary>
      const LOG_FUNCTION_ALL: Integer = $FFFF;

      /// <summary>
      /// A single file is written until it size reaches PAN_TRACE_SIZE
      /// </summary>
      const TRACE_FILE_SINGLE: Integer = $00;
      /// <summary>
      /// Traced data is distributed in several files with size PAN_TRACE_SIZE
      /// </summary>
      const TRACE_FILE_SEGMENTED: Integer = $01;
      /// <summary>
      /// Includes the date into the name of the trace file
      /// </summary>
      const TRACE_FILE_DATE: Integer = $02;
      /// <summary>
      /// Includes the start time into the name of the trace file
      /// </summary>
      const TRACE_FILE_TIME: Integer = $04;
      /// <summary>
      /// Causes the overwriting of available traces (same name)
      /// </summary>
      const TRACE_FILE_OVERWRITE: Integer = $80;

      /// <summary>
      /// Device supports flexible data-rate (CAN-FD)
      /// </summary>
      const FEATURE_FD_CAPABLE: Integer = $01;
      /// <summary>
      /// Device supports a delay between sending frames (FPGA based USB devices)
      /// </summary>
      const FEATURE_DELAY_CAPABLE: Integer = $2;
      /// <summary>
      /// Device supports I/O functionality for electronic circuits (USB-Chip devices)
      /// </summary>
      const FEATURE_IO_CAPABLE: Integer = $4;

      /// <summary>
      /// The service is not running
      /// </summary>
      const SERVICE_STATUS_STOPPED: Integer = $01;
      /// <summary>
      /// The service is running
      /// </summary>
      const SERVICE_STATUS_RUNNING: Integer = $04;

      /// <summary>
      /// Lookup channel by Device type (see PCAN devices e.g. PCAN_USB)
      /// </summary>
      const LOOKUP_DEVICE_TYPE: String = 'devicetype';
      /// <summary>
      /// Lookup channel by device id
      /// </summary>
      const LOOKUP_DEVICE_ID: String = 'deviceid';
      /// <summary>
      /// Lookup channel by CAN controller 0-based index
      /// </summary>
      const LOOKUP_CONTROLLER_NUMBER: String = 'controllernumber';
      /// <summary>
      /// Lookup channel by IP address (LAN channels only)
      /// </summary>
      const LOOKUP_IP_ADDRESS: String = 'ipaddress';

      /// <summary>
      /// Maximum length of the name of a device: 32 characters + terminator
      /// </summary>
      const MAX_LENGTH_HARDWARE_NAME: Integer = 33;
      /// <summary>
      /// Maximum length of a version string: 255 characters + terminator
      /// </summary>
      const MAX_LENGTH_VERSION_STRING: Integer = 256;

    /// <summary>
    /// Initializes a PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
    /// <param name="HwType">Non-PnP: The type of hardware and operation mode</param>
    /// <param name="IOPort">Non-PnP: The I/O address for the parallel port</param>
    /// <param name="Interrupt">Non-PnP: Interrupt number of the parallel port</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Initialize(
        Channel: TPCANHandle;
        Btr0Btr1: TPCANBaudrate;
        HwType: TPCANType;
        IOPort: LongWord;
        Interrupt: Word
        ): TPCANStatus; overload;
            
    /// <summary>
    /// Initializes a PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Btr0Btr1">The speed for the communication (BTR0BTR1 code)</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Initialize(
        Channel: TPCANHandle;
        Btr0Btr1: TPCANBaudrate
        ): TPCANStatus; overload;
            
    /// <summary>
    /// Initializes a FD capable PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="BitrateFD">The speed for the communication (FD bit rate string)</param>
    /// <remarks> See PCAN_BR_* values
    /// Bit rate string must follow the following construction rules:
    /// * parameter and values must be separated by '='
    /// * Couples of Parameter/value must be separated by ','
    /// * Following Parameter must be filled out: f_clock, data_brp, data_sjw, data_tseg1, data_tseg2,
    ///   nom_brp, nom_sjw, nom_tseg1, nom_tseg2.
    /// * Following Parameters are optional (not used yet): data_ssp_offset, nom_sam</remarks>
    /// <example>f_clock=80000000,nom_brp=10,nom_tseg1=5,nom_tseg2=2,nom_sjw=1,data_brp=4,data_tseg1=7,data_tseg2=2,data_sjw=1</example>
    /// <returns>A TPCANStatus error code</returns>
    class function InitializeFD(
        Channel: TPCANHandle;
        BitrateFD: TPCANBitrateFD
        ): TPCANStatus;

    /// <summary>
    /// Uninitializes one or all PCAN Channels initialized by CAN_Initialize
    /// </summary>
    /// <remarks>Giving the TPCANHandle value "PCAN_NONEBUS",
    /// uninitialize all initialized channels</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Uninitialize(
        Channel: TPCANHandle
        ): TPCANStatus;
            
    /// <summary>
    /// Resets the receive and transmit queues of the PCAN Channel
    /// </summary>
    /// <remarks>A reset of the CAN controller is not performed</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Reset(
        Channel: TPCANHandle
        ): TPCANStatus;
            
    /// <summary>
    /// Gets the current status of a PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <returns>A TPCANStatus error code</returns>
    class function GetStatus(
        Channel: TPCANHandle
        ): TPCANStatus;
            
    /// <summary>
    /// Reads a CAN message from the receive queue of a PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsg structure buffer to store the CAN message</param>
    /// <param name="TimestampBuffer">A TPCANTimestamp structure buffer to get
    /// the reception time of the message</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Read(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsg;
        var TimestampBuffer: TPCANTimestamp
        ):TPCANStatus; overload;
            
    /// <summary>
    /// Reads a CAN message from the receive queue of a PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsg structure buffer to store the CAN message</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Read(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsg
        ): TPCANStatus; overload;

    /// <summary>
    /// Reads a CAN message from the receive queue of a FD capable PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a FD capable PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsgFD structure buffer to store the CAN message</param>
    /// <param name="TimestampBuffer">A TPCANTimestampFD buffer to get the
    /// reception time of the message</param>
    /// <returns>A TPCANStatus error code</returns>
    class function ReadFD(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsgFD;
        var TimestampBuffer: TPCANTimestampFD
        ):TPCANStatus; overload;
            
    /// <summary>
    /// Reads a CAN message from the receive queue of a FD capable PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a FD capable PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsgFD structure buffer to store the CAN message</param>
    /// <returns>A TPCANStatus error code</returns>
    class function ReadFD(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsgFD
        ):TPCANStatus; overload;
            
    /// <summary>
    ///  Transmits a CAN message
    /// </summary>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsg buffer with the message to be sent</param>
    /// <returns>A TPCANStatus error code</returns>
    class function Write(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsg
        ): TPCANStatus;
            
    /// <summary>
    /// Transmits a CAN message over a FD capable PCAN Channel
    /// </summary>
    /// <param name="Channel">The handle of a FD capable PCAN Channel</param>
    /// <param name="MessageBuffer">A TPCANMsgFD buffer with the message to be sent</param>
    /// <returns>A TPCANStatus error code</returns>
    class function WriteFD(
        Channel: TPCANHandle;
        var MessageBuffer: TPCANMsgFD
        ): TPCANStatus;

    /// <summary>
    /// Configures the reception filter
    /// </summary>
    /// <remarks>The message filter will be expanded with every call to
    /// this function. If it is desired to reset the filter, please use
    /// the 'SetValue' function</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="FromID">The lowest CAN ID to be received</param>
    /// <param name="ToID">The highest CAN ID to be received</param>
    /// <param name="Mode">Message type, Standard (11-bit identifier) or
    /// Extended (29-bit identifier)</param>
    /// <returns>A TPCANStatus error code</returns>
    class function FilterMessages(
        Channel: TPCANHandle;
        FromID: LongWord;
        ToID: LongWord;
        Mode: TPCANMode
        ): TPCANStatus;

    /// <summary>
    /// Retrieves a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to get</param>
    /// <param name="NumericBuffer">Buffer for the parameter value</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function GetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        NumericBuffer: PLongWord;
        BufferLength: LongWord
        ): TPCANStatus; overload;
            
            
    /// <summary>
    /// Retrieves a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to get</param>
    /// <param name="NumericBuffer">Buffer for the parameter value</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function GetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        NumericBuffer: PUInt64;
        BufferLength: LongWord
        ): TPCANStatus; overload;
            
            
    /// <summary>
    /// Retrieves a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to get</param>
    /// <param name="StringBuffer">Buffer for the parameter value</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function GetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        StringBuffer: PAnsiChar;
        BufferLength: LongWord
        ): TPCANStatus; overload;

    class function GetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        ChannelsBuffer: PTPCANChannelInformation;
        BufferLength: LongWord
        ): TPCANStatus; overload;
            
    /// <summary>
    /// Configures or sets a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to set</param>
    /// <param name="NumericBuffer">Buffer with the value to be set</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function SetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        NumericBuffer: PLongWord;
        BufferLength: LongWord
        ): TPCANStatus; overload;

    /// <summary>
    /// Configures or sets a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to set</param>
    /// <param name="NumericBuffer">Buffer with the value to be set</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function SetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        NumericBuffer: PUInt64;
        BufferLength: LongWord
        ): TPCANStatus; overload;
            
    /// <summary>
    /// Configures or sets a PCAN Channel value
    /// </summary>
    /// <remarks>Parameters can be present or not according with the kind
    /// of Hardware (PCAN Channel) being used. If a parameter is not available,
    /// a PCAN_ERROR_ILLPARAMTYPE error will be returned</remarks>
    /// <param name="Channel">The handle of a PCAN Channel</param>
    /// <param name="Parameter">The TPCANParameter parameter to set</param>
    /// <param name="StringBuffer">Buffer with the value to be set</param>
    /// <param name="BufferLength">Size in bytes of the buffer</param>
    /// <returns>A TPCANStatus error code</returns>
    class function SetValue(
        Channel: TPCANHandle;
        Parameter: TPCANParameter;
        StringBuffer: PAnsiChar;
        BufferLength: LongWord
        ): TPCANStatus; overload;

    /// <summary>
    /// Returns a descriptive text of a given TPCANStatus error
    /// code, in any desired language
    /// </summary>
    /// <remarks>The current languages available for translation are:
    /// Neutral (0x00), German (0x07), English (0x09), Spanish (0x0A),
    /// Italian (0x10) and French (0x0C)</remarks>
    /// <param name="Error">A TPCANStatus error code</param>
    /// <param name="Language">Indicates a 'Primary language ID'</param>
    /// <param name="StringBuffer">Buffer for the text (must be at least 256 in length)</param>
    /// <returns>A TPCANStatus error code</returns>
    class function GetErrorText(
        Error: TPCANStatus;
        Language: Word;
        StringBuffer: PAnsiChar
        ): TPCANStatus;

    /// <summary>
    /// Finds a PCAN-Basic channel that matches with the given parameters
    /// </summary>
    /// <param name="Parameters">A comma separated string contained pairs of
    /// parameter-name/value to be matched within a PCAN-Basic channel</param>
    /// <param name="FoundChannel">Buffer for returning the PCAN-basic channel,
    /// when found</param>
    /// <returns>A TPCANStatus error code</returns>
    class function LookUpChannel(
        Parameters: PAnsiChar;
        FoundChannel: PTPCANHandle
        ): TPCANStatus;
  end;

implementation

uses SysUtils;

const DLL_Name = 'PCANBASIC.DLL';

function CAN_Initialize(Channel: TPCANHandle; Btr0Btr1: TPCANBaudrate; HwType: TPCANType; IOPort: LongWord; Interrupt: Word): TPCANStatus; stdcall;
external DLL_Name;
function CAN_InitializeFD(Channel: TPCANHandle; BitrateFD: TPCANBitrateFD): TPCANStatus; stdcall;
external DLL_Name;
function CAN_Uninitialize(Channel: TPCANHandle): TPCANStatus; stdcall;
external DLL_Name;
function CAN_Reset(Channel: TPCANHandle): TPCANStatus; stdcall;
external DLL_Name;
function CAN_GetStatus(Channel: TPCANHandle): TPCANStatus; stdcall;
external DLL_Name;
function CAN_Read(Channel: TPCANHandle; var MessageBuffer: TPCANMsg; TimestampBuffer: PTPCANTimestamp):TPCANStatus; overload; stdcall;
external DLL_Name;
function CAN_ReadFD(Channel: TPCANHandle; var MessageBuffer: TPCANMsgFD; TimestampBuffer: PUInt64):TPCANStatus; overload; stdcall;
external DLL_Name;
function CAN_Write(Channel: TPCANHandle; var MessageBuffer: TPCANMsg): TPCANStatus; stdcall;
external DLL_Name;
function CAN_WriteFD(Channel: TPCANHandle; var MessageBuffer: TPCANMsgFD): TPCANStatus; stdcall;
external DLL_Name;
function CAN_FilterMessages(Channel: TPCANHandle; FromID: LongWord; ToID: LongWord; Mode: TPCANMode): TPCANStatus; stdcall;
external DLL_Name;
function CAN_GetValue(Channel: TPCANHandle; Parameter: TPCANParameter; Buffer: Pointer; BufferLength: LongWord): TPCANStatus; stdcall;
external DLL_Name;
function CAN_SetValue(Channel: TPCANHandle; Parameter: TPCANParameter; Buffer: Pointer; BufferLength: LongWord): TPCANStatus; stdcall;
external DLL_Name;
function CAN_GetErrorText(Error: TPCANStatus; Language: Word; StringBuffer: PAnsiChar): TPCANStatus; stdcall;
external DLL_Name;
function CAN_LookUpChannel(Parameters: PAnsiChar; FoundChannel: PTPCANHandle): TPCANStatus; stdcall;
external DLL_Name;

class function TPCANBasic.Initialize(Channel: TPCANHandle; Btr0Btr1: TPCANBaudrate; HwType: TPCANType; IOPort: LongWord; Interrupt: Word): TPCANStatus;
begin
  Result:= CAN_Initialize(Channel,Btr0Btr1,HwType,IOPort,Interrupt);
end;

class function TPCANBasic.Initialize(Channel: TPCANHandle; Btr0Btr1: TPCANBaudrate): TPCANStatus;
begin
  Result:= CAN_Initialize(Channel,Btr0Btr1,PCAN_TYPE_ISA, 0,0);
end;

class function TPCANBasic.InitializeFD(Channel: TPCANHandle; BitrateFD: TPCANBitrateFD): TPCANStatus;
begin
  Result:= CAN_InitializeFD(Channel,BitrateFD);
end;

class function TPCANBasic.Uninitialize(Channel: TPCANHandle): TPCANStatus;
begin
  Result:= CAN_Uninitialize(Channel);
end;

class function TPCANBasic.Reset(Channel: TPCANHandle): TPCANStatus;
begin
  Result:= CAN_Reset(Channel);
end;

class function TPCANBasic.GetStatus(Channel: TPCANHandle): TPCANStatus;
begin
  Result:= CAN_GetStatus(Channel);
end;

class function TPCANBasic.Read(Channel: TPCANHandle; var MessageBuffer: TPCANMsg; var TimestampBuffer: TPCANTimestamp):TPCANStatus;
begin
  Result:= CAN_Read(Channel, MessageBuffer, @TimestampBuffer);
end;

class function TPCANBasic.Read(Channel: TPCANHandle; var MessageBuffer: TPCANMsg):TPCANStatus;
begin
  Result:= CAN_Read(Channel, MessageBuffer, nil);
end;

class function TPCANBasic.ReadFD(Channel: TPCANHandle; var MessageBuffer: TPCANMsgFD; var TimestampBuffer: TPCANTimestampFD):TPCANStatus;
begin
  Result:= CAN_ReadFD(Channel, MessageBuffer, @TimestampBuffer);
end;

class function TPCANBasic.ReadFD(Channel: TPCANHandle; var MessageBuffer: TPCANMsgFD):TPCANStatus;
begin
  Result:= CAN_ReadFD(Channel, MessageBuffer, nil);
end;

class function TPCANBasic.Write(Channel: TPCANHandle; var MessageBuffer: TPCANMsg): TPCANStatus;
begin
  Result:= CAN_Write(Channel, MessageBuffer);
end;

class function TPCANBasic.WriteFD(Channel: TPCANHandle; var MessageBuffer: TPCANMsgFD): TPCANStatus;
begin
  Result:= CAN_WriteFD(Channel, MessageBuffer);
end;

class function TPCANBasic.FilterMessages(Channel: TPCANHandle; FromID: LongWord; ToID: LongWord; Mode: TPCANMode): TPCANStatus;
begin
  Result:= CAN_FilterMessages(Channel, FromID,ToID,Mode);
end;

class function TPCANBasic.GetValue(Channel: TPCANHandle; Parameter: TPCANParameter; NumericBuffer: PLongWord; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_GetValue(Channel, Parameter, NumericBuffer, BufferLength);
end;

class function TPCANBasic.GetValue(Channel: TPCANHandle; Parameter: TPCANParameter; NumericBuffer: PUInt64; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_GetValue(Channel, Parameter, NumericBuffer, BufferLength);
end;

class function TPCANBasic.GetValue(Channel: TPCANHandle; Parameter: TPCANParameter; StringBuffer: PAnsiChar; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_GetValue(Channel, Parameter, StringBuffer, BufferLength);
end;

class function TPCANBasic.GetValue(Channel: TPCANHandle; Parameter: TPCANParameter; ChannelsBuffer: PTPCANChannelInformation; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_GetValue(Channel, Parameter, ChannelsBuffer, BufferLength);
end;

class function TPCANBasic.SetValue(Channel: TPCANHandle; Parameter: TPCANParameter; NumericBuffer: PLongWord; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_SetValue(Channel, Parameter, NumericBuffer, BufferLength);
end;

class function TPCANBasic.SetValue(Channel: TPCANHandle; Parameter: TPCANParameter; NumericBuffer: PUInt64; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_SetValue(Channel, Parameter, NumericBuffer, BufferLength);
end;

class function TPCANBasic.SetValue(Channel: TPCANHandle; Parameter: TPCANParameter; StringBuffer: PAnsiChar; BufferLength: LongWord): TPCANStatus;
begin
  Result:= CAN_SetValue(Channel, Parameter, StringBuffer, BufferLength);
end;

class function TPCANBasic.GetErrorText(Error: TPCANStatus; Language: Word; StringBuffer: PAnsiChar): TPCANStatus;
begin
  Result:= CAN_GetErrorText(Error, Language, StringBuffer);
end;

class function TPCANBasic.LookUpChannel(Parameters: PAnsiChar; FoundChannel: PTPCANHandle): TPCANStatus;
begin
  Result:= CAN_LookUpChannel(Parameters, FoundChannel);
end;

end.