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

#ifndef BUS_APPMGRSERVICE_H_
#define BUS_APPMGRSERVICE_H_

#include <boost/function.hpp>
#include <boost/signals2.hpp>
#include <bus/LaunchPointLunaAdapter.h>
#include <bus/LifeCycleLunaAdapter.h>
#include <bus/PackageLunaAdapter.h>
#include <bus/ServiceBase.h>
#include <map>
#include <memory>
#include <pbnjson.hpp>
#include <util/JUtil.h>
#include <util/Singleton.h>
#include <string>

typedef boost::function<void(LunaTaskPtr)> LunaApiHandler;

class AppMgrService: public ServiceBase,
                     public Singleton<AppMgrService> {
friend class Singleton<AppMgrService> ;
public:
    AppMgrService();
    virtual ~AppMgrService();

    virtual bool attach(GMainLoop* gml);
    virtual void detach();

    void registerApiHandler(const std::string& category, const std::string& method, const std::string& schema, LunaApiHandler handler);

    void onServiceReady();
    void setServiceStatus(bool status)
    {
        m_serviceReady = status;
    }
    bool isServiceReady() const
    {
        return m_serviceReady;
    }

    boost::signals2::signal<void()> signalOnServiceReady;

protected:
    virtual LSMethod* getMethods(std::string category) const;
    virtual void getCategories(std::vector<std::string>& categories) const;

private:
    void postAttach();
    static bool onApiCalled(LSHandle* lshandle, LSMessage* lsmsg, void* ctx);
    void handlePendingTask(std::vector<LunaTaskPtr>& tasks);

    LifeCycleLunaAdapter m_lifecycleLunaAdapter;
    PackageLunaAdapter m_packageLunaAdapter;
    LaunchPointLunaAdapter m_launchpointLunaAdapter;

    std::map<std::string, LunaApiHandler> m_apiHandlerMap;

    bool m_serviceReady;
};

#endif  // CORE_BUS_APPLICATION_SERVICE_H_
