#ifndef CAN_BUS_FACTORY_H
#define CAN_BUS_FACTORY_H

#include "ICanBus.h"
#include "CanBus.h"
#include "PCANBus.h"
#if USE_ETHERCAT
#include "EtherCAT.h"
#endif

namespace Communication
{
    class CanBusFactory
    {
    public:
        static std::unique_ptr<ICanBus> createCanBus(
        	uint32_t handId,
            const std::string& interfaceOrChannel,
            int bitrate,
            const LINKER_HAND linkerHand
        )
        {
            #ifdef _WIN32
                TPCANHandle channel = PCAN_USBBUS1;
                TPCANBaudrate baudrate = PCAN_BAUD_1M;
                if (interfaceOrChannel == "can1") channel = PCAN_USBBUS2;
                return std::make_unique<PCANBus>(channel, baudrate, linkerHand);
            #else
            	if (interfaceOrChannel == "can0" || interfaceOrChannel == "can1") {
            		return std::make_unique<CanBus>(interfaceOrChannel, bitrate, linkerHand);
            	} else if (interfaceOrChannel == "ethercat") {
                    #if USE_ETHERCAT
            		return std::make_unique<EtherCAT>(handId);
					#else
            		throw std::runtime_error("EtherCAT support is disabled (USE_ETHERCAT=0)");
					#endif
                }
            #endif
        }
    };
}

#endif // CAN_BUS_FACTORY_H
