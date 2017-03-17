#pragma once
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>


namespace omnitty {


static std::vector<std::string> SplitString(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


static void StripString(std::string &s)
{
    if (s.empty()) return;

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    s.erase(s.find_last_not_of('\0') + 1);
}


static std::string Int2Ip(uint32_t ip)
{
    struct in_addr ipAddr;
    ipAddr.s_addr = ip;
    return inet_ntoa(ipAddr);
}


inline uint32_t Ip2Int(const std::string& ip)
{
    struct sockaddr_in ipAddr;
    inet_aton(ip.c_str(), &ipAddr.sin_addr);
    return ipAddr.sin_addr.s_addr;
}


}

