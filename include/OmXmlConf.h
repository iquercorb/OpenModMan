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
#ifndef OMXMLDOC_H
#define OMXMLDOC_H

#include "OmBase.h"

class OmXmlNode;

/// \brief OmXmlNode pointer array
///
/// Typedef for an STL vector of OmXmlNode type
///
typedef std::vector<OmXmlNode> OmXmlNodeArray;

/// \brief Xml node interface.
///
/// Generic XML node, part of an XML data structure.
///
class OmXmlNode
{
  friend class OmXmlDoc;
  friend class OmXmlConf;

  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor
    ///
    /// Default object constructor
    ///
    OmXmlNode();

    /// \brief Constructor
    ///
    /// \param[in]  other : Other instance to copy.
    ///
    /// Copy constructor.
    ///
    OmXmlNode(const OmXmlNode& other);

    /// \brief Destructor
    ///
    /// Default object destructor
    ///
    ~OmXmlNode();

    /// \brief Assign operator.
    ///
    /// Copy from another instance.
    ///
    /// \param[in]  other : Other instance to copy.
    ///
    /// \return Reference to this
    ///
    OmXmlNode& operator=(const OmXmlNode& other);

    /// \brief Check empty.
    ///
    /// Check whether this instance is empty and does not represent any existing
    /// node in data tree.
    ///
    /// \return True if this instance is empty, false otherwise.
    ///
    bool empty() const;

    /// \brief Get node name.
    ///
    /// Returns the node tag name.
    ///
    /// \return Node tag name.
    ///
    const wchar_t* name() const;

    /// \brief Get node content.
    ///
    /// Returns the node plain text content.
    ///
    /// \return Node plain text content.
    ///
    const wchar_t* content() const;

    /// \brief Check whether node has attribute.
    ///
    /// Check whether node has the specified attribute.
    ///
    /// \param[in]  attr  : Attribute name to check.
    ///
    /// \return True if node has attribute, false otherwise.
    ///
    bool hasAttr(const OmWString& attr) const;

    /// \brief Get attribute as string.
    ///
    /// Returns the specified node attribute data as string
    ///
    /// \param[in]  attr  : Attribute name to get data from.
    ///
    /// \return Attribute data as string
    ///
    const wchar_t* attrAsString(const OmWString& attr) const;

    /// \brief Get attribute as integer.
    ///
    /// Returns the specified node attribute data as an integer number.
    ///
    /// \param[in]  attr  : Attribute name to get data from.
    ///
    /// \return Attribute data as an integer number.
    ///
    int attrAsInt(const OmWString& attr) const;

    /// \brief Get attribute as float.
    ///
    /// Returns the specified node attribute data as an float number.
    ///
    /// \param[in]  attr  : Attribute name to get data from.
    ///
    /// \return Attribute data as an float number.
    ///
    float attrAsFloat(const OmWString& attr) const;

    /// \brief Get attribute as double.
    ///
    /// Returns the specified node attribute data as an double float number.
    ///
    /// \param[in]  attr  : Attribute name to get data from.
    ///
    /// \return Attribute data as an double float number.
    ///
    double attrAsDouble(const OmWString& attr) const;

    /// \brief Get attribute as 64 bits unsigned integer.
    ///
    /// Returns the specified node attribute data as 64 bits unsigned integer number.
    ///
    /// \param[in]  attr  : Attribute name to get data from.
    ///
    /// \return Attribute data as 64 bits unsigned integer number.
    ///
    //uint64_t attrAsUint64(const OmWString& attr, int base) const;
    uint64_t attrAsUint64(const OmWString& attr) const;

    /// \brief Set node name.
    ///
    /// Set the node tag name.
    ///
    /// \param[in]  value : Name to set.
    ///
    void setName(const OmWString& value);

    /// \brief Set node content.
    ///
    /// Set the node plain text content.
    ///
    /// \param[in]  value : Content to set.
    ///
    void setContent(const OmWString& value);

