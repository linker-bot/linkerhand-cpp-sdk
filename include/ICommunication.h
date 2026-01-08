#include <set>
#include <vector>
#include <string>
#include <iostream>

// namespace LinkerHandComm
// {
    class ICommunication
    {
    public:
        virtual ~ICommunication() = default;
        virtual void send(const std::vector<uint8_t>& data, uint32_t &id, const int &start_address = 0, const int &num = 0) = 0;
        virtual std::vector<uint8_t> recv(uint32_t& id, const int &start_address = 0, const int &num = 0) = 0;
    };
// }