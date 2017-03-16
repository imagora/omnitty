#pragma once
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <typeindex>
#include <unordered_map>


namespace omnitty {


struct IpV4 {
    union {
        uint32_t ip;
        uint8_t repr[4];
    };
};


struct MacAddr {
    uint8_t m_addr[6];
};


class OmniArgsGuard
{
public:
    OmniArgsGuard(std::vector<char *> &argv, uint32_t argc, uint32_t argsLength);

    ~OmniArgsGuard();

private:
    std::vector<char *> &m_argv;
};


typedef std::pair<IpV4, IpV4> IpV4Pair;


class OmniOptParser
{
    struct OmniOptInfo {
        std::type_index m_type = typeid(bool);
        union {
            void        *m_dataPtr;
            bool        *m_boolPtr;
            int32_t     *m_int32Ptr;
            int64_t     *m_int64Ptr;
            uint32_t    *m_uint32Ptr;
            uint64_t    *m_uint64Ptr;
            double      *m_doublePtr;
            std::string *m_stringPtr;
            IpV4        *m_ipV4Ptr;
            MacAddr     *m_macAddrPtr;
            IpV4Pair    *m_ipV4PairPtr;
        };
    };
public:
    OmniOptParser();

    template<typename OptType>
    bool AddLongOpt(const char *longOpt, OptType *store);

    bool ParseOpts(int argc, char *const argv[]);

    void Clear();

    void PrintUsage(const char *exec_file, std::ostream &sout) const;

private:
    bool FillArg(const char *opt_name, const OmniOptInfo &opt, const char *optArg);

    bool InsertLongOpt(OmniOptInfo opt, const char *longOpt);

    bool ParseBool(const OmniOptInfo &opt, const char *optArg);

    bool ParseInt32(const OmniOptInfo &opt, const char *optArg);

    bool ParseInt64(const OmniOptInfo &opt, const char *optArg);

    bool ParseUInt32(const OmniOptInfo &opt, const char *optArg);

    bool ParseUInt64(const OmniOptInfo &opt, const char *optArg);

    bool ParseDouble(const OmniOptInfo &opt, const char *optArg);

    bool ParseString(const OmniOptInfo &opt, const char *optArg);

    bool ParseIpV4(const OmniOptInfo &opt, const char *optArg);

    bool ParseMacAddr(const OmniOptInfo &opt, const char *optArg);

    bool ParseIpV4Pair(const OmniOptInfo &opt, const char *optArg);

private:
    typedef bool (OmniOptParser::*ParseFuncPtr)(const OmniOptInfo &opt, const char *optArg);

    std::map<std::type_index, std::string>          m_typeNames;
    std::map<std::type_index, ParseFuncPtr>         m_parseFuncs;
    std::unordered_map<const char *, OmniOptInfo>   m_longOpts;
};


template<typename OptType>
bool OmniOptParser::AddLongOpt(const char *longOpt, OptType *store)
{
    OmniOptInfo arg;
    arg.m_type = std::type_index(typeid(OptType));
    arg.m_dataPtr = store;
    return InsertLongOpt(arg, longOpt);
}


}
