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

#ifndef PRELAUNCHER_INTERFACE_H_
#define PRELAUNCHER_INTERFACE_H_

#include <boost/signals2.hpp>

#include "core/lifecycle/launching_item.h"

class PrelauncherInterface
{
public:
    PrelauncherInterface() {}
    virtual ~PrelauncherInterface() {}

    virtual void add_item(AppLaunchingItemPtr item) = 0;
    virtual void remove_item(const std::string& item_uid) = 0;
    virtual void input_bridged_return(AppLaunchingItemPtr item, const pbnjson::JValue& jmsg) = 0;
    virtual void cancel_all() = 0;

public:
    boost::signals2::signal<void (const std::string& uid)> signal_prelaunching_done;
};

#endif