    /// \brief Set node attribute.
    ///
    /// Set node attribute value, create attribute if doesn't exists.
    ///
    /// \param[in]  attr  : Attribute name.
    /// \param[in]  value : Attribute value to set.
    ///
    void setAttr(const OmWString& attr, const OmWString& value);

    /// \brief Set node attribute.
    ///
    /// Set node attribute value, create attribute if doesn't exists.
    ///
    /// \param[in]  attr  : Attribute name.
    /// \param[in]  value : Attribute value to set.
    ///
    void setAttr(const OmWString& attr, int value);

    /// \brief Set node attribute.
    ///
    /// Set node attribute value, create attribute if doesn't exists.
    ///
    /// \param[in]  attr  : Attribute name.
    /// \param[in]  value : Attribute value to set.
    ///
    void setAttr(const OmWString& attr, float value);

    /// \brief Set node attribute.
    ///
    /// Set node attribute value, create attribute if doesn't exists.
    ///
    /// \param[in]  attr  : Attribute name.
    /// \param[in]  value : Attribute value to set.
    ///
    void setAttr(const OmWString& attr, double value);

    /// \brief Set node attribute.
    ///
    /// Set node attribute value, create attribute if doesn't exists.
    ///
    /// \param[in]  attr  : Attribute name.
    /// \param[in]  value : Attribute value to set.
    ///
    void setAttr(const OmWString& attr, uint64_t value);

    /// \brief Remove node attribute.
    ///
    /// Remove attribute from node.
    ///
    /// \param[in]  attr  : Attribute name.
    ///
    /// \return True if operation succeed, false node was not found.
    ///
    bool remAttr(const OmWString& attr);

    /// \brief Get parent node.
    ///
    /// Returns parent node of this instance.
    ///
    /// \return parent node.
    ///
    OmXmlNode parent() const;

    /// \brief Check whether node has child by tag name.
    ///
    /// Checks whether this node has at least one child with the specified
    /// tag name.
    ///
    /// \param[in]  name  : Child tag name to check.
    ///
    /// \return True if node has child with specified tag name.
    ///
    bool hasChild(const OmWString& name) const;

    /// \brief Checks whether has child by attribute value.
    ///
    /// Checks whether this node has has at least one child with specified
    /// tag name and attribute value.
    ///
    /// \param[in]  name  : Child tag name to check.
    /// \param[in]  attr  : Child  Attribute name to check.
    /// \param[in]  value : Child  Attribute value to check.
    ///
    /// \return True if found, false otherwise.
    ///
    bool hasChild(const OmWString& name, const OmWString& attr, const OmWString& value) const;

    /// \brief Get total children count.
    ///
    /// Returns count of all direct children nodes of this instance.
    ///
    /// \return Count of children.
    ///
    unsigned childCount() const;

    /// \brief Get children count by tag name.
    ///
    /// Returns count of direct children nodes of this instance which have the
    /// specified tag name.
    ///
    /// \return Count of children with the specified tag name.
    ///
    unsigned childCount(const OmWString& name) const;

    /// \brief Get child.
    ///
    /// Returns node child at specified index.
    ///
    /// \param[in]  i   : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(unsigned i = 0) const;

    /// \brief Get child by tag name.
    ///
    /// Returns node child with specified tag name at specified index.
    ///
    /// \param[in]  name  : Child tag name.
    /// \param[in]  i     : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(const OmWString& name, unsigned i = 0) const;

    /// \brief Get child by tag name and attribute value.
    ///
    /// Returns the first node child with specified tag
    /// name and attribute value.
    ///
    /// \param[in]  name  : Child tag name.
    /// \param[in]  attr  : Child attribute name.
    /// \param[in]  value : Child attribute value.
    ///
    /// \return The first child that matches.
    ///
    OmXmlNode child(const OmWString& name, const OmWString& attr, const OmWString& value);

    /// \brief Get children list.
    ///
    /// Returns list of direct children of this instance.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children() const;

    /// \brief Get children list.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    ///
    void children(OmXmlNodeArray& ret) const;

