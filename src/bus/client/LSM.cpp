// Copyright (c) 2012-2019 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <bus/client/LSM.h>
#include <pbnjson.hpp>
#include "base/AppDescription.h"
#include "base/LaunchPointList.h"
#include "base/LaunchPoint.h"
#include "base/RunningApp.h"
#include "base/RunningAppList.h"
#include "manager/LifecycleManager.h"
#include "bus/service/ApplicationManager.h"
#include "util/JValueUtil.h"

const string LSM::NAME = "com.webos.surfacemanager";
const string LSM::NAME_GET_FOREGROUND_APP_INFO = "getForegroundAppInfo";
const string LSM::NAME_GET_RECENTS_APP_LIST = "getRecentsAppList";

LSM::LSM()
    : AbsLunaClient(NAME)
{
    setClassName("LSM");
}

LSM::~LSM()
{

}

void LSM::onInitialze()
{

}

void LSM::onServerStatusChanged(bool isConnected)
{
    if (isConnected) {
        pbnjson::JValue requestPayload = pbnjson::Object();
        requestPayload.put("serviceName", "com.webos.surfacemanager");
        requestPayload.put("category", "/");
        requestPayload.put("subscribe", true);

        m_registerServiceCategoryCall = ApplicationManager::getInstance().callMultiReply(
            "luna://com.webos.service.bus/signal/registerServiceCategory",
            requestPayload.stringify().c_str(),
            onServiceCategoryChanged,
            nullptr
        );
    } else {
        m_registerServiceCategoryCall.cancel();
        m_getRecentsAppListCall.cancel();
        m_getForegroundAppInfoCall.cancel();
    }
}

bool LSM::onServiceCategoryChanged(LSHandle* sh, LSMessage* message, void* context)
{
    pbnjson::JValue responsePayload = JDomParser::fromString(LSMessageGetPayload(message));
    if (responsePayload.isNull())
        return false;

    Logger::debug(LSM::getInstance().getClassName(), __FUNCTION__, "lscall", responsePayload.stringify());
    if (!responsePayload.hasKey("/") || !responsePayload["/"].isArray() || responsePayload["/"].arraySize() < 1)
        return true;

    int arraySize = responsePayload["/"].arraySize();
    for (int i = 0; i < arraySize; ++i) {
        std::string method = responsePayload["/"][i].asString();
        if (method.compare(NAME_GET_FOREGROUND_APP_INFO) == 0) {
            LSM::getInstance().subscribeGetForegroundAppInfo();
        } else if (method.compare(NAME_GET_RECENTS_APP_LIST) == 0) {
            LSM::getInstance().subscribeGetRecentsAppList();
        }
    }
    return true;
}

void LSM::subscribeGetForegroundAppInfo()
{
    static std::string method = std::string("luna://") + getName() + std::string("/getForegroundAppInfo");

    if (m_getForegroundAppInfoCall.isActive())
        return;

    m_getForegroundAppInfoCall = ApplicationManager::getInstance().callMultiReply(
        method.c_str(),
        AbsLunaClient::getSubscriptionPayload().stringify().c_str(),
        onGetForegroundAppInfo,
        nullptr
    );
}

bool LSM::isFullscreenWindowType(const pbnjson::JValue& foregroundInfo)
{
    bool windowGroup = foregroundInfo["windowGroup"].asBool();
    bool windowGroupOwner = (windowGroup == false ? true : foregroundInfo["windowGroupOwner"].asBool());
    std::string windowType = foregroundInfo["windowType"].asString();

    if (!windowGroupOwner)
        return false;

    return SettingsImpl::getInstance().isFullscreenWindowTypes(windowType);
}

