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
#include "pugixml/pugixml.hpp"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmXmlConf.h"


#define PUGI_DOC(x) static_cast<pugi::xml_document*>(x)

#define PUGI_NODE(x) static_cast<pugi::xml_node*>(x)

/// \brief Hexadecimal digits
///
/// Static translation string to convert integer value to hexadecimal digit.
///
static const wchar_t __hex_digit[] = L"0123456789abcdef";


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::OmXmlNode() :
  _node(new pugi::xml_node)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::OmXmlNode(const OmXmlNode& other) :
  _node(new pugi::xml_node)
{
  *PUGI_NODE(_node) = *PUGI_NODE(other._node);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode::~OmXmlNode()
{
  delete PUGI_NODE(_node);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode& OmXmlNode::operator=(const OmXmlNode& other)
{
  *PUGI_NODE(_node) = *PUGI_NODE(other._node);
  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::empty() const
{
  return PUGI_NODE(_node)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::parent() const
{
  OmXmlNode parent;
  *PUGI_NODE(parent._node) = PUGI_NODE(_node)->parent();
  return parent;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::name() const
{
  return PUGI_NODE(_node)->name();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::content() const
{
  return PUGI_NODE(_node)->child_value();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::hasAttr(const OmWString& attr) const
{
  return !(PUGI_NODE(_node)->attribute(attr.c_str()).empty());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
const wchar_t* OmXmlNode::attrAsString(const OmWString& attr) const
{
  return PUGI_NODE(_node)->attribute(attr.c_str()).value();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
int OmXmlNode::attrAsInt(const OmWString& attr) const
{
  return PUGI_NODE(_node)->attribute(attr.c_str()).as_int();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
float OmXmlNode::attrAsFloat(const OmWString& attr) const
{
  return PUGI_NODE(_node)->attribute(attr.c_str()).as_float();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
double OmXmlNode::attrAsDouble(const OmWString& attr) const
{
  return PUGI_NODE(_node)->attribute(attr.c_str()).as_double();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
uint64_t OmXmlNode::attrAsUint64(const OmWString& attr) const
{
  return PUGI_NODE(_node)->attribute(attr.c_str()).as_ullong();
}
/*
uint64_t OmXmlNode::attrAsUint64(const OmWString& attr, int base) const
{
  return wcstoull(PUGI_NODE(_node)->attribute(attr.c_str()).value(), nullptr, base);
}
*/

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setName(const OmWString& value)
{
  PUGI_NODE(_node)->set_name(value.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setContent(const OmWString& value)
{
  if(PUGI_NODE(_node)->first_child().type() == pugi::node_pcdata) {
    PUGI_NODE(_node)->first_child().set_value(value.c_str());
  } else {
    PUGI_NODE(_node)->append_child(pugi::node_pcdata).set_value(value.c_str());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const OmWString& attr, const OmWString& value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }
  attribute.set_value(value.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const OmWString& attr, int value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const OmWString& attr, float value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const OmWString& attr, double value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::setAttr(const OmWString& attr, uint64_t value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }
  attribute.set_value(value);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
/*
void OmXmlNode::setAttr(const OmWString& attr, uint64_t value)
{
  pugi::xml_attribute attribute = PUGI_NODE(_node)->attribute(attr.c_str());
  if(attribute.empty()) {
    attribute = PUGI_NODE(_node)->append_attribute(attr.c_str());
  }

  wchar_t buf[17];
  wchar_t *p = buf;

  uint8_t c, b = 64;
  while(b) {
    b -= 8;
    c = (uint8_t)((value >> b) & 0xFF);
    *p++ = __hex_digit[(c >> 4) & 0x0F];
    *p++ = __hex_digit[(c)      & 0x0F];
  }

  *p = 0;

  attribute.set_value(buf);
}
*/

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::remAttr(const OmWString& attr)
{
  return PUGI_NODE(_node)->remove_attribute(attr.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlNode::childCount() const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
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
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(n == i) {
      *PUGI_NODE(result._node) = child;
      return result;
    }
    ++n;
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNodeArray OmXmlNode::children() const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *PUGI_NODE(node._node) = child;
    result.push_back(node);
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::children(OmXmlNodeArray& result) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *PUGI_NODE(node._node) = child;
    result.push_back(node);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::hasChild(const OmWString& name) const
{
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::hasChild(const OmWString& name, const OmWString& attr, const OmWString& value) const
{
  pugi::xml_attribute xml_attr;

  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      xml_attr = child.attribute(attr.c_str());
      if(!xml_attr.empty()) {
        if(!wcscmp(value.c_str(), xml_attr.as_string()))
          return true;
      }
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlNode::childCount(const OmWString& name) const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      ++n;
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::child(const OmWString& name, unsigned i) const
{
  OmXmlNode result;

  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      if(n == i) {
        *PUGI_NODE(result._node) = child;
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
OmXmlNode OmXmlNode::child(const OmWString& name, const OmWString& attr, const OmWString& value)
{
  OmXmlNode result;

  pugi::xml_attribute xml_attr;

  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      xml_attr = child.attribute(attr.c_str());
      if(!xml_attr.empty()) {
        if(!wcscmp(value.c_str(), xml_attr.as_string())) {
          *PUGI_NODE(result._node) = child;
          return result;
        }
      }
    }
  }

  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNodeArray OmXmlNode::children(const OmWString& name) const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::children(OmXmlNodeArray& result, const OmWString& name) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_NODE(_node)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlNode::addChild(const OmWString& name)
{
  OmXmlNode result;
  *static_cast<pugi::xml_node*>(result._node) = PUGI_NODE(_node)->append_child(name.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::remChild(const OmXmlNode& child)
{
  return PUGI_NODE(_node)->remove_child(*PUGI_NODE(child._node));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlNode::remChild(const OmWString& name)
{
  return PUGI_NODE(_node)->remove_child(name.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlNode::clear()
{
  delete PUGI_NODE(_node);
  _node = new pugi::xml_node;
}





/// \brief PugiXML writer structure
///
/// Custom writer structure used to get XML data as a string object.
///
typedef struct _Xml_OmWString_writer: pugi::xml_writer
{
  OmCString result;
  virtual void write(const void* data, size_t size) {
    result.append(static_cast<const char*>(data), size);
  }
} _Xml_OmWString_writer;


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::OmXmlDoc() :
  _docu(new pugi::xml_document),
  _ercode(0),
  _erpoff(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::OmXmlDoc(const OmXmlDoc& other) :
  _docu(new pugi::xml_document),
  _ercode(0),
  _erpoff(0)
{

  PUGI_DOC(_docu)->reset(*PUGI_DOC(other._docu));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc::~OmXmlDoc()
{
  delete PUGI_DOC(_docu);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlDoc& OmXmlDoc::operator=(const OmXmlDoc& other)
{
  PUGI_DOC(_docu)->reset(*PUGI_DOC(other._docu));
  _ercode = 0;
  _erpoff = 0;

  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::parse(const OmWString& xml)
{
  pugi::xml_parse_result result;
  result = PUGI_DOC(_docu)->load_string(xml.c_str(), pugi::parse_default);
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
bool OmXmlDoc::load(const OmWString& src)
{
  pugi::xml_parse_result result;
  result = PUGI_DOC(_docu)->load_file(src.c_str(), pugi::parse_default, pugi::encoding_utf8);
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
bool OmXmlDoc::save(const OmWString& dst)
{
  if(!PUGI_DOC(_docu)->save_file(dst.c_str(), L"  ", pugi::format_default|pugi::format_save_file_text, pugi::encoding_utf8)) {
    _ercode = 17;
    return false;
  }
  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmCString OmXmlDoc::data()
{
  _Xml_OmWString_writer writer;
  PUGI_DOC(_docu)->print(writer);
  return writer.result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::empty() const
{
  return PUGI_DOC(_docu)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::root() const
{
  OmXmlNode node;
  *PUGI_NODE(node._node) = PUGI_DOC(_docu)->document_element();
  return node;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlDoc::childCount() const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling())
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
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(n == i) {
      *static_cast<pugi::xml_node*>(result._node) = child;
      return result;
    }
    ++n;
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNodeArray OmXmlDoc::children() const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._node) = child;
    result.push_back(node);
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::children(OmXmlNodeArray& result) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *PUGI_NODE(node._node) = child;
    result.push_back(node);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::hasChild(const OmWString& name) const
{
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlDoc::childCount(const OmWString& name) const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      ++n;
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::child(const OmWString& name, unsigned i) const
{
  OmXmlNode result;

  unsigned n = 0;
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      if(n == i) {
        *PUGI_NODE(result._node) = child;
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
OmXmlNodeArray OmXmlDoc::children(const OmWString& name) const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::children(OmXmlNodeArray& result, const OmWString& name) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_DOC(_docu)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlDoc::addChild(const OmWString& name)
{
  OmXmlNode result;
  *PUGI_NODE(result._node) = PUGI_DOC(_docu)->append_child(name.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::remChild(const OmXmlNode& child)
{
  return PUGI_DOC(_docu)->remove_child(*PUGI_NODE(child._node));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlDoc::remChild(const OmWString& name)
{
  return PUGI_DOC(_docu)->remove_child(name.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlDoc::clear()
{
  PUGI_DOC(_docu)->reset();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmXmlDoc::lastErrorStr() const
{
  OmWString result;

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




///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlConf::OmXmlConf() :
  _docu(new pugi::xml_document),
  _root(new pugi::xml_node),
  _ercode(0),
  _erpoff(0)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlConf::OmXmlConf(const OmXmlConf& other) :
  _docu(new pugi::xml_document),
  _root(new pugi::xml_node),
  _ercode(0),
  _erpoff(0)
{
  PUGI_DOC(_docu)->reset(*PUGI_DOC(other._docu));
  *PUGI_NODE(_root) = PUGI_DOC(_docu)->document_element();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlConf::OmXmlConf(const OmWString& sign) :
  _docu(new pugi::xml_document),
  _root(new pugi::xml_node),
  _ercode(0),
  _erpoff(0)
{
  *PUGI_NODE(_root) = PUGI_DOC(_docu)->append_child(sign.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlConf::~OmXmlConf()
{
  delete PUGI_DOC(_docu);
  delete PUGI_NODE(_root);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlConf& OmXmlConf::operator=(const OmXmlConf& other)
{
  PUGI_DOC(_docu)->reset(*PUGI_DOC(other._docu));
  *PUGI_NODE(_root) = PUGI_DOC(_docu)->document_element();
  _ercode = 0;
  _erpoff = 0;

  return *this;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::parse(const OmWString& xml, const OmWString& sign)
{
  this->clear();

  pugi::xml_parse_result result;
  result = PUGI_DOC(_docu)->load_string(xml.c_str(), pugi::parse_default);
  if(!result) {
    _ercode = result.status;
    _erpoff = result.offset;
    return false;
  }

  if(sign == PUGI_DOC(_docu)->document_element().name()) {
    *PUGI_NODE(_root) = PUGI_DOC(_docu)->document_element();
    return true;
  }

  _ercode = pugi::status_no_document_element;

  PUGI_DOC(_docu)->reset();

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::load(const OmWString& path, const OmWString& sign)
{
  this->clear();

  pugi::xml_parse_result result;
  result = PUGI_DOC(_docu)->load_file(path.c_str(), pugi::parse_default, pugi::encoding_utf8);
  if(!result) {
    _ercode = result.status;
    _erpoff = result.offset;
    return false;
  }

  if(sign == PUGI_DOC(_docu)->document_element().name()) {
    *PUGI_NODE(_root) = PUGI_DOC(_docu)->document_element();
    _path = path;
    return true;
  }

  _ercode = pugi::status_no_document_element;

  PUGI_DOC(_docu)->reset();

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlConf::init(const OmWString& sign)
{
  *PUGI_NODE(_root) = PUGI_DOC(_docu)->append_child(sign.c_str());
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlConf::init(const OmWString& path, const OmWString& sign)
{
  *PUGI_NODE(_root) = PUGI_DOC(_docu)->append_child(sign.c_str());

  _path = path;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::save()
{
  if(!PUGI_DOC(_docu)->empty() && _path.size()) {

    if(!PUGI_DOC(_docu)->save_file(_path.c_str(), L"  ", pugi::format_default|pugi::format_save_file_text, pugi::encoding_utf8)) {
      _ercode = pugi::status_io_error;
      return false;
    }

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::save(const OmWString& path)
{
  if(!PUGI_DOC(_docu)->empty()) {

    if(!PUGI_DOC(_docu)->save_file(path.c_str(), L"  ", pugi::format_default|pugi::format_save_file_text, pugi::encoding_utf8)) {
      _ercode = pugi::status_io_error;
      return false;
    }

    return true;
  }

  return false;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmCString OmXmlConf::data()
{
  _Xml_OmWString_writer writer;
  PUGI_DOC(_docu)->print(writer);
  return writer.result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::empty() const
{
  return PUGI_DOC(_docu)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::valid() const
{
  return !PUGI_DOC(_docu)->empty();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlConf::childCount() const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling())
    ++n;
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlConf::child(unsigned i) const
{
  OmXmlNode result;
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(n == i) {
      *PUGI_NODE(result._node) = child;
      return result;
    }
    ++n;
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNodeArray OmXmlConf::children() const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *static_cast<pugi::xml_node*>(node._node) = child;
    result.push_back(node);
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlConf::children(OmXmlNodeArray& result) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    OmXmlNode node;
    *PUGI_NODE(node._node) = child;
    result.push_back(node);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::hasChild(const OmWString& name) const
{
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      return true;
    }
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
unsigned OmXmlConf::childCount(const OmWString& name) const
{
  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      ++n;
    }
  }
  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlConf::child(const OmWString& name, unsigned i) const
{
  OmXmlNode result;

  unsigned n = 0;
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      if(n == i) {
        *PUGI_NODE(result._node) = child;
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
OmXmlNodeArray OmXmlConf::children(const OmWString& name) const
{
  OmXmlNodeArray result;

  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlConf::children(OmXmlNodeArray& result, const OmWString& name) const
{
  result.clear();
  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      OmXmlNode node;
      *PUGI_NODE(node._node) = child;
      result.push_back(node);
    }
  }
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlConf::child(const OmWString& name, const OmWString& attr, const OmWString& value)
{
  OmXmlNode result;

  pugi::xml_attribute xml_attr;

  for(pugi::xml_node child = PUGI_NODE(_root)->first_child(); child; child = child.next_sibling()) {
    if(!wcscmp(name.c_str(), child.name())) {
      xml_attr = child.attribute(attr.c_str());
      if(!xml_attr.empty()) {
        if(!wcscmp(value.c_str(), xml_attr.as_string())) {
          *PUGI_NODE(result._node) = child;
          return result;
        }
      }
    }
  }

  return result;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmXmlNode OmXmlConf::addChild(const OmWString& name)
{
  OmXmlNode result;
  *PUGI_NODE(result._node) = PUGI_NODE(_root)->append_child(name.c_str());
  return result;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::remChild(const OmXmlNode& child)
{
  return PUGI_NODE(_root)->remove_child(*PUGI_NODE(child._node));
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmXmlConf::remChild(const OmWString& name)
{
  return PUGI_NODE(_root)->remove_child(name.c_str());
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmXmlConf::clear()
{
  PUGI_DOC(_docu)->reset();
  _path.clear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmWString OmXmlConf::lastErrorStr() const
{
  OmWString result;

  switch(_ercode)
  {
  case pugi::status_ok:
    result = L"PARSE_OK";
    break;
  case pugi::status_file_not_found:
    result = L"FILE_NOT_FOUND";
    break;
  case pugi::status_io_error:
    result = L"FILE_IO_ERROR";
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

