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
#include "OmBase.h"

#include "OmBaseApp.h"

#include "OmUtilFs.h"
#include "OmUtilWin.h"
#include "OmUtilHsh.h"
#include "OmUtilErr.h"
#include "OmUtilStr.h"

#include "OmModMan.h"
#include "OmModHub.h"
#include "OmModChan.h"
#include "OmModPack.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmModPset.h"


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPset::OmModPset() :
  _ModHub(nullptr),
  _locked(false),
  _index(0),
  _installonly(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPset::OmModPset(OmModHub* ModHub) :
  _ModHub(ModHub),
  _locked(false),
  _index(0),
  _installonly(false)
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPset::~OmModPset()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPset::open(const OmWString& path)
{
  // clear the instance
  this->close();

  // try to open and parse the XML file
  if(!this->_xml.load(path, OM_XMAGIC_PST)) {
    this->_error(L"open", Om_errParse(L"Preset definition", path, this->_xml.lastErrorStr()));
    return false;
  }

  // check for the presence of <uuid> and <title> nodes
  if(!this->_xml.hasChild(L"uuid") || !this->_xml.hasChild(L"title")) {
    this->_error(L"open", Om_errParse(L"Preset definition", path, L"base node(s) missing"));
    return false;
  }

  this->_path = path;
  this->_uuid = this->_xml.child(L"uuid").content();
  this->_title = this->_xml.child(L"title").content();
  this->_index = this->_xml.child(L"title").attrAsInt(L"index");

  // check for <options>
  if(this->_xml.hasChild(L"options"))
    this->_installonly = this->_xml.child(L"options").attrAsInt(L"installonly");

  this->_log(OM_LOG_OK, L"open", L"succeed");

  // Launch repair
  this->repair();

  return true;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::setTitle(const OmWString& title)
{
  if(this->_locked || !this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"title")) {
    this->_xml.child(L"title").setContent(title);
  } else {
    this->_xml.addChild(L"title").setContent(title);
  }

  this->_xml.save();

  this->_title = title;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::setIndex(unsigned index)
{
  if(this->_locked || !this->_xml.valid())
    return;

  if(this->_xml.hasChild(L"title")) {
    this->_xml.child(L"title").setAttr(L"index", static_cast<int>(index));
  }

  this->_xml.save();

  this->_index = index;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::setInstallOnly(bool enable)
{
  if(this->_locked || !this->_xml.valid())
    return;

  OmXmlNode xml_options;
  if(this->_xml.hasChild(L"options")) {
    xml_options = this->_xml.child(L"options");
  } else {
    xml_options = this->_xml.addChild(L"options");
  }

  xml_options.setAttr(L"installonly", static_cast<int>(enable));

  this->_xml.save();

  this->_installonly = enable;

}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmModPset::setupCount()
{
  if(this->_xml.valid())
    return this->_xml.childCount(L"setup");

  return 0;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPset::discardSetup(const OmWString& uuid)
{
  if(this->_locked || !this->_xml.valid())
    return false;

  bool found = false;

  // get the proper <setup> node.
  OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", uuid);

  // if <setup> with UUID is found, delete it
  if(!xml_setup.empty()) {

    this->_xml.remChild(xml_setup);

    found = true;
  }

  // save definition
  //this->_xml.save();

  return found;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::addSetupEntry(const OmModChan* ModChan, const OmModPack* ModPack)
{
  if(this->_locked || !this->_xml.valid())
    return;

  // get the proper <setup> node.
  OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

  // if no <setup> with UUID was found, create it
  if(xml_setup.empty()) {
    xml_setup = this->_xml.addChild(L"setup");
    xml_setup.setAttr(L"uuid", ModChan->uuid());
  }

  // add <install> entry if not already exists
  if(!xml_setup.hasChild(L"install", L"ident", ModPack->iden())) {
     OmXmlNode xml_ins = xml_setup.addChild(L"install");
     xml_ins.setAttr(L"ident", ModPack->iden());
     xml_ins.setAttr(L"hash", ModPack->hash());
  }

  // save definition
  //this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPset::deleteSetupEntry(const OmModChan* ModChan, const OmWString& iden)
{
  if(this->_locked || !this->_xml.valid())
    return false;

  bool found = false;

  // get the proper <setup> node.
  OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

  // if no <setup> with uuid was found, return
  if(xml_setup.empty()) {
    return false;
  }

  // check for <install> entry and remove it
  if(xml_setup.hasChild(L"install", L"ident", iden)) {
    found = xml_setup.remChild(xml_setup.child(L"install", L"ident", iden));
  }

  // save definition
  //this->_xml.save();

  return found;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmModPset::setupEntryCount(const OmModChan* ModChan)
{
  if(this->_xml.valid()) {

    // get the proper <setup> node.
    OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_setup.empty())
      return xml_setup.childCount(L"install");
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmModPack* OmModPset::getSetupEntry(const OmModChan* ModChan, size_t i)
{
  if(this->_xml.valid()) {

    // get the proper <setup> node.
    OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_setup.empty()) {

      OmXmlNode xml_install = xml_setup.child(L"install", i);
      OmModPack* ModPack;

      // first try and rely on package hash value
      if(xml_install.hasAttr(L"hash")) {
        if((ModPack = ModChan->findModpack(xml_install.attrAsUint64(L"hash"))))
          return ModPack;
      }

      // then try with identity
      if(xml_install.hasAttr(L"ident")) {
        if((ModPack = ModChan->findModpack(xml_install.attrAsString(L"ident"))))
          return ModPack;
      }
    }
  }

  return nullptr;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPset::setupHasEntry(const OmModChan* ModChan, OmModPack* ModPack)
{
  if(this->_xml.valid()) {

    // get the proper <setup> node.
    OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_setup.empty()) {

      OmXmlNodeArray xml_install;
      xml_setup.children(xml_install, L"install");

      for(size_t i = 0; i < xml_install.size(); ++i) {
        if(xml_install[i].hasAttr(L"hash")) {
          if(xml_install[i].attrAsUint64(L"hash") == ModPack->hash())
            return true;
        }
        if(xml_install[i].hasAttr(L"ident")) {
          if(xml_install[i].attrAsString(L"ident") == ModPack->iden())
            return true;
        }
      }
    }
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmModPset::getSetupEntryList(const OmModChan* ModChan, OmPModPackArray* mod_ls)
{
  if(this->_xml.valid()) {

    // get the proper <setup> node.
    OmXmlNode xml_setup = this->_xml.child(L"setup", L"uuid", ModChan->uuid());

    // if found, return count of <install> child nodes.
    if(!xml_setup.empty()) {

      OmXmlNodeArray xml_install;
      xml_setup.children(xml_install, L"install");

      for(size_t i = 0; i < xml_install.size(); ++i) {

        OmModPack* ModPack;

        // first try and rely on package hash value
        if(xml_install[i].hasAttr(L"hash")) {
          if((ModPack = ModChan->findModpack(xml_install[i].attrAsUint64(L"hash")))) {
            mod_ls->push_back(ModPack); continue;
          }
        }

        // then try with identity
        if(xml_install[i].hasAttr(L"ident")) {
          if((ModPack = ModChan->findModpack(xml_install[i].attrAsString(L"ident")))) {
            mod_ls->push_back(ModPack); continue;
          }
        }
      }
      return mod_ls->size();
    }
  }

  return 0;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmModPset::repair()
{
  if(!this->_ModHub) {
    return false;
  }

  if(this->_locked || !this->_xml.valid())
    return false;

  bool has_change = false;

  // <setup> nodes list
  OmXmlNodeArray xml_setup;
  this->_xml.children(xml_setup, L"setup");

  // remove Mod Channel broken references

  // list for nodes to remove from tree
  OmXmlNodeArray xml_discard;

  // search for reference to non-existing Mod Channels
  for(size_t i = 0; i < xml_setup.size(); ++i) {

    if(!this->_ModHub->findChannel(xml_setup[i].attrAsString(L"uuid")))
      xml_discard.push_back(xml_setup[i]);
  }

  // delete ghost references to non existing Mod Channels
  if(xml_discard.size()) {

    for(size_t i = 0; i < xml_discard.size(); ++i)
      this->_xml.remChild(xml_discard[i]);

    has_change = true;  //< the definition has changed
  }

  // clear and load new setup list
  xml_setup.clear();
  this->_xml.children(xml_setup, L"setup");

  OmXmlNodeArray xml_install;

  // remove Package broken references
  for(size_t i = 0; i < xml_setup.size(); ++i) {

    // get Mod Channel
    OmModChan* ModChan = this->_ModHub->findChannel(xml_setup[i].attrAsString(L"uuid"));

    // get install list for this setup
    xml_install.clear();
    xml_setup[i].children(xml_install, L"install");

    // clear the discard list
    xml_discard.clear();

    // build the discard list or repair reference
    for(size_t j = 0; j < xml_install.size(); ++j) {

      OmModPack* ModPack = nullptr;

      if(xml_install[j].hasAttr(L"hash")) {

        uint64_t hash = xml_install[j].attrAsUint64(L"hash");

        if((ModPack = ModChan->findModpack(hash))) {

            // add missing iden reference
            if(!xml_install[j].hasAttr(L"ident")) {
              xml_install[j].setAttr(L"ident", ModPack->iden());
              has_change = true;
            }

        } else {
          xml_discard.push_back(xml_install[j]);
        }

      } else if(xml_install[j].hasAttr(L"ident")) {

        OmWString iden = xml_install[j].attrAsString(L"ident");

        if((ModPack = ModChan->findModpack(iden))) {

            // add missing hash reference
            if(!xml_install[j].hasAttr(L"hash")) {
              xml_install[j].setAttr(L"hash", ModPack->hash());
              has_change = true;
            }

        } else {
          xml_discard.push_back(xml_install[j]);
        }

      } else { // no <ident> and no <hash>: invalid node
        xml_discard.push_back(xml_install[j]);
      }

    }

    // remove nodes
    if(xml_discard.size()) {

      for(size_t j = 0; j < xml_discard.size(); ++j)
        xml_setup[i].remChild(xml_discard[j]);

      has_change = true;
    }
  }

  // save definition
  if(has_change)
    this->_xml.save();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::save()
{
  if(this->_locked || !this->_xml.valid())
    return;

  this->_xml.save();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::close()
{
  this->_xml.clear();
  this->_path.clear();
  this->_title.clear();
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::_log(unsigned level, const OmWString& origin,  const OmWString& detail)
{
  if(this->_ModHub) {
    OmWString root(L"ModPset["); root.append(Om_getNamePart(this->_path)); root.append(L"].");
    this->_ModHub->escalateLog(level, root + origin, detail);
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmModPset::_error(const OmWString& origin, const OmWString& detail)
{
  this->_lasterr = detail;
  this->_log(OM_LOG_ERR, origin, detail);
}

