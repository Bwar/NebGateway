/*******************************************************************************
 * Project:  NebulaInterface
 * @file     ModuleHello.cpp
 * @brief 
 * @author   Bwar
 * @date:    2019年1月27日
 * @note
 * Modify history:
 ******************************************************************************/
#include "SessionRoute.hpp"

#include <fstream>

namespace gate
{

SessionRoute::SessionRoute(const std::string& strSessionId)
    : neb::Session(strSessionId)
{
}

SessionRoute::~SessionRoute()
{
}

neb::E_CMD_STATUS SessionRoute::Timeout()
{
    for (auto iter = m_mapService.begin(); iter != m_mapService.end(); ++iter)
    {
        iter->second.Reset();
    }
    return(neb::CMD_STATUS_RUNNING);
}

bool SessionRoute::Init(neb::CJsonObject& oJsonConf)
{
    oJsonConf.Get("app", m_strApp);
    oJsonConf.Get("max_concurrent_per_service", m_uiMaxConcurrent);
    oJsonConf.Get("stat_interval", m_uiStatInterval);
    oJsonConf.Get("success_rate", m_fSuccessRate);
    oJsonConf.Get("timeout", m_fTimeout);
    oJsonConf.Get("schema", m_strSchema);
    for (int i = 0; i < oJsonConf["service"].GetArraySize(); ++i)
    {
        RouteStat oRoteStat;
        m_mapService.insert(std::make_pair(oJsonConf["service"](i), std::move(oRoteStat)));
    }
    m_iterService = m_mapService.begin();
    return(true);
}

bool SessionRoute::ApplyService(std::string& strTargetService)
{
    if (m_mapService.empty())
    {
        return(false);
    }
    auto cursor_iter = m_iterService;
    float success_rate = 0;
    if (m_iterService == m_mapService.end())
    {
        m_iterService = m_mapService.begin();
    }
    while (m_iterService != m_mapService.end())
    {
        if (m_iterService->second.e->uiRequestNum == 0)
        {
            success_rate = 1;
        }
        else
        {
            success_rate = m_iterService->second.e->uiSuccessNum
                    / m_iterService->second.e->uiRequestNum;
        }
        if (m_iterService->second.e->uiServingNum < m_uiMaxConcurrent
                && success_rate >= m_fSuccessRate)
        {
            strTargetService = m_iterService->first;
            m_iterService->second.e->uiRequestNum++;
            ++m_iterService;
            return(true);
        }
    }
    for (m_iterService = m_mapService.begin();
            (m_iterService != cursor_iter
                    && m_iterService != m_mapService.end()); ++m_iterService)
    {
        if (m_iterService->second.e->uiServingNum < m_uiMaxConcurrent
                && success_rate >= m_fSuccessRate)
        {
            strTargetService = m_iterService->first;
            m_iterService->second.e->uiRequestNum++;
            ++m_iterService;
            return(true);
        }
    }
    return(true);
}

bool SessionRoute::ServiceSucceed(const std::string& strTargetService)
{
    auto iter = m_mapService.find(strTargetService);
    if (iter == m_mapService.end())
    {
        return(false);
    }
    else
    {
        iter->second.e->uiSuccessNum++;
        iter->second.e->uiServingNum = (iter->second.e->uiServingNum > 0)
                ? (iter->second.e->uiServingNum - 1) : 0;
        return(true);
    }
}

bool SessionRoute::ServiceFailed(const std::string& strTargetService)
{
    auto iter = m_mapService.find(strTargetService);
    if (iter == m_mapService.end())
    {
        return(false);
    }
    else
    {
        iter->second.e->uiFailedNum++;
        iter->second.e->uiServingNum = (iter->second.e->uiServingNum > 0)
                ? (iter->second.e->uiServingNum - 1) : 0;
        return(true);
    }
}

bool SessionRoute::ServiceTimeout(const std::string& strTargetService)
{
    auto iter = m_mapService.find(strTargetService);
    if (iter == m_mapService.end())
    {
        return(false);
    }
    else
    {
        iter->second.e->uiTimeoutNum++;
        iter->second.e->uiServingNum = (iter->second.e->uiServingNum > 0)
                ? (iter->second.e->uiServingNum - 1) : 0;
        return(true);
    }
}

} /* namespace gate */
