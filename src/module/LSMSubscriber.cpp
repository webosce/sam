// Copyright (c) 2012-2018 LG Electronics, Inc.
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

#include <bus/AppMgrService.h>
#include <module/LSMSubscriber.h>
#include <module/ServiceObserver.h>
#include <pbnjson.hpp>
#include <util/JUtil.h>
#include <util/Logging.h>
#include <util/LSUtils.h>

static LSMSubscriber* g_this = NULL;

const std::string LSM_API_FOREGROUND_INFO = "getForegroundAppInfo";
const std::string LSM_API_RECENT_LIST = "getRecentsAppList";

LSMSubscriber::LSMSubscriber() :
        m_token_category_watcher(0), m_token_foreground_info(0), m_token_recent_list(0)
{
    g_this = this;
}

LSMSubscriber::~LSMSubscriber()
{

}

void LSMSubscriber::initialize()
{
    ServiceObserver::instance().add(WEBOS_SERVICE_LSM, std::bind(&LSMSubscriber::onServerStatusChanged, this, std::placeholders::_1));
}

void LSMSubscriber::onServerStatusChanged(bool connection)
{
    if (connection) {
        /*
         luna://com.webos.service.bus/signal/registerServiceCategory
         '{"serviceName": "com.webos.surfacemanager", "category": "/"}'
         */
        pbnjson::JValue payload = pbnjson::Object();
        payload.put("serviceName", "com.webos.surfacemanager");
        payload.put("category", "/");
        payload.put("subscribe", true);

        LSErrorSafe lserror;
        if (!LSCall(AppMgrService::instance().serviceHandle(),
                    "luna://com.webos.service.bus/signal/registerServiceCategory",
                    payload.stringify().c_str(),
                    categoryWatcher,
                    NULL,
                    &m_token_category_watcher,
                    &lserror)) {
            LOG_ERROR(MSGID_LSCALL_ERR, 3,
                      PMLOGKS("type", "lscall"),
                      PMLOGJSON("payload", payload.stringify().c_str()),
                      PMLOGKS("where", "signal_register_category"),
                      "err: %s", lserror.message);
        }
    } else {
        //cancel LSM registerServiceCategory subscription
        if (0 != m_token_category_watcher) {
            LSErrorSafe lserror;
            LSCallCancel(AppMgrService::instance().serviceHandle(), m_token_category_watcher, &lserror);
            if (LSErrorIsSet(&lserror))
                LSErrorPrint(&lserror, stderr);
            m_token_category_watcher = 0;
        }

        //init m_lsmGetForgrndAppInfoReq
        if (0 != m_token_foreground_info) {
            LSErrorSafe lserror;
            LSCallCancel(AppMgrService::instance().serviceHandle(), m_token_foreground_info, &lserror);
            if (LSErrorIsSet(&lserror))
                LSErrorPrint(&lserror, stderr);
            m_token_foreground_info = 0;
        }
    }
}

bool LSMSubscriber::categoryWatcher(LSHandle* handle, LSMessage* lsmsg, void* user_data)
{
    pbnjson::JValue jmsg = JUtil::parse(LSMessageGetPayload(lsmsg), std::string(""));
    if (jmsg.isNull())
        return false;

    LOG_DEBUG("%s , LSM CategoryPayLoad : %s", __PRETTY_FUNCTION__, jmsg.stringify().c_str());

    if (!jmsg.hasKey("/") || !jmsg["/"].isArray() || jmsg["/"].arraySize() < 1)
        return true;

    bool foreground_api_ready = false;
    bool recent_list_api_ready = false;

    int array_size = jmsg["/"].arraySize();
    for (int i = 0; i < array_size; ++i) {
        std::string lsm_api = jmsg["/"][i].asString();
        if (lsm_api.compare(LSM_API_FOREGROUND_INFO) == 0)
            foreground_api_ready = true;
        else if (lsm_api.compare(LSM_API_RECENT_LIST) == 0)
            recent_list_api_ready = true;
    }

    if (foreground_api_ready)
        g_this->subscribeForegroundInfo();

    if (recent_list_api_ready)
        g_this->subscribeRecentList();

    return true;
}

void LSMSubscriber::subscribeForegroundInfo()
{
    if (m_token_foreground_info != 0)
        return;

    LSErrorSafe lserror;
    if (!LSCall(AppMgrService::instance().serviceHandle(), "luna://com.webos.surfacemanager/getForegroundAppInfo", "{\"subscribe\":true}", onForegroundInfo, NULL, &m_token_foreground_info,
            &lserror)) {
        LOG_ERROR(MSGID_LSCALL_ERR, 3, PMLOGKS("type", "lscall"), PMLOGJSON("payload", "{\"subscribe\":true}"), PMLOGKS("where", "lsm_get_foreground_info"), "err: %s", lserror.message);
    }
}

bool LSMSubscriber::onForegroundInfo(LSHandle* handle, LSMessage* lsmsg, void* user_data)
{
    pbnjson::JValue jmsg = JUtil::parse(LSMessageGetPayload(lsmsg), std::string(""));
    if (jmsg.isNull())
        return false;

    g_this->signal_foreground_info(jmsg);

    return true;
}

void LSMSubscriber::subscribeRecentList()
{
    if (m_token_recent_list != 0)
        return;

    LSErrorSafe lserror;
    if (!LSCall(AppMgrService::instance().serviceHandle(), "luna://com.webos.surfacemanager/getRecentsAppList", "{\"subscribe\":true}", onRecentList, NULL, &m_token_recent_list, &lserror)) {
        LOG_ERROR(MSGID_LSCALL_ERR, 3, PMLOGKS("type", "lscall"), PMLOGJSON("payload", "{\"subscribe\":true}"), PMLOGKS("where", "lsm_get_recent_list"), "err: %s", lserror.message);
    }
}

bool LSMSubscriber::onRecentList(LSHandle* handle, LSMessage* lsmsg, void* user_data)
{
    pbnjson::JValue jmsg = JUtil::parse(LSMessageGetPayload(lsmsg), std::string(""));
    if (jmsg.isNull())
        return false;

    g_this->signal_recent_list(jmsg);

    return true;
}