    /// \brief Get children list by tag name.
    ///
    /// Returns list of direct children with specified tag name of this instance.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children(const OmWString& name) const;

    /// \brief Get children list by tag name.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    /// \param[in]  name  : Children tag name.
    ///
    void children(OmXmlNodeArray& ret, const OmWString& name) const;

    /// \brief Add new child.
    ///
    /// Creates a new node child of this instance.
    ///
    /// \param[in]  name  : Tag name of child to create.
    ///
    /// \return Created node.
    ///
    OmXmlNode addChild(const OmWString& name);

    /// \brief Remove child.
    ///
    /// Deletes the specified child node.
    ///
    /// \param[in]  child : Reference to node to delete.
    ///
    /// \return True if operation succeed, false if an error occurred.
    ///
    bool remChild(const OmXmlNode& child);

    /// \brief Remove child by tag name.
    ///
    /// Deletes child node with the specified tag name.
    ///
    /// \param[in]  name  : Tag name of child to delete.
    ///
    /// \return True if operation succeed, false node was not found.
    ///
    bool remChild(const OmWString& name);

    /// \brief Clear node.
    ///
    /// Reset node to set it as empty or invalid.
    ///
    void clear();

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _node;    //< XML node class pointer

};


/// \brief Xml document interface
///
/// Generic XML document data structure.
///
class OmXmlDoc
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmXmlDoc();

    /// \brief Constructor.
    ///
    /// Copy destructor.
    ///
    OmXmlDoc(const OmXmlDoc& other);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmXmlDoc();

    /// \brief Copy operator.
    ///
    /// Copy operator.
    ///
    OmXmlDoc& operator=(const OmXmlDoc& other);

    /// \brief Parse XML data.
    ///
    /// Parse the supplied XML data.
    ///
    /// \param[in]  xml   : XML data to parse.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool parse(const OmWString& xml);

    /// \brief Load XML file.
    ///
    /// Read and parse the specified XML file.
    ///
    /// \param[in]  path  : Path to XML to load.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(const OmWString& path);

    /// \brief Save XML file.
    ///
    /// Write the specified XML file with current document structure.
    ///
    /// \param[in]  dst   : Path to XML to save.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save(const OmWString& dst);

    /// \brief Get XML data string.
    ///
    /// Returns XML document data as string.
    ///
    /// \return XML data.
    ///
    OmCString data();

    /// \brief Check empty.
    ///
    /// Check whether this instance is empty.
    ///
    /// \return True if this instance is empty, false otherwise.
    ///
    bool empty() const;

    /// \brief Get document root node.
    ///
    /// Returns the document root node.
    ///
    /// \return Document root node.
    ///
    OmXmlNode root() const;

    /// \brief Check whether node has child by tag name.
    ///
    /// Checks whether this node has at least one child with the specified
    /// tag name.
    ///
    /// \param[in]  name  : Child tag name to check.
    ///
    /// \return True if node has child with specified tag name.
    ///
    bool hasChild(const OmWString& name) const;

    /// \brief Get total children count.
    ///
    /// Returns count of all direct children nodes of this instance.
    ///
    /// \return Count of children.
    ///
    unsigned childCount() const;

    /// \brief Get children count by tag name.
    ///
    /// Returns count of direct children nodes of this instance which have the
    /// specified tag name.
    ///
    /// \return Count of children with the specified tag name.
    ///
    unsigned childCount(const OmWString& name) const;

    /// \brief Get child.
    ///
    /// Returns node child at specified index.
    ///
    /// \param[in]  i     : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(unsigned i = 0) const;

    /// \brief Get child by tag name.
    ///
    /// Returns node child with specified tag name at specified index.
    ///
    /// \param[in]  name  : Child tag name.
    /// \param[in]  i     : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(const OmWString& name, unsigned i = 0) const;

    /// \brief Get children list.
    ///
    /// Returns list of direct children of this instance.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children() const;

    /// \brief Get children list.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    ///
    void children(OmXmlNodeArray& ret) const;

    /// \brief Get children list by tag name.
    ///
    /// Returns list of direct children with specified tag name of this instance.
    ///
    /// \param[in]  name    : Children tag name.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children(const OmWString& name) const;

    /// \brief Get children list by tag name.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    /// \param[in]  name  : Children tag name.
    ///
    void children(OmXmlNodeArray& ret, const OmWString& name) const;

    /// \brief Add new child.
    ///
    /// Creates a new node child of this instance.
    ///
    /// \param[in] name   : Tag name of child to create.
    ///
    /// \return Created node.
    ///
    OmXmlNode addChild(const OmWString& name);

    /// \brief Remove child.
    ///
    /// Deletes the specified child node.
    ///
    /// \param[in]  child : Reference to node to delete.
    ///
    /// \return True if operation succeed, false if an error occurred.
    ///
    bool remChild(const OmXmlNode& child);

    /// \brief Remove child by tag name.
    ///
    /// Deletes child node with the specified tag name.
    ///
    /// \param[in]  child : Tag name of child to delete.
    ///
    /// \return True if operation succeed, false node was not found.
    ///
    bool remChild(const OmWString& name);

    /// \brief Clear document.
    ///
    /// Reset document XML data structure.
    ///
    void clear();

    /// \brief Get last error string.
    ///
    /// Returns the string corresponding to the last XML parsing error code.
    ///
    /// \return XML parsing error as string.
    ///
    OmWString lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _docu;        //< XML document class pointer

    unsigned            _ercode;      //< last error code

    uint64_t            _erpoff;      //< last error position offset
};


