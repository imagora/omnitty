#include <vector>
#include <getopt.h>
#include "log.h"
#include "utils.h"
#include "opt_parser.h"


using namespace omnitty;


OmniArgsGuard::OmniArgsGuard(std::vector<char *> &argv, uint32_t argc, uint32_t argsLength)
  : m_argv(argv)
{
    for (uint32_t i = 0; i < argc; ++i) {
        char *buffer = new char[argsLength];
        memset(buffer, '\0', argsLength);
        m_argv.push_back(buffer);
    }
}

OmniArgsGuard::~OmniArgsGuard()
{
    for (auto *p : m_argv) {
        if (p != nullptr) {
            delete []p;
            p = nullptr;
        }
    }
}



OmniOptParser::OmniOptParser()
{
    m_parseFuncs = {
        {std::type_index(typeid(bool)),         &OmniOptParser::ParseBool},
        {std::type_index(typeid(int32_t)),      &OmniOptParser::ParseInt32},
        {std::type_index(typeid(int64_t)),      &OmniOptParser::ParseInt64},
        {std::type_index(typeid(uint32_t)),     &OmniOptParser::ParseUInt32},
        {std::type_index(typeid(uint64_t)),     &OmniOptParser::ParseUInt64},
        {std::type_index(typeid(double)),       &OmniOptParser::ParseDouble},
        {std::type_index(typeid(std::string)),  &OmniOptParser::ParseString},
        {std::type_index(typeid(IpV4)),         &OmniOptParser::ParseIpV4},
        {std::type_index(typeid(MacAddr)),      &OmniOptParser::ParseMacAddr},
        {std::type_index(typeid(IpV4Pair)),     &OmniOptParser::ParseIpV4Pair},
    };
    m_typeNames = {
        {std::type_index(typeid(int32_t)),      std::string("INTEGER32")},
        {std::type_index(typeid(int64_t)),      std::string("INTEGER64")},
        {std::type_index(typeid(uint32_t)),     std::string("UINTEGER32")},
        {std::type_index(typeid(uint64_t)),     std::string("UINTEGER64")},
        {std::type_index(typeid(double)),       std::string("DOUBLE")},
        {std::type_index(typeid(std::string)),  std::string("STRING")},
        {std::type_index(typeid(IpV4)),         std::string("ddd.ddd.ddd.ddd")},
        {std::type_index(typeid(MacAddr)),      std::string("xx:xx:xx:xx:xx:xx")},
        {std::type_index(typeid(IpV4Pair)),     std::string("ddd.ddd.ddd.ddd ddd.ddd.ddd.ddd")},
    };
}


bool OmniOptParser::ParseOpts(int argc, char* const argv[])
{
    std::vector<option> longOpts;
    longOpts.reserve(m_longOpts.size() + 1);

    for (const auto &longOpt : m_longOpts) {
        option arg = {longOpt.first, longOpt.second.m_type == std::type_index(typeid(bool)) ? no_argument : required_argument, 0, 0};
        longOpts.push_back(arg);
    }
    option end_indicator = {NULL, 0, NULL, 0};
    longOpts.push_back(end_indicator);

    optind = 1;
    optarg = NULL;
    int index, ret;
    while ((ret = getopt_long_only(argc, argv, "", &longOpts[0], &index)) == 0 || (ret > 0 && ret != '?')) {
        if (ret == 0) {
            const option &opt = longOpts[index];
            const OmniOptInfo &a = m_longOpts[opt.name];
            const char *arg = optarg;
            if (!arg && a.m_type != std::type_index(typeid(bool)) && optind < argc) {
                arg = argv[optind];
                ++optind;
            }

            if (!FillArg(opt.name, a, arg)) {
                optind = 0;
                return false;
            }
            m_parsedArgNames.insert(opt.name);
        }
    }

    index = optind;
    optind = 0;

    if (index < argc) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Unrecognized option argument: %s", argv[index]);
        return false;
    }

    if (ret != -1) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Unrecognized option argument: %s", argv[index - 1]);
        return false;
    }

    return true;
}


void OmniOptParser::Clear()
{
    m_longOpts.clear();
    m_parsedArgNames.clear();
}


void OmniOptParser::PrintUsage(const char *exec_file, std::ostream &sout) const
{
    sout << "Usage: \n  " << exec_file << " ";
    for (const auto &longOpt : m_longOpts) {
        sout << "--" << longOpt.first << " ";

        const OmniOptInfo &opt = longOpt.second;
        if (opt.m_type == std::type_index(typeid(bool))) continue;

        auto iter = m_typeNames.find(opt.m_type);
        if (iter == m_typeNames.end()) {
            LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Cannot get type name for %s", longOpt.first);
            continue;
        }
        sout << iter->second << " ";
    }

    sout << std::endl;
}


