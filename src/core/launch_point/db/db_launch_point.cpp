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

#include "core/launch_point/db/db_launch_point.h"

#include "core/base/jutil.h"
#include "core/base/logging.h"
#include "core/base/lsutils.h"

DBLaunchPoint::DBLaunchPoint() {
  db_name_ = "com.webos.applicationManager.launchPoints:1";

  db_permissions_ = pbnjson::Object();
  db_kind_ = pbnjson::Object();
}

DBLaunchPoint::~DBLaunchPoint() {
}

void DBLaunchPoint::Init() {
}

bool DBLaunchPoint::InsertData(const pbnjson::JValue& json) {
  std::string db8_api = "luna://com.webos.service.db/put";
  std::string db8_qry;
  pbnjson::JValue json_db_qry = pbnjson::Object();

  if (json.hasKey("appDescription"))  json_db_qry.put("appDescription", json["appDescription"]);
  if (json.hasKey("bgColor"))         json_db_qry.put("bgColor", json["bgColor"]);
  if (json.hasKey("bgImage"))         json_db_qry.put("bgImage", json["bgImage"]);
  if (json.hasKey("bgImages"))        json_db_qry.put("bgImages", json["bgImages"]);
  if (json.hasKey("icon"))            json_db_qry.put("icon", json["icon"]);
  if (json.hasKey("iconColor"))       json_db_qry.put("iconColor", json["iconColor"]);
  if (json.hasKey("imageForRecents")) json_db_qry.put("imageForRecents", json["imageForRecents"]);
  if (json.hasKey("largeIcon"))       json_db_qry.put("largeIcon", json["largeIcon"]);
  if (json.hasKey("unmovable"))       json_db_qry.put("unmovable", json["unmovable"]);
  if (json.hasKey("params"))          json_db_qry.put("params", json["params"]);
  if (json.hasKey("tileSize"))        json_db_qry.put("tileSize", json["tileSize"]);
  if (json.hasKey("title"))           json_db_qry.put("title", json["title"]);
  if (json.hasKey("userData"))        json_db_qry.put("userData", json["userData"]);
  if (json.hasKey("miniicon"))        json_db_qry.put("miniicon", json["miniicon"]);

  if (json_db_qry.isNull()) return false;

  json_db_qry.put("_kind", db_name_);
  json_db_qry.put("id", json["id"]);
  json_db_qry.put("lptype", json["lptype"]);
  json_db_qry.put("launchPointId", json["launchPointId"]);

  db8_qry = "{\"objects\":[" + JUtil::jsonToString(json_db_qry) + "]}";

  return Db8Query(db8_api, db8_qry);
}

bool DBLaunchPoint::UpdateData(const pbnjson::JValue& json) {
  std::string db8_api = "luna://com.webos.service.db/merge";
  std::string db8_qry;
  pbnjson::JValue json_db_qry = pbnjson::Object();
  pbnjson::JValue json_where  = pbnjson::Object();

  json_db_qry.put("props",pbnjson::Object());

  if (json.hasKey("appDescription"))  json_db_qry["props"].put("appDescription", json["appDescription"]);
  if (json.hasKey("bgColor"))         json_db_qry["props"].put("bgColor", json["bgColor"]);
  if (json.hasKey("bgImage"))         json_db_qry["props"].put("bgImage", json["bgImage"]);
  if (json.hasKey("bgImages"))        json_db_qry["props"].put("bgImages", json["bgImages"]);
  if (json.hasKey("icon"))            json_db_qry["props"].put("icon", json["icon"]);
  if (json.hasKey("iconColor"))       json_db_qry["props"].put("iconColor", json["iconColor"]);
  if (json.hasKey("imageForRecents")) json_db_qry["props"].put("imageForRecents", json["imageForRecents"]);
  if (json.hasKey("largeIcon"))       json_db_qry["props"].put("largeIcon", json["largeIcon"]);
  if (json.hasKey("unmovable"))       json_db_qry["props"].put("unmovable", json["unmovable"]);
  if (json.hasKey("params"))          json_db_qry["props"].put("params", json["params"]);
  if (json.hasKey("tileSize"))        json_db_qry["props"].put("tileSize", json["tileSize"]);
  if (json.hasKey("title"))           json_db_qry["props"].put("title", json["title"]);
  if (json.hasKey("userData"))        json_db_qry["props"].put("userData", json["userData"]);
  if (json.hasKey("miniicon"))        json_db_qry["props"].put("miniicon", json["miniicon"]);

  if (json_db_qry["props"].isNull()) return false;

  json_db_qry.put("query", pbnjson::Object());
  json_db_qry["query"].put("from", db_name_);
  json_db_qry["query"].put("where", pbnjson::Array());

  json_where.put("prop", "launchPointId");
  json_where.put("op", "=");
  json_where.put("val", json["launchPointId"]);
  json_db_qry["query"]["where"].append(json_where);

  db8_qry = JUtil::jsonToString(json_db_qry);

  return Db8Query(db8_api, db8_qry);
}

bool DBLaunchPoint::DeleteData(const pbnjson::JValue& json) {
  std::string db8_api = "luna://com.webos.service.db/del";
  std::string db8_qry;
  pbnjson::JValue json_db_qry = pbnjson::Object();
  pbnjson::JValue json_where  = pbnjson::Object();

  json_db_qry.put("query", pbnjson::Object());
  json_db_qry["query"].put("from", db_name_);
  json_db_qry["query"].put("where", pbnjson::Array());

  json_where.put("prop", "launchPointId");
  json_where.put("op", "=");
  json_where.put("val", json["launchPointId"]);
  json_db_qry["query"]["where"].append(json_where);

  db8_qry = JUtil::jsonToString(json_db_qry);

  return Db8Query(db8_api, db8_qry);
}
