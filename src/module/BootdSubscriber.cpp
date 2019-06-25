// Copyright (c) 2017-2018 LG Electronics, Inc.
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
#include <module/BootdSubscriber.h>
#include <module/ServiceObserver.h>
#include <pbnjson.hpp>
#include <util/JUtil.h>
#include <util/Logging.h>
#include <util/LSUtils.h>

BootdSubscriber::BootdSubscriber()
    : m_tokenBootStatus(0)
{
}

BootdSubscriber::~BootdSubscriber()
{
}

void BootdSubscriber::initialize()
{
    ServiceObserver::instance().add(WEBOS_SERVICE_BOOTMGR, std::bind(&BootdSubscriber::onServerStatusChanged, this, std::placeholders::_1));
}

boost::signals2::connection BootdSubscriber::subscribeBootStatus(boost::function<void(const pbnjson::JValue&)> func)
{
    return notify_boot_status.connect(func);
}

void BootdSubscriber::onServerStatusChanged(bool connection)
{
    if (connection) {
        requestBootStatus();
    } else {
        if (0 != m_tokenBootStatus) {
            (void) LSCallCancel(AppMgrService::instance().serviceHandle(), m_tokenBootStatus, NULL);
            m_tokenBootStatus = 0;
        }
    }
}

void BootdSubscriber::requestBootStatus()
{
    if (m_tokenBootStatus != 0)
        return;

    std::string method = std::string("luna://") + WEBOS_SERVICE_BOOTMGR + std::string("/getBootStatus");

    LSErrorSafe lserror;
    if (!LSCall(AppMgrService::instance().serviceHandle(),
                method.c_str(),
                "{\"subscribe\":true}",
                onBootStatusCallback,
                this,
                &m_tokenBootStatus,
                &lserror)) {
        LOG_ERROR(MSGID_LSCALL_ERR, 3,
                  PMLOGKS("type", "lscall"),
                  PMLOGJSON("payload", "{\"subscribe\":true}"),
                  PMLOGKS("where", __FUNCTION__),
                  "err: %s", lserror.message);
    }
}

bool BootdSubscriber::onBootStatusCallback(LSHandle* handle, LSMessage* lsmsg, void* user_data)
{
    BootdSubscriber* subscriber = static_cast<BootdSubscriber*>(user_data);
    if (!subscriber)
        return false;

    pbnjson::JValue jmsg = JUtil::parse(LSMessageGetPayload(lsmsg), std::string(""));
    if (jmsg.isNull())
        return false;

    if (jmsg.hasKey("bootStatus")) {
        // factory, normal, firstUse
        subscriber->m_bootStatusStr = jmsg["bootStatus"].asString();
    }

    subscriber->notify_boot_status(jmsg);
    return true;
}
