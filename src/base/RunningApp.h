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
#include "util/Logger.h"
#include "util/Time.h"

enum class LifeStatus : int8_t {
    LifeStatus_INVALID = -1,
    LifeStatus_STOP = 0,
    LifeStatus_PRELOADING,
    LifeStatus_LAUNCHING,
    LifeStatus_RELAUNCHING,
    LifeStatus_FOREGROUND,
    LifeStatus_BACKGROUND,
    LifeStatus_CLOSING,
    LifeStatus_PAUSING,
    LifeStatus_RUNNING, // internal event
};

class RunningApp {
friend class RunningAppList;
public:
    static const char* toString(LifeStatus status);

    RunningApp(LaunchPointPtr launchPoint);
    virtual ~RunningApp();

    // APIs
    bool launch(LunaTaskPtr lunaTask);
    bool close(LunaTaskPtr lunaTask);
    bool registerApp(LunaTaskPtr lunaTask);

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

    const std::string& getInstanceId() const
    {
        return m_instanceId;
    }
    void setInstanceId(const std::string& instanceId)
    {
        m_instanceId = instanceId;
    }

    LaunchPointPtr getLaunchPoint() const
    {
        return m_launchPoint;
    }

    const std::string& getDisplayId() const
    {
        return m_displayId;
    }
    void setDisplayId(const std::string& displayId)
    {
        m_displayId = displayId;
    }

    const std::string& getPid() const
    {
        return m_pid;
    }
    void setPid(const std::string& pid)
    {
        m_pid = pid;
    }

    const std::string& getWebprocid() const
    {
        return m_webprocid;
    }
    void setWebprocid(const std::string& webprocid)
    {
        if (webprocid == "-1" || webprocid == "0")
            return;
        m_webprocid = webprocid;
    }

    const std::string& getReason() const
    {
        return m_reason;
    }
    void setReason(const std::string& reason)
    {
        m_reason = reason;
    }

    int getInterfaceVersion()
    {
        return m_interfaceVersion;
    }

    bool isRegistered()
    {
        return m_isRegistered;
    }

    bool getPreloadMode() const
    {
        return m_isPreloadMode;
    }
    void setPreloadMode(bool mode)
    {
        m_isPreloadMode = mode;
    }

    string getPreload() const
    {
        string preload = "";
        JValueUtil::getValue(m_requestPayload, "preload", preload);
        return preload;
    }

    double getLastLaunchTime() const
    {
        return m_lastLaunchTime;
    }
    void saveLastLaunchTime()
    {
        m_lastLaunchTime = Time::getCurrentTime();
    }

    LifeStatus getLifeStatus() const
    {
        return m_lifeStatus;
    }
    void setLifeStatus(const LifeStatus& lifeStatus);

    bool isShowSplash()
    {
        bool noSplash = true;
        if (JValueUtil::getValue(m_requestPayload, "noSplash", noSplash))
            return noSplash;
        return this->getLaunchPoint()->getAppDesc()->isNoSplashOnLaunch();
    }

    bool isShowSpinner() const
    {
        bool spinner = true;
        if (JValueUtil::getValue(m_requestPayload, "spinner", spinner))
            return spinner;
        return this->getLaunchPoint()->getAppDesc()->isSpinnerOnLaunch();
    }

    void toJson(JValue& object)
    {
        object.put("instanceId", m_instanceId);
        object.put("id", m_launchPoint->getAppDesc()->getAppId());
        object.put("displayId", m_displayId);
        object.put("processid", m_pid);
        object.put("webprocessid", m_webprocid);
        object.put("defaultWindowType", m_launchPoint->getAppDesc()->getDefaultWindowType());
        object.put("appType", AppDescription::toString(m_launchPoint->getAppDesc()->getAppType()));
    }

private:
    static const string CLASS_NAME;

    RunningApp(const RunningApp&);
    RunningApp& operator=(const RunningApp&) const;

    static gboolean onKillingTimer(gpointer context);
    void startKillingTimer(guint timeout);
    void stopKillingTimer();

    LaunchPointPtr m_launchPoint;
    JValue m_requestPayload;

    string m_instanceId;
    string m_displayId;
    string m_pid;
    string m_webprocid;
    string m_reason;

    // for native app
    int m_interfaceVersion;
    bool m_isRegistered;
    guint m_killingTimer;

    bool m_isPreloadMode;
    double m_lastLaunchTime;

    LifeStatus m_lifeStatus;

};

typedef std::shared_ptr<RunningApp> RunningAppPtr;

#endif
