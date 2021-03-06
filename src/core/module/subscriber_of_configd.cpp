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

#include "core/module/subscriber_of_configd.h"

#include <pbnjson.hpp>

#include "core/base/jutil.h"
#include "core/base/logging.h"
#include "core/base/lsutils.h"
#include "core/bus/appmgr_service.h"
#include "core/module/service_observer.h"

ConfigdSubscriber::ConfigdSubscriber() : token_config_info_(0) {
}

ConfigdSubscriber::~ConfigdSubscriber() {
}

void ConfigdSubscriber::Init() {
  ServiceObserver::instance().Add(WEBOS_SERVICE_CONFIGD,
      std::bind(&ConfigdSubscriber::OnServerStatusChanged, this, std::placeholders::_1));
}

boost::signals2::connection ConfigdSubscriber::SubscribeConfigInfo(boost::function<void(pbnjson::JValue)> func) {
  return notify_config_info.connect(func);
}

void ConfigdSubscriber::OnServerStatusChanged(bool connection) {
  if(connection) {
    RequestConfigInfo();
  } else {
    if(0 != token_config_info_) {
      (void) LSCallCancel(AppMgrService::instance().ServiceHandle(), token_config_info_, NULL);
      token_config_info_ = 0;
    }
  }
}

void ConfigdSubscriber::RequestConfigInfo() {
  if(token_config_info_ != 0) return;
  if(config_keys_.empty()) return;

  pbnjson::JValue payload = pbnjson::Object();
  pbnjson::JValue configs = pbnjson::Array();

  payload.put("subscribe", true);
  for(auto& key: config_keys_) {
    configs.append(key);
  }
  payload.put("configNames", configs);

  std::string method = std::string("luna://") + WEBOS_SERVICE_CONFIGD + std::string("/getConfigs");

  LSErrorSafe lserror;
  if (!LSCall(AppMgrService::instance().ServiceHandle(),
      method.c_str(), JUtil::jsonToString(payload).c_str(),
      ConfigInfoCallback, this, &token_config_info_, &lserror)) {
          LOG_ERROR(MSGID_LSCALL_ERR, 3, PMLOGKS("type", "lscall"),
              PMLOGJSON("payload", JUtil::jsonToString(payload).c_str()),
              PMLOGKS("where", __FUNCTION__), "err: %s", lserror.message);
  }
}

bool ConfigdSubscriber::ConfigInfoCallback(LSHandle* handle, LSMessage* lsmsg, void* user_data) {
  ConfigdSubscriber* subscriber = static_cast<ConfigdSubscriber*>(user_data);
  if(!subscriber) return false;

  pbnjson::JValue jmsg = JUtil::parse(LSMessageGetPayload(lsmsg), std::string(""));
  if (jmsg.isNull()) return false;

  subscriber->notify_config_info(jmsg);
  return true;
}