/// \brief Xml document interface
///
/// Generic XML document data structure.
///
class OmXmlConf
{
  public: ///         - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    /// \brief Constructor.
    ///
    /// Default constructor.
    ///
    OmXmlConf();

    /// \brief Constructor.
    ///
    /// Copy constructor.
    ///
    OmXmlConf(const OmXmlConf& other);

    /// \brief Constructor with initiliazion.
    ///
    /// Constructor with initial root node.
    ///
    OmXmlConf(const OmWString& sign);

    /// \brief Destructor.
    ///
    /// Default destructor.
    ///
    ~OmXmlConf();

    /// \brief Copy operator.
    ///
    /// Copy operator.
    ///
    OmXmlConf& operator=(const OmXmlConf& other);

    /// \brief Parse XML config data.
    ///
    /// Try to parse XML config data with the root node. If
    /// expected root node is not the same, the function fail.
    ///
    /// \param[in]  xml     : XML content to parse.
    /// \param[in]  sign    : Expected root node name.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool parse(const OmWString& xml, const OmWString& sign);

    /// \brief Open an existing XML config file.
    ///
    /// Try to open XML config file with the specified root node. If
    /// expected root node is not the same, the function fail.
    ///
    /// \param[in]  path    : Path to file to be parsed.
    /// \param[in]  sign    : Expected root node to validate.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool load(const OmWString& path, const OmWString& sign);

    /// \brief Initialize new XML config.
    ///
    /// Initialize new XML config structure with specified root node.
    ///
    /// \param[in]  sign    : Config root node to initialize.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    void init(const OmWString& sign);

    /// \brief Initialize new XML config.
    ///
    /// Initialize new XML config structure with specified root node
    /// and path to save file.
    ///
    /// \param[in]  path    : Path to file to set as default for saving.
    /// \param[in]  sign    : Config root node to initialize.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    void init(const OmWString& path, const OmWString& sign);

    /// \brief Save loaded XML config.
    ///
    /// Save a previously loaded XML config file.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save();

    /// \brief Save XML config.
    ///
    /// Save XML config file as specified filename and location.
    ///
    /// \return True if operation succeed, false otherwise.
    ///
    bool save(const OmWString& path);

    /// \brief Get XML data string.
    ///
    /// Returns XML document data as string.
    ///
    /// \return XML data.
    ///
    OmCString data();

    /// \brief Check empty.
    ///
    /// Check whether this instance is empty.
    ///
    /// \return True if this instance is empty, false otherwise.
    ///
    bool empty() const;

