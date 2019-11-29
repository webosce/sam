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

#ifndef RUNNING_INFO_H
#define RUNNING_INFO_H

#include <map>
#include <memory>
#include <string>
#include <pbnjson.hpp>

#include "base/LaunchPoint.h"
#include "base/LunaTask.h"
#include "base/LunaTaskList.h"
#include "conf/SAMConf.h"
#include "util/Logger.h"
#include "util/Time.h"

//                  < RunningApp LIFECYCLES >
//
//   |--------------(PRELOADING)------------->PRELOADED
//   |                                            |
//   |                                      (RELAUNCHING)
//   |                                            |
// STOP--(SPLASHING)-->SPLASHED--(LAUNCHING)-->FORGROUND--(PAUSING)-->PAUSED
//   |                                            |
//   |                                      (RELAUNCHING)
//   |                                            |
//   |-----------(LAUNCHING:Hidden)---------->BACKGROUND

enum class LifeStatus : int8_t {
    LifeStatus_STOP = 0,
    LifeStatus_PRELOADING, // ==> PRELOADED
    LifeStatus_PRELOADED, // ==> RELAUNCHING
    LifeStatus_SPLASHING, // ==> LAUNCHING
    LifeStatus_SPLASHED, // ==> LAUNCHING
    LifeStatus_LAUNCHING, // ==> FOREGROUND
    LifeStatus_RELAUNCHING, // ==> FOREGROUND
    LifeStatus_FOREGROUND, // ==> BACKGROUND
    LifeStatus_BACKGROUND, // ==> PAUSING
    LifeStatus_PAUSING, // ==> PAUSED
    LifeStatus_PAUSED, // ==> CLOSING
    LifeStatus_CLOSING, // ==> STOP
};

class RunningApp {
friend class RunningAppList;
public:
    static const char* toString(LifeStatus status);
    static bool isTransition(LifeStatus status);

    RunningApp(LaunchPointPtr launchPoint);
    virtual ~RunningApp();

    // APIs
    void launch(LunaTaskPtr lunaTask);
    void close(LunaTaskPtr lunaTask);
    void registerApp(LunaTaskPtr lunaTask);

    bool sendEvent(pbnjson::JValue& payload);
    string getLaunchParams(LunaTaskPtr item);
    JValue getRelaunchParams(LunaTaskPtr item);

    string getAppId() const
    {
        return m_launchPoint->getAppDesc()->getAppId();
    }

    string getLaunchPointId() const
    {
        return m_launchPoint->getLaunchPointId();
    }

    const string& getInstanceId() const
    {
        return m_instanceId;
    }
    void setInstanceId(const string& instanceId)
    {
        if (!m_instanceId.empty()) {
            return;
        }
        m_instanceId = instanceId;
    }

    LaunchPointPtr getLaunchPoint() const
    {
        return m_launchPoint;
    }

    const string& getWindowId() const
    {
        return m_windowId;
    }
    void setWindowId(const string& windowId)
    {
        m_windowId = windowId;
    }

    const int getDisplayId() const
    {
        return m_displayId;
    }
    void setDisplayId(const int displayId)
    {
        m_displayId = displayId;
    }

    const string& getProcessId() const
    {
        return m_processId;
    }
    void setProcessId(const string& pid)
    {
        m_processId = pid;
    }

    void setWebprocid(const string& webprocid)
    {
        m_webprocid = webprocid;
    }

    int getInterfaceVersion()
    {
        return m_interfaceVersion;
    }

    bool isRegistered()
    {
        return m_isRegistered;
    }

    LifeStatus getLifeStatus() const
    {
        return m_lifeStatus;
    }
    bool setLifeStatus(LifeStatus lifeStatus);

    void loadRequestPayload(const JValue requestPayload)
    {
        if (!JValueUtil::getValue(requestPayload, "noSplash", m_noSplash)) {
            m_noSplash = this->getLaunchPoint()->getAppDesc()->isNoSplashOnLaunch();
        }
        if (!JValueUtil::getValue(requestPayload, "spinner", m_spinner)) {
            m_spinner = this->getLaunchPoint()->getAppDesc()->isSpinnerOnLaunch();
        }
        JValueUtil::getValue(requestPayload, "preload", m_preload);
        JValueUtil::getValue(requestPayload, "params", "launchedHidden", m_isLaunchedHidden);

        JValueUtil::getValue(requestPayload, "keepAlive", m_keepAlive);
        if (!m_keepAlive && SAMConf::getInstance().isKeepAliveApp(this->getAppId())) {
            m_keepAlive = true;
        }
    }

    string getPreload() const
    {
        // full, semi-full, partial, minimal
        return m_preload;
    }

    bool isKeepAlive() const
    {
        return m_keepAlive;
    }

    bool isShowSplash()
    {
        return !m_noSplash;
    }

    bool isShowSpinner() const
    {
        return m_spinner;
    }

    bool isLaunchedHidden() const
    {
        return m_isLaunchedHidden;
    }

    bool isFirstLaunch()
    {
        return m_isFirstLaunch;
    }
    void setFirstLaunch(bool isFirstLaunch)
    {
        m_isFirstLaunch = isFirstLaunch;
    }

    const string& getReason() const
    {
        return m_reason;
    }
    void setReason(const string& reason)
    {
        m_reason = reason;
    }

    void toJson(JValue& object, bool status)
    {
        object.put("instanceId", m_instanceId);
        object.put("launchPointid", m_launchPoint->getLaunchPointId());
        object.put("id", m_launchPoint->getAppId());

        if (m_displayId != -1)
            object.put("displayId", m_displayId);
        object.put("processId", m_processId);
        object.put("webprocessid", m_webprocid);

        if (status) {
            // getAppLifeStatus
            object.put("status", toString(m_lifeStatus));
            object.put("reason", m_reason);
            object.put("type", AppDescription::toString(m_launchPoint->getAppDesc()->getAppType()));

        } else {
            // runningList
            object.put("defaultWindowType", m_launchPoint->getAppDesc()->getDefaultWindowType());
            object.put("appType", AppDescription::toString(m_launchPoint->getAppDesc()->getAppType()));
        }
    }

private:
    static const string CLASS_NAME;
    static const int TIMEOUT_TRANSITION = 10000; // 10 seconds

    RunningApp(const RunningApp&);
    RunningApp& operator=(const RunningApp&) const;

    static gboolean onKillingTimer(gpointer context);
    void startKillingTimer(guint timeout);
    void stopKillingTimer();

    LaunchPointPtr m_launchPoint;

    string m_instanceId;
    string m_processId;
    string m_webprocid;
    string m_windowId;
    int m_displayId;

    // for native app
    int m_interfaceVersion;
    bool m_isRegistered;
    LS::Message m_registeredApp;

    LifeStatus m_lifeStatus;
    guint m_killingTimer;

    // initial parameter
    string m_preload;
    bool m_keepAlive;
    bool m_noSplash;
    bool m_spinner;
    bool m_isLaunchedHidden;
    bool m_isFirstLaunch;

    string m_reason;

};

typedef shared_ptr<RunningApp> RunningAppPtr;

#endif