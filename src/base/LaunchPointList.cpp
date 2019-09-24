// Copyright (c) 2019 LG Electronics, Inc.
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

#include "base/LaunchPointList.h"

#include <sys/time.h>
#include <boost/lexical_cast.hpp>

LaunchPointList::LaunchPointList()
{
}

LaunchPointList::~LaunchPointList()
{

}

void LaunchPointList::clear()
{
    m_list.clear();
}

void LaunchPointList::sort()
{
    m_list.sort(LaunchPoint::compareTitle);
}

LaunchPointPtr LaunchPointList::createBootmark(AppDescriptionPtr appDesc, const JValue& database)
{
    string launchPointId = generateLaunchPointId(LaunchPointType::LaunchPoint_BOOKMARK, appDesc->getAppId());
    LaunchPointPtr launchPoint = std::make_shared<LaunchPoint>(appDesc, launchPointId);
    launchPoint->setType(LaunchPointType::LaunchPoint_BOOKMARK);
    launchPoint->updateDatabase(database);
    return launchPoint;
}

LaunchPointPtr LaunchPointList::createDefault(AppDescriptionPtr appDesc)
{
    string launchPointId = generateLaunchPointId(LaunchPointType::LaunchPoint_DEFAULT, appDesc->getAppId());
    LaunchPointPtr launchPoint = std::make_shared<LaunchPoint>(appDesc, launchPointId);
    launchPoint->setType(LaunchPointType::LaunchPoint_DEFAULT);
    return launchPoint;
}

LaunchPointPtr LaunchPointList::add(LaunchPointPtr launchPoint)
{
    if (launchPoint == nullptr)
        return nullptr;
    launchPoint->syncDatabase();
    m_list.push_back(launchPoint);
    return launchPoint;
}

LaunchPointPtr LaunchPointList::getByAppId(const string& appId)
{
    if (appId.empty())
        return nullptr;

    string launchPointId = generateLaunchPointId(LaunchPointType::LaunchPoint_DEFAULT, appId);

    return getByLaunchPointId(launchPointId);
}

LaunchPointPtr LaunchPointList::getByLaunchPointId(const string& launchPointId)
{
    if (launchPointId.empty())
        return nullptr;

    for (auto it = m_list.begin(); it != m_list.end(); ++it) {
        if ((*it)->getLaunchPointId() == launchPointId)
            return *it;
    }
    return nullptr;
}

void LaunchPointList::remove(AppDescriptionPtr appDesc)
{
    for (auto it = m_list.begin(); it != m_list.end();) {
        if ((*it)->getAppDesc() == appDesc) {
            it = m_list.erase(it);
        } else {
            ++it;
        }
    }
}

void LaunchPointList::remove(LaunchPointPtr launchPoint)
{
    for (auto it = m_list.begin(); it != m_list.end(); ++it) {
        if ((*it) == launchPoint) {
            m_list.erase(it);
            return;
        }
    }
}

void LaunchPointList::removeByAppId(const string& appId)
{
    for (auto it = m_list.begin(); it != m_list.end();) {
        if ((*it)->getAppDesc()->getAppId() == appId) {
            // notifyLaunchPointChanged(*it);
            // DB8::getInstance().deleteLaunchPoint((*it)->getLaunchPointId());
            it = m_list.erase(it);
        } else {
            ++it;
        }
    }
}

void LaunchPointList::removeByLaunchPointId(const string& launchPointId)
{
    for (auto it = m_list.begin(); it != m_list.end(); ++it) {
        if ((*it)->getLaunchPointId() == launchPointId) {
            m_list.erase(it);
            return;
        }
    }
}

void LaunchPointList::toJson(JValue& json)
{
    if (!json.isArray()) {
        return;
    }

    for (auto it = m_list.begin(); it != m_list.end(); ++it) {
        if ((*it)->isVisible()) {
            JValue item;
            (*it)->toJson(item);
            json.append(item);
        }
    }
}

std::string LaunchPointList::generateLaunchPointId(LaunchPointType type, const string& appId)
{
    if (type == LaunchPointType::LaunchPoint_DEFAULT) {
        return appId + "_default";
    }

    while (true) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        double verifier = tv.tv_usec;

        std::string launchPointId = appId + "_" + boost::lexical_cast<std::string>(verifier);
        if (LaunchPointList::getInstance().getByLaunchPointId(launchPointId) == nullptr)
            return launchPointId;
    }

    return std::string("");
}
