/*
  This file is part of Open Mod Manager.

  Open Mod Manager is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Mod Manager is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Mod Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#include "thirdparty/pugixml/pugixml.hpp"
#include "OmXmlDoc.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::OmXmlNode() :
  _data(new pugi::xml_node)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::OmXmlNode(const OmXmlNode& other) :
  _data(new pugi::xml_node)
{
  *static_cast<pugi::xml_node*>(_data) = *static_cast<pugi::xml_node*>(other._data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::~OmXmlNode()
{
  delete static_cast<pugi::xml_node*>(_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode& OmXmlNode::operator=(const OmXmlNode& other)
{
  *static_cast<pugi::xml_node*>(_data) = *static_cast<pugi::xml_node*>(other._data);
  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::empty() const
{
  return static_cast<pugi::xml_node*>(_data)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::parent() const
{
  OmXmlNode parent;
  *static_cast<pugi::xml_node*>(parent._data) = static_cast<pugi::xml_node*>(_data)->parent();
  return parent;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::name() const
{
  return static_cast<pugi::xml_node*>(_data)->name();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::content() const
{
  return static_cast<pugi::xml_node*>(_data)->child_value();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::hasAttr(const wstring& attr) const
{
  return !(static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str()).empty());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::attrAsString(const wstring& attr) const
{
  return static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str()).value();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmXmlNode::attrAsInt(const wstring& attr) const
{
  return static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str()).as_int();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
float OmXmlNode::attrAsFloat(const wstring& attr) const
{
  return static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str()).as_float();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
double OmXmlNode::attrAsDouble(const wstring& attr) const
{
  return static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str()).as_double();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setName(const wstring& value)
{
  static_cast<pugi::xml_node*>(_data)->set_name(value.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setContent(const wstring& value)
{
  if(static_cast<pugi::xml_node*>(_data)->first_child().type() == pugi::node_pcdata) {
    static_cast<pugi::xml_node*>(_data)->first_child().set_value(value.c_str());
  } else {
    static_cast<pugi::xml_node*>(_data)->append_child(pugi::node_pcdata).set_value(value.c_str());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const wstring& attr, const wstring& value)
{
  pugi::xml_attribute attribute = static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = static_cast<pugi::xml_node*>(_data)->append_attribute(attr.c_str());
  }
  attribute.set_value(value.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const wstring& attr, int value)
{
  pugi::xml_attribute attribute = static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = static_cast<pugi::xml_node*>(_data)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const wstring& attr, float value)
{
  pugi::xml_attribute attribute = static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = static_cast<pugi::xml_node*>(_data)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const wstring& attr, double value)
{
  pugi::xml_attribute attribute = static_cast<pugi::xml_node*>(_data)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = static_cast<pugi::xml_node*>(_data)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlNode::childCount() const
{
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    ++n;
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::child(unsigned i) const
{
  OmXmlNode result;
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(n == i) {
      *static_cast<pugi::xml_node*>(result._data) = child;
      return result;
    }
    ++n;
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
vector<OmXmlNode> OmXmlNode::children() const
{
  vector<OmXmlNode> result;

  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._data) = child;
    result.push_back(node);
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::children(vector<OmXmlNode>& result) const
{
  result.clear();
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._data) = child;
    result.push_back(node);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::hasChild(const wstring& name) const
{
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlNode::childCount(const wstring& name) const
{
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      ++n;
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::child(const wstring& name, unsigned i) const
{
  OmXmlNode result;

  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      if(n == i) {
        *static_cast<pugi::xml_node*>(result._data) = child;
        return result;
      }
      ++n;
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
vector<OmXmlNode> OmXmlNode::children(const wstring& name) const
{
  vector<OmXmlNode> result;

  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *static_cast<pugi::xml_node*>(node._data) = child;
      result.push_back(node);
    }
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::children(vector<OmXmlNode>& result, const wstring& name) const
{
  result.clear();
  for(pugi::xml_node child = static_cast<pugi::xml_node*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *static_cast<pugi::xml_node*>(node._data) = child;
      result.push_back(node);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::addChild(const wstring& name)
{
  OmXmlNode result;
  *static_cast<pugi::xml_node*>(result._data) = static_cast<pugi::xml_node*>(_data)->append_child(name.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::remChild(const OmXmlNode& child)
{
  return static_cast<pugi::xml_node*>(_data)->remove_child(*static_cast<pugi::xml_node*>(child._data));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::remChild(const wstring& name)
{
  return static_cast<pugi::xml_node*>(_data)->remove_child(name.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::clear()
{
  delete static_cast<pugi::xml_node*>(_data);
  _data = new pugi::xml_node;
}





/// \brief PugiXML writer structure
///
/// Custom writer structure used to get XML data as a string object.
///
typedef struct _Xml_wstring_writer: pugi::xml_writer
{
  string result;
  virtual void write(const void* data, size_t size) {
    result.append(static_cast<const char*>(data), size);
  }
} _Xml_wstring_writer;


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::OmXmlDoc() :
  _data(new pugi::xml_document),
  _ercode(0),
  _erpoff(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::OmXmlDoc(const OmXmlDoc& other) :
  _data(new pugi::xml_document),
  _ercode(0),
  _erpoff(0)
{
  static_cast<pugi::xml_document*>(_data)->reset(*static_cast<pugi::xml_document*>(other._data));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::~OmXmlDoc()
{
  delete static_cast<pugi::xml_document*>(_data);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc& OmXmlDoc::operator=(const OmXmlDoc& other)
{
  static_cast<pugi::xml_document*>(_data)->reset(*static_cast<pugi::xml_document*>(other._data));
  _ercode = 0;
  _erpoff = 0;

  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::parse(const wstring& xml)
{
  pugi::xml_parse_result result;
  result = static_cast<pugi::xml_document*>(_data)->load_string(xml.c_str(), pugi::parse_default);
  if(!result) {
    _ercode = result.status;
    _erpoff = result.offset;
    return false;
  }
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::load(const wstring& src)
{
  pugi::xml_parse_result result;
  result = static_cast<pugi::xml_document*>(_data)->load_file(src.c_str(), pugi::parse_default, pugi::encoding_utf8);
  if(!result) {
    _ercode = result.status;
    _erpoff = result.offset;
    return false;
  }
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::save(const wstring& dst)
{
  if(!static_cast<pugi::xml_document*>(_data)->save_file(dst.c_str(), L"\t", pugi::format_default, pugi::encoding_utf8)) {
    _ercode = 17;
    return false;
  }
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
string OmXmlDoc::data()
{
  _Xml_wstring_writer writer;
  static_cast<pugi::xml_document*>(_data)->print(writer);
  return writer.result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::empty() const
{
  return static_cast<pugi::xml_document*>(_data)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::root() const
{
  OmXmlNode node;
  *static_cast<pugi::xml_node*>(node._data) = static_cast<pugi::xml_document*>(_data)->root();
  return node;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlDoc::childCount() const
{
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling())
    ++n;
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::child(unsigned i) const
{
  OmXmlNode result;
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(n == i) {
      *static_cast<pugi::xml_node*>(result._data) = child;
      return result;
    }
    ++n;
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
vector<OmXmlNode> OmXmlDoc::children() const
{
  vector<OmXmlNode> result;

  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._data) = child;
    result.push_back(node);
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::children(vector<OmXmlNode>& result) const
{
  result.clear();
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._data) = child;
    result.push_back(node);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::hasChild(const wstring& name) const
{
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlDoc::childCount(const wstring& name) const
{
  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      ++n;
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::child(const wstring& name, unsigned i) const
{
  OmXmlNode result;

  unsigned n = 0;
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      if(n == i) {
        *static_cast<pugi::xml_node*>(result._data) = child;
        return result;
      }
      ++n;
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
vector<OmXmlNode> OmXmlDoc::children(const wstring& name) const
{
  vector<OmXmlNode> result;

  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *static_cast<pugi::xml_node*>(node._data) = child;
      result.push_back(node);
    }
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::children(vector<OmXmlNode>& result, const wstring& name) const
{
  result.clear();
  for(pugi::xml_node child = static_cast<pugi::xml_document*>(_data)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *static_cast<pugi::xml_node*>(node._data) = child;
      result.push_back(node);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::addChild(const wstring& name)
{
  OmXmlNode result;
  *static_cast<pugi::xml_node*>(result._data) = static_cast<pugi::xml_document*>(_data)->append_child(name.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::remChild(const OmXmlNode& child)
{
  return static_cast<pugi::xml_document*>(_data)->remove_child(*static_cast<pugi::xml_node*>(child._data));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::remChild(const wstring& name)
{
  return static_cast<pugi::xml_document*>(_data)->remove_child(name.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::clear()
{
  static_cast<pugi::xml_document*>(_data)->reset();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
wstring OmXmlDoc::lastErrorStr() const
{
  wstring result;

  switch(_ercode)
  {
  case pugi::status_ok:
    result = L"PARSE_OK";
    break;
  case pugi::status_file_not_found:
    result = L"FILE_NOT_FOUND";
    break;
  case pugi::status_io_error:
    result = L"FILE_IO_READ_ERROR";
    break;
  case pugi::status_out_of_memory:
    result = L"OUT_OF_MEMORY";
    break;
  case pugi::status_internal_error:
    result = L"XML_PARSE_INTERNAL_ERROR";
    break;
  case pugi::status_unrecognized_tag:
    result = L"XML_PARSE_UNRECOGNIZED_TAG";
    break;
  case pugi::status_bad_pi:
    result = L"XML_PARSE_BAD_PI";
    break;
  case pugi::status_bad_comment:
    result = L"XML_PARSE_BAD_COMMENT";
    break;
  case pugi::status_bad_cdata:
    result = L"XML_PARSE_BAD_CDATA";
    break;
  case pugi::status_bad_doctype:
    result = L"XML_PARSE_BAD_DOCTYPE";
    break;
  case pugi::status_bad_pcdata:
    result = L"XML_PARSE_BAD_PCDATA";
    break;
  case pugi::status_bad_start_element:
    result = L"XML_PARSE_BAD_START_ELEMENT";
    break;
  case pugi::status_bad_attribute:
    result = L"XML_PARSE_BAD_ATTRIBUTE";
    break;
  case pugi::status_bad_end_element:
    result = L"XML_PARSE_BAD_END_ELEMENT";
    break;
  case pugi::status_end_element_mismatch:
    result = L"XML_PARSE_END_ELEMENT_MISMATCH";
    break;
  case pugi::status_no_document_element:
    result = L"XML_PARSE_NO_DOCUMENT_ELEMENT";
    break;
  case 17:
    result = L"FILE_IO_WRITE_ERROR";
    break;
  default:
    result = L"XML_PARSE_UNKNOW_ERROR";
    return result;
  }

  if(_ercode > 4) {
    result += L" at ";
    result += _erpoff;
  }

  return result;
}
