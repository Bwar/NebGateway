/*******************************************************************************
 * Project:  NebGateway
 * @file     StepSwitch.hpp
 * @brief
 * @author   Bwar
 * @date:    2016年7月6日
 * @note
 * Modify history:
 ******************************************************************************/
#ifndef SRC_STEPSWITCH_HPP_
#define SRC_STEPSWITCH_HPP_

#include <actor/step/HttpStep.hpp>
#include <util/json/CJsonObject.hpp>
#include "SessionRoute.hpp"

namespace gate
{

class StepSwitch: public neb::HttpStep, public neb::DynamicCreator<StepSwitch,
    std::shared_ptr<neb::SocketChannel>, HttpMsg, std::string, std::shared_ptr<SessionRoute>>
{
public:
    StepSwitch(std::shared_ptr<neb::SocketChannel> pChannel, const HttpMsg& oInHttpMsg,
            const std::string& strTargetService, std::shared_ptr<SessionRoute> pRoute);
    virtual ~StepSwitch();

    virtual neb::E_CMD_STATUS Emit(int iErrno, const std::string& strErrMsg = "",  void* data = NULL);

    virtual neb::E_CMD_STATUS Callback(
                    std::shared_ptr<neb::SocketChannel> pUpstreamChannel,
                    const HttpMsg& oHttpMsg,
                    void* data = NULL);

    virtual neb::E_CMD_STATUS Timeout();

protected:
    bool Response(const std::string& strData);

private:
    std::shared_ptr<neb::SocketChannel> m_pChannel;
    HttpMsg m_oReqHttpMsg;
    std::string m_strService;
    std::shared_ptr<SessionRoute> m_pRoute;
};

} /* namespace gate */

#endif /* SRC_STEPSWITCH_HPP_ */