bool LSM::onGetForegroundAppInfo(LSHandle* sh, LSMessage* message, void* context)
{
    pbnjson::JValue subscriptionPayload = JDomParser::fromString(LSMessageGetPayload(message));
    if (subscriptionPayload.isNull())
        return false;

    pbnjson::JValue rawForegroundAppInfo;
    if (!JValueUtil::getValue(subscriptionPayload, "foregroundAppInfo", rawForegroundAppInfo)) {
        Logger::error(getInstance().getClassName(), __FUNCTION__, "invalid message from LSM");
        return true;
    }
    Logger::info(getInstance().getClassName(), __FUNCTION__, "ForegroundApps", "", rawForegroundAppInfo.stringify());

    std::string oldForegroundAppId = RunningAppList::getInstance().getForegroundAppId();
    std::string newForegroundAppId = "";
    std::vector<std::string> oldForegroundApps = RunningAppList::getInstance().getForegroundAppIds();
    std::vector<std::string> newForegroundApps;
    pbnjson::JValue oldForegroundInfo = RunningAppList::getInstance().getForegroundInfo();
    pbnjson::JValue newForegroundInfo = pbnjson::Array();

    for (int i = 0; i < rawForegroundAppInfo.arraySize(); ++i) {
        std::string appId;
        if (!JValueUtil::getValue(rawForegroundAppInfo[i], "appId", appId) || appId.empty()) {
            continue;
        }

        RunningAppPtr runningApp = RunningAppList::getInstance().getByAppId(appId, true);

        newForegroundInfo.append(rawForegroundAppInfo[i].duplicate());
        newForegroundApps.push_back(appId);

        if (getInstance().isFullscreenWindowType(rawForegroundAppInfo[i])) {
            newForegroundAppId = appId;
        }
    }

    // update foreground info into app info manager
    RunningAppList::getInstance().setForegroundApp(newForegroundAppId);
    RunningAppList::getInstance().setForegroundAppIds(newForegroundApps);
    RunningAppList::getInstance().setForegroundInfo(newForegroundInfo);

    Logger::info(getInstance().getClassName(), __FUNCTION__, Logger::format("newForegroundAppId(%s) oldForegroundAppId(%s)", newForegroundAppId.c_str(), oldForegroundAppId.c_str()));

    // set background
    for (auto& oldAppId : oldForegroundApps) {
        bool found = false;
        for (auto& newAppId : newForegroundApps) {
            if ((oldAppId) == (newAppId)) {
                found = true;
                break;
            }
        }

        if (found == false) {
            RunningAppPtr runningApp = RunningAppList::getInstance().getByAppId(oldAppId);
            switch (runningApp->getLifeStatus()) {
            case LifeStatus::FOREGROUND:
            case LifeStatus::PAUSING:
                LifecycleManager::getInstance().setAppLifeStatus(oldAppId, "", LifeStatus::BACKGROUND);
                break;

            default:
                break;
            }
        }
    }

    // set foreground
    for (auto& newAppId : newForegroundApps) {
        LifecycleManager::getInstance().setAppLifeStatus(newAppId, "", LifeStatus::FOREGROUND);

        if (!RunningAppList::getInstance().isRunning(newAppId)) {
            Logger::info(getInstance().getClassName(), __FUNCTION__, newAppId, "no running info, but received foreground info");
        }
    }

    // this is TV specific scenario related to TV POWER (instant booting)
    // improve this tv dependent structure later
    if (subscriptionPayload.hasKey("reason") && subscriptionPayload["reason"].asString() == "forceMinimize") {
        Logger::info(getInstance().getClassName(), __FUNCTION__, "no trigger last input handler");
    }

    // signal foreground app changed
    if (oldForegroundAppId != newForegroundAppId) {
        ApplicationManager::getInstance().postGetForegroundAppIfo(newForegroundAppId);
    }

    // reply subscription foreground with extraInfo
    if (oldForegroundInfo != newForegroundInfo) {
        ApplicationManager::getInstance().postGetForegroundAppIfoExtra(newForegroundInfo);
    }

    return true;
}

void LSM::subscribeGetRecentsAppList()
{
    static std::string method = std::string("luna://") + getName() + std::string("/getRecentsAppList");

    if (m_getRecentsAppListCall.isActive())
        return;

    m_getRecentsAppListCall = ApplicationManager::getInstance().callMultiReply(
        method.c_str(),
        AbsLunaClient::getSubscriptionPayload().stringify().c_str(),
        onGetRecentsAppList,
        nullptr
    );
}

bool LSM::onGetRecentsAppList(LSHandle* sh, LSMessage* message, void* context)
{
    pbnjson::JValue responsePayload = JDomParser::fromString(LSMessageGetPayload(message));
    if (responsePayload.isNull())
        return false;

    LSM::getInstance().EventRecentsAppListChanged(responsePayload);
    return true;
}
