/*******************************************************************************
 * Project:  NebGateway
 * @file     SessionRoute.hpp
 * @brief    Http请求示例
 * @author   Bwar
 * @date:    2019年1月27日
 * @note     
 * Modify history:
 ******************************************************************************/
#ifndef SESSIONROUTE_HPP_
#define SESSIONROUTE_HPP_

#include <string>
#include <unordered_map>
#include <actor/session/Session.hpp>

namespace gate
{

struct RouteStat
{
    struct StatElement
    {
        uint32 uiRequestNum = 0;
        uint32 uiSuccessNum = 0;
        uint32 uiFailedNum  = 0;
        uint32 uiTimeoutNum = 0;
        uint32 uiServingNum = 0;
    };

    StatElement* e;

    RouteStat()
    {
        e = new StatElement();
    }
    RouteStat(RouteStat&& stat)
    {
        this->e = stat.e;
        stat.e = nullptr;
    }
    RouteStat(const RouteStat& stat) = delete;
    ~RouteStat()
    {
        if (e != nullptr)
        {
            delete e;
            e = nullptr;
        }
    }
    RouteStat& operator=(RouteStat&& stat)
    {
        this->e = stat.e;
        stat.e = nullptr;
        return(*this);
    }
    RouteStat& operator=(const RouteStat& stat) = delete;
    void Reset()
    {
        e->uiRequestNum = 0;
        e->uiSuccessNum = 0;
        e->uiFailedNum  = 0;
        e->uiTimeoutNum = 0;
        e->uiServingNum = 0;
    }
};

class SessionRoute: public neb::Session,
    public neb::DynamicCreator<SessionRoute, std::string>
{
public:
    SessionRoute(const std::string& strSessionId);
    virtual ~SessionRoute();

    virtual neb::E_CMD_STATUS Timeout();

    bool Init(neb::CJsonObject& oJsonConf);

    int GetMaxConcurrent() const
    {
        return(m_uiMaxConcurrent);
    }

    float GetTimeout() const
    {
        return(m_fTimeout);
    }

    const std::string& GetApp() const
    {
        return(m_strApp);
    }

    const std::string& GetSchema() const
    {
        return(m_strSchema);
    }

    bool ApplyService(std::string& strTargetService);

    bool ServiceSucceed(const std::string& strTargetService);

    bool ServiceFailed(const std::string& strTargetService);

    bool ServiceTimeout(const std::string& strTargetService);

private:
    uint32 m_uiMaxConcurrent = 100;
    uint32 m_uiStatInterval = 60;
    float m_fSuccessRate = 0.99;
    float m_fTimeout = 1.0;
    std::string m_strApp;
    std::string m_strSchema;
    std::unordered_map<std::string, RouteStat> m_mapService;
    std::unordered_map<std::string, RouteStat>::iterator m_iterService;
};

} /* namespace gate */

#endif /* SESSIONROUTE_HPP_ */