    /// \brief Check valid.
    ///
    /// Check whether this instance is valid.
    ///
    /// \return True if this instance is valid, false otherwise.
    ///
    bool valid() const;

    /// \brief Check whether node has child by tag name.
    ///
    /// Checks whether this node has at least one child with the specified
    /// tag name.
    ///
    /// \param[in]  name  : Child tag name to check.
    ///
    /// \return True if node has child with specified tag name.
    ///
    bool hasChild(const OmWString& name) const;

    /// \brief Get total children count.
    ///
    /// Returns count of all direct children nodes of this instance.
    ///
    /// \return Count of children.
    ///
    unsigned childCount() const;

    /// \brief Get children count by tag name.
    ///
    /// Returns count of direct children nodes of this instance which have the
    /// specified tag name.
    ///
    /// \return Count of children with the specified tag name.
    ///
    unsigned childCount(const OmWString& name) const;

    /// \brief Get child.
    ///
    /// Returns node child at specified index.
    ///
    /// \param[in]  i     : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(unsigned i = 0) const;

    /// \brief Get child by tag name.
    ///
    /// Returns node child with specified tag name at specified index.
    ///
    /// \param[in]  name  : Child tag name.
    /// \param[in]  i     : Index of child to get.
    ///
    /// \return Child at specified index.
    ///
    OmXmlNode child(const OmWString& name, unsigned i = 0) const;

    /// \brief Get child by tag name and attribute value.
    ///
    /// Returns the first node child with specified tag
    /// name and attribute value.
    ///
    /// \param[in]  name  : Child tag name.
    /// \param[in]  attr  : Child attribute name.
    /// \param[in]  value : Child attribute value.
    ///
    /// \return The first child that matches.
    ///
    OmXmlNode child(const OmWString& name, const OmWString& attr, const OmWString& value);

    /// \brief Get children list.
    ///
    /// Returns list of direct children of this instance.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children() const;

    /// \brief Get children list.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    ///
    void children(OmXmlNodeArray& ret) const;

    /// \brief Get children list by tag name.
    ///
    /// Returns list of direct children with specified tag name of this instance.
    ///
    /// \param[in]  name    : Children tag name.
    ///
    /// \return List of children.
    ///
    OmXmlNodeArray children(const OmWString& name) const;

    /// \brief Get children list by tag name.
    ///
    /// Retrieves list of direct children of this instance.
    ///
    /// \param[out] ret   : OmXmlNode vector array to get list.
    /// \param[in]  name  : Children tag name.
    ///
    void children(OmXmlNodeArray& ret, const OmWString& name) const;

    /// \brief Add new child.
    ///
    /// Creates a new node child of this instance.
    ///
    /// \param[in] name   : Tag name of child to create.
    ///
    /// \return Created node.
    ///
    OmXmlNode addChild(const OmWString& name);

    /// \brief Remove child.
    ///
    /// Deletes the specified child node.
    ///
    /// \param[in]  child : Reference to node to delete.
    ///
    /// \return True if operation succeed, false if an error occurred.
    ///
    bool remChild(const OmXmlNode& child);

    /// \brief Remove child by tag name.
    ///
    /// Deletes child node with the specified tag name.
    ///
    /// \param[in]  child : Tag name of child to delete.
    ///
    /// \return True if operation succeed, false node was not found.
    ///
    bool remChild(const OmWString& name);

    /// \brief Clear document.
    ///
    /// Reset document XML data structure.
    ///
    void clear();

    /// \brief Get last error string.
    ///
    /// Returns the string corresponding to the last XML parsing error code.
    ///
    /// \return XML parsing error as string.
    ///
    OmWString lastErrorStr() const;

  private: ///          - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void*               _docu;        //< XML document class pointer

    void*               _root;        //< XML root node class pointer

    OmWString           _path;        //< Saved file path

    unsigned            _ercode;      //< last error code

    uint64_t            _erpoff;      //< last error position offset
};


#endif // OMXMLDOC_H
