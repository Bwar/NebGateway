/*******************************************************************************
 * Project:  InterfaceServer
 * @file     StepSwitch.cpp
 * @brief 
 * @author   lbh
 * @date:    2016年7月6日
 * @note
 * Modify history:
 ******************************************************************************/

#include <util/http/http_parser.h>
#include "StepSwitch.hpp"

namespace gate
{

StepSwitch::StepSwitch(std::shared_ptr<neb::SocketChannel> pChannel,
                const HttpMsg& oInHttpMsg, const std::string& strTargetService,
                std::shared_ptr<SessionRoute> pRoute)
    : m_pChannel(pChannel), m_oReqHttpMsg(oInHttpMsg),
      m_strService(strTargetService), m_pRoute(pRoute)
{
}

StepSwitch::~StepSwitch()
{
}

neb::E_CMD_STATUS StepSwitch::Emit(int iErrno, const std::string& strErrMsg, void* data)
{
    LOG4_DEBUG("%s()", __FUNCTION__);
    m_oReqHttpMsg.set_url(m_pRoute->GetSchema() + "://" + m_strService + m_oReqHttpMsg.url());
    if (HttpRequest(m_oReqHttpMsg))
    {
        return(neb::CMD_STATUS_RUNNING);
    }
    else
    {
        m_pRoute->ServiceFailed(m_strService);
        return(neb::CMD_STATUS_FAULT);
    }
}

neb::E_CMD_STATUS StepSwitch::Callback(
        std::shared_ptr<neb::SocketChannel> pChannel,
        const HttpMsg& oHttpMsg,
        void* data)
{
    LOG4_TRACE("%s()", __FUNCTION__);
    m_pRoute->ServiceSucceed(m_strService);
    if (SendTo(m_pChannel, oHttpMsg))
    {
        return(neb::CMD_STATUS_COMPLETED);
    }
    else
    {
        return(neb::CMD_STATUS_FAULT);
    }
}

neb::E_CMD_STATUS StepSwitch::Timeout()
{
    LOG4_DEBUG("%s()", __FUNCTION__);
    neb::CJsonObject oResponseData;
    oResponseData.Add("code", 10010);
    oResponseData.Add("msg", "timeout");
    Response(oResponseData.ToFormattedString());
    m_pRoute->ServiceTimeout(m_strService);
    return(neb::CMD_STATUS_FAULT);
}

bool StepSwitch::Response(const std::string& strData)
{
    LOG4_TRACE("%s()", __FUNCTION__);
    HttpMsg oHttpMsg;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(m_oReqHttpMsg.http_major());
    oHttpMsg.set_http_minor(m_oReqHttpMsg.http_minor());
    oHttpMsg.set_body(strData);
    if (SendTo(m_pChannel, oHttpMsg))
    {
        return(true);
    }
    return(false);
}

} /* namespace im */
