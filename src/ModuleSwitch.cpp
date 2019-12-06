/*******************************************************************************
 * Project:  InterfaceServer
 * @file     ModuleSwitch.cpp
 * @brief 
 * @author   Bwar
 * @date:    2016年7月6日
 * @note
 * Modify history:
 ******************************************************************************/
#include <fstream>
#include <set>
#include "ModuleSwitch.hpp"

namespace gate
{

ModuleSwitch::ModuleSwitch(const std::string& strModulePath)
    : neb::Module(strModulePath)
{
}

ModuleSwitch::~ModuleSwitch()
{
}

bool ModuleSwitch::Init()
{
    std::string strConfFile = GetWorkPath() + std::string("/conf/gateway_route.json");
    LOG4_DEBUG("CONF FILE = %s.", strConfFile.c_str());

    std::ifstream fin(strConfFile.c_str());
    if (fin.good())
    {
        std::stringstream ssContent;
        neb::CJsonObject oGatewayConf;
        ssContent << fin.rdbuf();
        fin.close();
        if (oGatewayConf.Parse(ssContent.str()))
        {
            std::string strKey;
            std::set<std::string> setKeys;
            while (oGatewayConf["url_path_route"].GetKey(strKey))
            {
                if (setKeys.find(strKey) != setKeys.end())
                {
                    LOG4_ERROR("the path \"%s\" is duplicate, skip it.", strKey.c_str());
                    continue;
                }
                auto pSharedSession = MakeSharedSession("gate::SessionRoute", strKey);
                if (pSharedSession != nullptr)
                {
                    std::shared_ptr<SessionRoute> pRoute
                        = std::dynamic_pointer_cast<SessionRoute>(pSharedSession);
                    pRoute->Init(oGatewayConf["url_path_route"][strKey]);
                    setKeys.insert(strKey);
                }
            }
            while (oGatewayConf["http_header_route"].GetKey(strKey))
            {
                if (setKeys.find(strKey) != setKeys.end())
                {
                    LOG4_ERROR("the x-Route value \"%s\" is duplicate, skip it.", strKey.c_str());
                    continue;
                }
                auto pSharedSession = MakeSharedSession("gate::SessionRoute", strKey);
                if (pSharedSession != nullptr)
                {
                    std::shared_ptr<SessionRoute> pRoute
                        = std::dynamic_pointer_cast<SessionRoute>(pSharedSession);
                    pRoute->Init(oGatewayConf["http_header_route"][strKey]);
                    setKeys.insert(strKey);
                }
            }
        }
        else
        {
            LOG4_ERROR("oGatewayConf pasre error");
            return(false);
        }
    }
    else
    {
        //配置信息流读取失败
        LOG4_ERROR("Open conf \"%s\" error!", strConfFile.c_str());
        return(false);
    }

    return(true);
}

bool ModuleSwitch::AnyMessage(
                std::shared_ptr<neb::SocketChannel> pChannel,
                const HttpMsg& oInHttpMsg)
{
    if (HTTP_OPTIONS == oInHttpMsg.method())
    {
        LOG4_TRACE("receive an OPTIONS");
        ResponseOptions(pChannel, oInHttpMsg);
        return(true);
    }
    auto pSharedSession = GetSession(oInHttpMsg.path());
    if (pSharedSession == nullptr)
    {
        auto header_iter = oInHttpMsg.headers().find("x-Route");
        if (header_iter != oInHttpMsg.headers().end())
        {
            pSharedSession = GetSession(header_iter->second);
        }
        if (pSharedSession == nullptr)
        {
            HttpMsg oOutHttpMsg;
            LOG4_ERROR("no path \"%s\" or x-Route header \"%s\" config in \"gateway_route.json\"!",
                    oInHttpMsg.path().c_str(), header_iter->second.c_str());
            oOutHttpMsg.set_type(HTTP_RESPONSE);
            oOutHttpMsg.set_status_code(404);
            oOutHttpMsg.set_http_major(oInHttpMsg.http_major());
            oOutHttpMsg.set_http_minor(oInHttpMsg.http_minor());
            SendTo(pChannel, oOutHttpMsg);
            return(false);
        }
    }

    std::string strService;
    std::shared_ptr<SessionRoute> pSharedRoute
        = std::dynamic_pointer_cast<SessionRoute>(pSharedSession);
    if (pSharedRoute->ApplyService(strService))
    {
        auto pStepSwitch = MakeSharedStep("gate::StepSwitch", pChannel,
                oInHttpMsg, strService, pSharedRoute);
        if ((pStepSwitch))
        {
            if (neb::CMD_STATUS_RUNNING == pStepSwitch->Emit())
            {
                LOG4_TRACE("pStepSwitch running");
                return(true);
            }
        }
    }
    else
    {
        Response(pChannel, oInHttpMsg, 10009, "Concurrent request exceed 5! response by gateway.");
        return(true);
    }
}

void ModuleSwitch::Response(std::shared_ptr<neb::SocketChannel> pChannel,
        const HttpMsg& oInHttpMsg, int iErrno, const std::string& strErrMsg)
{
    HttpMsg oHttpMsg;
    neb::CJsonObject oResponseData;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(oInHttpMsg.http_major());
    oHttpMsg.set_http_minor(oInHttpMsg.http_minor());
    oResponseData.Add("code", iErrno);
    oResponseData.Add("msg", strErrMsg);
    if (iErrno == 0)
    {
        oResponseData.Add("data", neb::CJsonObject("[]"));
    }
    oHttpMsg.set_body(oResponseData.ToFormattedString());
    SendTo(pChannel, oHttpMsg);
}

void ModuleSwitch::ResponseOptions(
        std::shared_ptr<neb::SocketChannel> pChannel,
        const HttpMsg& oInHttpMsg)
{
    LOG4_DEBUG("%s()", __FUNCTION__);
    HttpMsg oHttpMsg;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(oInHttpMsg.http_major());
    oHttpMsg.set_http_minor(oInHttpMsg.http_minor());
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Connection", "keep-alive"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Origin", "*"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Headers", "Origin, Content-Type, Cookie, Accept"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Methods", "GET, POST"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Credentials", "true"));
    SendTo(pChannel, oHttpMsg);
}

} /* namespace inter */
