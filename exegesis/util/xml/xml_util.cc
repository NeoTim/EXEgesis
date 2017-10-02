// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "exegesis/util/xml/xml_util.h"

#include <vector>
#include "strings/string.h"

#include "glog/logging.h"
#include "strings/str_cat.h"
#include "tinyxml2.h"
#include "util/task/canonical_errors.h"
#include "util/task/status.h"
#include "util/task/status_macros.h"
#include "util/task/statusor.h"

namespace exegesis {
namespace xml {

namespace {

using ::exegesis::util::InternalError;
using ::exegesis::util::NotFoundError;
using ::exegesis::util::OkStatus;
using ::exegesis::util::Status;
using ::exegesis::util::StatusOr;
using ::tinyxml2::XMLDocument;
using ::tinyxml2::XMLElement;
using ::tinyxml2::XMLError;
using ::tinyxml2::XMLNode;
using ::tinyxml2::XMLPrinter;

}  // namespace

Status GetStatus(const XMLError& status) {
  if (status == XMLError::XML_SUCCESS) return OkStatus();
  // Use a dummy document to print a user-friendly error string.
  XMLDocument dummy;
  dummy.SetError(status, "", "");
  return InternalError(StrCat("XML Error ", status, ": ", dummy.ErrorName()));
}

string DebugString(const XMLNode* node) {
  CHECK(node != nullptr);
  XMLPrinter printer;
  node->Accept(&printer);
  return string(printer.CStr());
}

StatusOr<XMLElement*> FindChild(XMLNode* node, const char* name) {
  CHECK(node != nullptr);
  XMLElement* element = node->FirstChildElement(name);
  if (element != nullptr) return element;
  return NotFoundError(StrCat("Element <", name ? name : "",
                              "> not found in:\n", DebugString(node)));
}

std::vector<XMLElement*> FindChildren(XMLNode* node, const char* name) {
  CHECK(node != nullptr);
  std::vector<XMLElement*> children;
  for (XMLElement* element = node->FirstChildElement(name); element != nullptr;
       element = element->NextSiblingElement(name)) {
    children.push_back(element);
  }
  return children;
}

string ReadAttribute(const XMLElement* element, const char* name) {
  CHECK(element != nullptr);
  CHECK(name != nullptr);
  const char* value = element->Attribute(name);
  return value ? string(value) : "";
}

StatusOr<int> ReadIntAttribute(const XMLElement* element, const char* name) {
  CHECK(element != nullptr);
  CHECK(name != nullptr);
  int value = -1;
  RETURN_IF_ERROR(GetStatus(element->QueryIntAttribute(name, &value)));
  return value;
}

int ReadIntAttributeOrDefault(const XMLElement* element, const char* name,
                              int default_value) {
  CHECK(element != nullptr);
  CHECK(name != nullptr);
  const auto value = ReadIntAttribute(element, name);
  return value.ok() ? value.ValueOrDie() : default_value;
}

string ReadSimpleText(const XMLElement* element) {
  CHECK(element != nullptr);
  const char* text = element->GetText();
  return text ? string(text) : "";
}

string ReadHtmlText(const XMLElement* element) {
  CHECK(element != nullptr);
  XMLPrinter printer(nullptr, /* compact = */ true, /* depth = */ 0);
  element->Accept(&printer);
  return string(CHECK_NOTNULL(printer.CStr()));
}

}  // namespace xml
}  // namespace exegesis