bool OmniOptParser::FillArg(const char *opt_name, const OmniOptInfo &opt, const char *optArg)
{
    LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "arg name %s", opt_name);
    if (opt.m_type == std::type_index(typeid(bool))) {
        return ParseBool(opt, optArg);
    }

    if (optArg == nullptr) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "No argument available for %s", opt_name);
        return false;
    }

    auto iter = m_parseFuncs.find(opt.m_type);
    if (iter == m_parseFuncs.end()) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Cannot parse type for %s", opt_name);
        return false;
    }
    return (this->*(iter->second))(opt, optArg);
}


bool OmniOptParser::InsertLongOpt(OmniOptInfo opt, const char *longOpt)
{
    if (!longOpt) {
        LOG4CPLUS_ERROR_STR(omnitty::LOGGER_NAME, "A full parameter should be supplied!");
        return false;
    }

    if (m_longOpts.count(longOpt) > 0) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "{%s} has been occupied yet!", longOpt);
        return false;
    }

    m_longOpts[longOpt] = opt;
    return true;
}


bool OmniOptParser::ParseBool(const OmniOptInfo &opt, const char *)
{
    *opt.m_boolPtr = true;
    return true;
}


bool OmniOptParser::ParseInt32(const OmniOptInfo &opt, const char *optArg)
{
    char *end_ptr = nullptr;
    long n = strtol(optArg, &end_ptr, 10);
    if (*end_ptr != '\0') {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Invalid integer argument: %s", optArg);
        return false;
    }
    *opt.m_int32Ptr = int32_t(n);
    return true;
}


bool OmniOptParser::ParseInt64(const OmniOptInfo &opt, const char *optArg)
{
    char *end_ptr = nullptr;
    long long n = strtoll(optArg, &end_ptr, 10);
    if (*end_ptr != '\0') {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Invalid integer argument: %s", optArg);
        return false;
    }
    *opt.m_int64Ptr = int64_t(n);
    return true;
}


bool OmniOptParser::ParseUInt32(const OmniOptInfo &opt, const char *optArg)
{
    char *end_ptr = nullptr;
    unsigned long n = strtoul(optArg, &end_ptr, 10);
    if (*end_ptr != '\0') {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Invalid integer argument: %s", optArg);
        return false;
    }
    *opt.m_uint32Ptr = uint32_t(n);
    return true;
}


bool OmniOptParser::ParseUInt64(const OmniOptInfo &opt, const char *optArg)
{
    char *end_ptr = nullptr;
    unsigned long long n = strtoull(optArg, &end_ptr, 10);
    if (*end_ptr != '\0') {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Invalid integer argument: %s", optArg);
        return false;
    }
    *opt.m_uint64Ptr = uint64_t(n);
    return true;
}


bool OmniOptParser::ParseDouble(const OmniOptInfo &opt, const char *optArg)
{
    char *end_ptr = nullptr;
    double n = strtod(optArg, &end_ptr);
    if (*end_ptr != '\0') {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Invalid double argument: %s", optArg);
        return false;
    }
    *opt.m_doublePtr = n;
    return true;
}


bool OmniOptParser::ParseString(const OmniOptInfo &opt, const char *optArg)
{
    *opt.m_stringPtr = optArg;
    return true;
}


bool OmniOptParser::ParseIpV4(const OmniOptInfo &opt, const char *optArg)
{
    uint8_t (&a)[4] = opt.m_ipV4Ptr->repr;
    if (sscanf(optArg, "%hhu.%hhu.%hhu.%hhu", &a[0], &a[1], &a[2], &a[3]) != 4) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Illegal IP Format: %s", optArg);
        return false;
    }
    return true;
}


bool OmniOptParser::ParseMacAddr(const OmniOptInfo &opt, const char *optArg)
{
    uint8_t (&b)[6] = opt.m_macAddrPtr->m_addr;
    if (sscanf(optArg, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Illegal ethernet physical address: %s", optArg);
        return false;
    }

    return true;
}

bool OmniOptParser::ParseIpV4Pair(const OmniOptParser::OmniOptInfo &opt, const char *optArg)
{
    std::vector<std::string> ips = SplitString(optArg, ',');
    if (ips.size() != 2) return false;

    uint8_t (&a)[4] = opt.m_ipV4PairPtr->first.repr;
    if (sscanf(ips[0].c_str(), "%hhu.%hhu.%hhu.%hhu", &a[0], &a[1], &a[2], &a[3]) != 4) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Illegal IP Format: %s", optArg);
        return false;
    }

    uint8_t (&b)[4] = opt.m_ipV4PairPtr->second.repr;
    if (sscanf(ips[1].c_str(), "%hhu.%hhu.%hhu.%hhu", &b[0], &b[1], &b[2], &b[3]) != 4) {
        LOG4CPLUS_ERROR_FMT(omnitty::LOGGER_NAME, "Illegal IP Format: %s", optArg);
        return false;
    }
    return true;
}
