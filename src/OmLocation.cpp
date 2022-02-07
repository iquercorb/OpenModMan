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
#include <algorithm>            //< std::find

#include "OmBaseApp.h"

#include "Util/OmUtilFs.h"
#include "Util/OmUtilHsh.h"
#include "Util/OmUtilErr.h"
#include "Util/OmUtilStr.h"

#include "OmManager.h"
#include "OmContext.h"

///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
#include "OmLocation.h"


/// \brief Package name comparison callback
///
/// std::sort callback comparison function for sorting
/// package by name in alphabetical order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __pkg_sort_name_fn(const OmPackage* a, const OmPackage* b)
{
  // test against the shorter string
  size_t l = a->ident().size() < b->ident().size() ? b->ident().size() : a->ident().size();

  const wchar_t* a_srt = a->ident().c_str();
  const wchar_t* b_str = b->ident().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a_srt[i]) != towupper(b_str[i])) {
      if(towupper(a_srt[i]) < towupper(b_str[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->ident().size() != b->ident().size())
    if(a->ident().size() < b->ident().size())
      return true;

  // strings are strictly equals, we sort by "IsZip" status
  if(a->isZip() && !b->isZip())
    return true;

  return false;
}

/// \brief Remote package name comparison callback
///
/// std::sort callback comparison function for sorting
/// remote package by name in alphabetical order.
///
/// \param[in]  a     : Left Remote package.
/// \param[in]  b     : Right Remote package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __rmt_sort_name_fn(const OmRemote* a, const OmRemote* b)
{
  // test against the shorter string
  size_t l = a->ident().size() < b->ident().size() ? b->ident().size() : a->ident().size();

  const wchar_t* a_srt = a->ident().c_str();
  const wchar_t* b_str = b->ident().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a_srt[i]) != towupper(b_str[i])) {
      if(towupper(a_srt[i]) < towupper(b_str[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->ident().size() != b->ident().size())
    if(a->ident().size() < b->ident().size())
      return true;

  return false;
}


/// \brief Package category comparison callback
///
/// std::sort callback comparison function for sorting
/// package by category in alphabetical order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __pkg_sort_catg_fn(const OmPackage* a, const OmPackage* b)
{
  // test against the shorter string
  size_t l = a->category().size() < b->category().size() ? b->category().size() : a->category().size();

  const wchar_t* a_srt = a->category().c_str();
  const wchar_t* b_str = b->category().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a_srt[i]) != towupper(b_str[i])) {
      if(towupper(a_srt[i]) < towupper(b_str[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->category().size() != b->category().size())
    if(a->category().size() < b->category().size())
      return true;

  // strings are strictly equals, we sort by name
  return __pkg_sort_name_fn(a, b);
}


/// \brief Remote package category comparison callback
///
/// std::sort callback comparison function for sorting
/// remote package by category in alphabetical order.
///
/// \param[in]  a     : Left Remote package.
/// \param[in]  b     : Right Remote package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __rmt_sort_catg_fn(const OmRemote* a, const OmRemote* b)
{
  // test against the shorter string
  size_t l = a->category().size() < b->category().size() ? b->category().size() : a->category().size();

  const wchar_t* a_srt = a->category().c_str();
  const wchar_t* b_str = b->category().c_str();

  // test for ASCII value greater than the other
  for(unsigned i = 0; i < l; ++i) {
    if(towupper(a_srt[i]) != towupper(b_str[i])) {
      if(towupper(a_srt[i]) < towupper(b_str[i])) {
        return true;
      } else {
        return false;
      }
    }
  }

  // strings are equals in tester portion, sort by string size
  if(a->category().size() < b->category().size())
    return true;

  // strings are strictly equals, we sort by name
  return __rmt_sort_name_fn(a, b);
}


/// \brief Package version comparison callback
///
/// std::sort callback comparison function for sorting package
/// by version ascending.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __pkg_sort_vers_fn(const OmPackage* a, const OmPackage* b)
{
  if(a->version() == b->version()) {
    return __pkg_sort_name_fn(a, b);
  } else {
    return (a->version() < b->version());
  }
}


/// \brief Remote package version comparison callback
///
/// std::sort callback comparison function for sorting remote package
/// by version ascending.
///
/// \param[in]  a     : Left Remote package.
/// \param[in]  b     : Right Remote package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __rmt_sort_vers_fn(const OmRemote* a, const OmRemote* b)
{
  if(a->version() == b->version()) {
    return __rmt_sort_name_fn(a, b);
  } else {
    return (a->version() < b->version());
  }
}


/// \brief Package state comparison callback
///
/// std::sort callback comparison function for sorting package
/// by installation state order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __pkg_sort_stat_fn(const OmPackage* a, const OmPackage* b)
{
  if(a->hasBck() && b->hasBck()) {
    return __pkg_sort_name_fn(a, b);
  } else {
    return (a->hasBck() && !b->hasBck());
  }
}


/// \brief Remote package state comparison callback
///
/// std::sort callback comparison function for sorting remote package
/// by state order.
///
/// \param[in]  a     : Left Remote package.
/// \param[in]  b     : Right Remote package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __rmt_sort_stat_fn(const OmRemote* a, const OmRemote* b)
{
  if(a->state() == b->state()) {
    return __rmt_sort_name_fn(a, b);
  } else {
    if(a->isState(RMT_STATE_NEW) == b->isState(RMT_STATE_NEW)) {
      return (a->state() < b->state());
    } else {
      return (!a->isState(RMT_STATE_NEW) && b->isState(RMT_STATE_NEW));
    }
  }
}


/// \brief Remote package size comparison callback
///
/// std::sort callback comparison function for sorting remote package
/// by size order.
///
/// \param[in]  a     : Left Package.
/// \param[in]  b     : Right Package.
///
/// \return True if Package a is "before" Package b, false otherwise
///
static bool __rmt_sort_size_fn(const OmRemote* a, const OmRemote* b)
{
  if(a->bytes() == b->bytes()) {
    return __rmt_sort_name_fn(a, b);
  } else {
    return (a->bytes() < b->bytes());
  }
}



/// \brief Get package source dependencies.
///
/// Recursively explores package source dependency tree to gather all
/// found required packages and create list of missing dependencies for
/// the specified target.
///
/// \param[out]   out_ls : Output list of found dependency packages.
/// \param[out]   mis_ls : Output list missing dependency identities.
/// \param[in]    lib_ls : Input list of available packages.
/// \param[in]    target : Input target package to explore dependencies.
///
/// \return Count of dependency packages found.
///
static size_t __src_get_dependencies(vector<OmPackage*>& out_ls, vector<wstring>& mis_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  bool missing;

  for(size_t i = 0; i < target->depCount(); ++i) {

    missing = true;

    for(size_t j = 0; j < lib_ls.size(); ++j) {

      // rely only on packages
      if(!lib_ls[j]->isZip()) continue;

      if(target->depGet(i) == lib_ls[j]->ident()) {

        n += __src_get_dependencies(out_ls, mis_ls, lib_ls, lib_ls[j]);

        // we add to list only if unique and not already installed, this allow
        // us to get a consistent dependency list for a bunch of package by
        // calling this function for each package without clearing the list
        if(!lib_ls[j]->hasBck()) {
          if(std::find(out_ls.begin(), out_ls.end(), lib_ls[j]) == out_ls.end()) {
            out_ls.push_back(lib_ls[j]);
            ++n;
          }
        }

        missing = false;
        break;
      }
    }

    if(missing) {
      // we add to list only if unique
      if(std::find(mis_ls.begin(), mis_ls.end(), target->depGet(i)) == mis_ls.end()) {
        mis_ls.push_back(target->depGet(i));
      }
    }
  }

  return n;
}


/// \brief Get package source missing dependencies.
///
/// Recursively explores package source dependency tree to get
/// list of missing dependencies for the specified target.
///
/// \param[out]   mis_ls : Output list missing dependency identities.
/// \param[in]    lib_ls : Input list of available packages.
/// \param[in]    target : Input target package to explore dependencies.
///
/// \return Count of missing dependency packages.
///
static size_t __src_get_missing_deps(vector<wstring>& mis_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  bool missing;

  for(size_t i = 0; i < target->depCount(); ++i) {

    missing = true;

    for(size_t j = 0; j < lib_ls.size(); ++j) {

      // rely only on packages
      if(!lib_ls[j]->isZip()) continue;

      if(target->depGet(i) == lib_ls[j]->ident()) {

        n += __src_get_missing_deps(mis_ls, lib_ls, lib_ls[j]);

        missing = false;
        break;
      }
    }

    if(missing) {
      // we add to list only if unique
      if(std::find(mis_ls.begin(), mis_ls.end(), target->depGet(i)) == mis_ls.end()) {
        mis_ls.push_back(target->depGet(i));
        n++;
      }
    }
  }

  return n;
}


/// \brief Check package source dependencies
///
/// Recursively explores package source dependency tree to check
/// for missing dependencies of the specified target.
///
/// \param[in]    lib_ls : Input list of available packages.
/// \param[in]    target : Input target package to explore dependencies.
///
/// \return True if no dependency missing, false otherwise.
///
static bool __src_chk_dependencies(const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  bool missing;

  for(size_t i = 0; i < target->depCount(); ++i) {

    missing = true;

    for(size_t j = 0; j < lib_ls.size(); ++j) {

      // rely only on packages
      if(!lib_ls[j]->isZip())
        continue;

      if(target->depGet(i) == lib_ls[j]->ident()) {

        // recursively check
        if(!__src_chk_dependencies(lib_ls, lib_ls[j]))
          return false;

        missing = false;
        break;
      }
    }

    if(missing)
      return false;
  }

  return true;
}


/// \brief Check package dependents.
///
/// Recursively explores package dependency tree to check
/// for dependents packages of the specified target.
///
/// This function does not care if packages are currently installed
/// or not.
///
/// \param[out]   out_ls  : Output list of found dependents packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    target  : Input target package to check overlapping.
///
/// \return True if dependents package found, false otherwise.
///
static bool __src_chk_dependents(const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  for(size_t i = 0; i < lib_ls.size(); ++i) {

    if(lib_ls[i] == target)
      continue;

    if(lib_ls[i]->depHas(target->ident())) {
      return true;
    }
  }

  return false;
}


/// \brief Get package backup overlaps
///
/// Recursively explores package backup overlapping tree to gather all
/// packages that overlaps on the specified target.
///
/// \param[out]   out_ls  : Output list of found overlapping packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    target  : Input target package to check overlapping.
///
/// \return Count of overlapping packages found.
///
static size_t __bck_get_overlaps(vector<OmPackage*>& out_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  for(size_t i = 0; i < lib_ls.size(); ++i) {

    // search only among installed packages
    if(!lib_ls[i]->hasBck())
      continue;

    if(lib_ls[i] == target)
      continue;

    if(lib_ls[i]->ovrHas(target->hash())) {

      // the function is recursive, we want the full list like a
      // depth-first search in the right order
      n += __bck_get_overlaps(out_ls, lib_ls, lib_ls[i]);

      // recursive way can produce doubles, we want to avoid it
      // so we add only if not already in the list
      if(std::find(out_ls.begin(), out_ls.end(), lib_ls[i]) == out_ls.end()) {
        out_ls.push_back(lib_ls[i]);
        ++n;
      }
    }
  }

  return n;
}


/// \brief Get package backup dependents.
///
/// Recursively explores package backup dependency tree to gather all
/// dependents packages of the specified target.
///
/// This function lookup only on installed package to be used in
/// context of package uninstall check.
///
/// \param[out]   out_ls  : Output list of found dependents packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    target  : Input target package to check overlapping.
///
/// \return Count of dependents packages found.
///
static size_t __bck_get_dependents(vector<OmPackage*>& out_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  for(size_t i = 0; i < lib_ls.size(); ++i) {

    // search only among installed packages
    if(!lib_ls[i]->hasBck())
      continue;

    if(lib_ls[i] == target)
      continue;

    if(lib_ls[i]->depHas(target->ident())) {

      // check recursively, this give depth-first sorted list
      n += __bck_get_dependents(out_ls, lib_ls, lib_ls[i]);

      // add only if unique
      if(std::find(out_ls.begin(), out_ls.end(), lib_ls[i]) == out_ls.end()) {
        out_ls.push_back(lib_ls[i]);
        ++n;
      }
    }
  }

  return n;
}


/// \brief Check for package backup dependents.
///
/// Recursively explores package backup dependency tree to check
/// whether the specified target has installed dependents package.
///
/// This function lookup only on installed package to be used in
/// context of package uninstall check.
///
/// \param[out]   out_ls  : Output list of found dependents packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    target  : Input target package to check overlapping.
///
/// \return Count of dependents packages found.
///
static bool __bck_chk_dependents(const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  for(size_t i = 0; i < lib_ls.size(); ++i) {

    // search only among installed packages
    if(lib_ls[i]->hasBck() && lib_ls[i] != target) {

      if(lib_ls[i]->depHas(target->ident())) {
        return true;
      }
    }
  }

  return false;
}


/// \brief Get package backup dependencies.
///
/// Recursively explores installed package backup dependency tree to
/// gather all dependencies of the specified installed target.
///
/// \param[out]   out_ls  : Output list of found dependencies packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    target  : Input target package to check overlapping.
///
/// \return Count of dependencies packages found.
///
static size_t __bck_get_dependencies(vector<OmPackage*>& out_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  for(size_t i = 0; i < lib_ls.size(); ++i) {

    // search only among installed packages
    if(!lib_ls[i]->hasBck())
      continue;

    if(target->depHas(lib_ls[i]->ident())) {

      // check recursively, this give depth-first sorted list
      n += __bck_get_dependencies(out_ls, lib_ls, lib_ls[i]);

      // add only if unique
      if(std::find(out_ls.begin(), out_ls.end(), lib_ls[i]) == out_ls.end()) {
        out_ls.push_back(lib_ls[i]);
        ++n;
      }
    }
  }

  return n;
}


/// \brief Get package backup relations.
///
/// Recursively explores package backup relations tree to gather both
/// dependents packages and overlapping packages of the specified
/// target.
///
/// \param[out]   out_ls  : Output list of found related, either dependents or overlapping packages.
/// \param[out]   ovr_ls  : Output list of found overlapping packages.
/// \param[out]   dpt_ls  : Output list of found dependents packages.
/// \param[in]    lib_ls  : Input list of available packages.
/// \param[in]    hash    : Input target package to get backup relations.
///
/// \return Count of dependents and overlapping packages found.
///
static size_t __bck_get_relations(vector<OmPackage*>& out_ls, vector<OmPackage*>& ovr_ls, vector<OmPackage*>& dpt_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  size_t n = 0;

  bool is_ovr;
  bool is_dep;

  for(size_t i = 0; i < lib_ls.size(); ++i) {

    // search only among installed packages
    if(!lib_ls[i]->hasBck())
      continue;

    // check both if package is overlapping and/or depend on
    // the currently specified one
    is_ovr = lib_ls[i]->ovrHas(target->hash());
    is_dep = lib_ls[i]->depHas(target->ident());

    if(is_ovr || is_dep) {

      // we go for recursive search to get a properly sorted list of
      // packages in depth-first search order.
      n += __bck_get_relations(out_ls, ovr_ls, dpt_ls, lib_ls, lib_ls[i]);

      // we now add to the proper lists
      if(is_ovr) {
        // add only if unique
        if(std::find(ovr_ls.begin(), ovr_ls.end(), lib_ls[i]) == ovr_ls.end()) {
          ovr_ls.push_back(lib_ls[i]);
        }
      }

      if(is_dep) {
        // add only if unique
        if(std::find(dpt_ls.begin(), dpt_ls.end(), lib_ls[i]) == dpt_ls.end()) {
          dpt_ls.push_back(lib_ls[i]);
        }
      }

      // finally add to main list
      if(std::find(out_ls.begin(), out_ls.end(), lib_ls[i]) == out_ls.end()) {
        out_ls.push_back(lib_ls[i]);
        ++n;
      }
    }
  }

  return n;
}

///
/// Get remote package dependencies (declaration).
///
static size_t __rmt_get_dependencies(vector<OmRemote*>&, vector<wstring>&, const vector<OmRemote*>&, const vector<OmPackage*>&, const OmRemote*);

/// \brief Get package available remote dependencies.
///
/// Recursively explores package source dependency tree to gather all
/// found required remote packages and create list of missing dependencies
/// for the specified target.
///
/// \param[out]   out_ls : Output list of found dependency packages.
/// \param[out]   mis_ls : Output list missing dependency identities.
/// \param[in]    net_ls : Input list of available remote packages.
/// \param[in]    lib_ls : Input list of available library packages.
/// \param[in]    target : Input target remote package to explore dependencies.
///
/// \return Count of dependency remote packages found.
///
static size_t __pkg_get_downloads(vector<OmRemote*>& dnl_ls, vector<wstring>& mis_ls, const vector<OmRemote*>& net_ls, const vector<OmPackage*>& lib_ls, const OmPackage* target)
{
  // get all missing dependencies for this package
  vector<wstring> dep_ls;

  __src_get_missing_deps(dep_ls, lib_ls, target);

  if(dep_ls.empty())
    return 0;

  size_t n = 0;

  bool missing;

  // try to found packages in the remote package list
  for(size_t i = 0; i < dep_ls.size(); ++i) {

    missing = true;

    for(size_t r = 0; r < net_ls.size(); ++r) {

      if(dep_ls[i] == net_ls[r]->ident()) {

        n += __rmt_get_dependencies(dnl_ls, mis_ls, net_ls, lib_ls, net_ls[r]);

        if(std::find(dnl_ls.begin(), dnl_ls.end(), net_ls[r]) == dnl_ls.end()) {
          dnl_ls.push_back(net_ls[r]);
          ++n;
        }

        missing = false;
      }
    }

    if(missing) {
      // we add to list only if unique
      if(std::find(mis_ls.begin(), mis_ls.end(), dep_ls[i]) == mis_ls.end()) {
        mis_ls.push_back(dep_ls[i]);
      }
    }
  }

  return n;
}

/// \brief Get remote package dependencies.
///
/// Recursively explores remote package dependency tree to gather all
/// found required remote packages and create list of missing dependencies
/// for the specified target.
///
/// \param[out]   out_ls : Output list of found dependency packages.
/// \param[out]   mis_ls : Output list missing dependency identities.
/// \param[in]    net_ls : Input list of available remote packages.
/// \param[in]    lib_ls : Input list of available library packages.
/// \param[in]    target : Input target remote package to explore dependencies.
///
/// \return Count of dependency remote packages found.
///
size_t __rmt_get_dependencies(vector<OmRemote*>& out_ls, vector<wstring>& mis_ls, const vector<OmRemote*>& net_ls, const vector<OmPackage*>& lib_ls, const OmRemote* target)
{
  size_t n = 0;

  bool in_library;
  bool missing;

  // first check required dependency in local library to
  // gather all missing dependencies, this will add dependencies
  // to found in the remote package list
  for(size_t i = 0; i < target->depCount(); ++i) {

    in_library = false;

    // first check whether required dependency is in package library
    for(size_t p = 0; p < lib_ls.size(); ++p) {

      // rely only on packages
      if(!lib_ls[p]->isZip())
        continue;

      // if we found package, we must verify its dependencies list
      if(target->depGet(i) == lib_ls[p]->ident()) {

        // get all available remote dependencies for this package
        n += __pkg_get_downloads(out_ls, mis_ls, net_ls, lib_ls, lib_ls[p]);

        in_library = true;

        break;
      }
    }

    if(in_library) //< skip if already in library
      continue;

    missing = true;

    for(size_t r = 0; r < net_ls.size(); ++r) {

      if(target->depGet(i) == net_ls[r]->ident()) {

        n += __rmt_get_dependencies(out_ls, mis_ls, net_ls, lib_ls, net_ls[r]);

        if(std::find(out_ls.begin(), out_ls.end(), net_ls[r]) == out_ls.end()) {
          out_ls.push_back(net_ls[r]);
          ++n;
        }

        missing = false;
        break;
      }
    }

    if(missing) {
      // we add to list only if unique
      if(std::find(mis_ls.begin(), mis_ls.end(), target->depGet(i)) == mis_ls.end()) {
        mis_ls.push_back(target->depGet(i));
      }
    }
  }

  return n;
}

/// \brief Get required old packages.
///
/// Get list of library packages that the specified remote package
/// supersedes and is required as dependency by other.
///
/// \param[out]   out_ls : Output list of required old packages.
/// \param[in]    lib_ls : Input list of available library packages.
/// \param[in]    target : Input target remote package to explore dependencies.
///
/// \return Count of required old packages found.
///
static size_t __rmt_get_old_required(vector<OmPackage*>& out_ls, const vector<OmPackage*>& lib_ls, const OmRemote* target)
{
  size_t n = 0;

  // check for supersedes dependencies
  if(target->isState(RMT_STATE_UPG)) {

    for(size_t i = 0; i < target->supCount(); ++i) {

      if(__src_chk_dependents(lib_ls, target->supGet(i))) {

        // add only if unique
        if(std::find(out_ls.begin(), out_ls.end(), target->supGet(i)) == out_ls.end()) {
          out_ls.push_back(target->supGet(i));
          n++;
        }
      }
    }
  }

  return n;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmLocation::OmLocation(OmContext* pCtx) :
  _context(pCtx), _config(), _uuid(), _title(), _index(0), _home(), _path(),
  _dstDir(), _libDir(), _libDirCust(false), _libDevMode(true), _bckDir(),
  _bckDirCust(false), _pkgLs(), _bckZipLevel(0), _pkgSorting(LS_SORT_NAME),
  _upgdRename(false), _rmtSorting(LS_SORT_NAME), _warnOverlaps(true),
  _warnExtraInst(true), _warnMissDeps(true), _warnExtraUnin(true),
  _warnExtraDnld(true), _warnMissDnld(true), _valid(false), _error()
{

}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
OmLocation::~OmLocation()
{
  this->close();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::libSetSorting(OmLocLsSort sorting)
{
  // we check if the requested sorting kind is the same as the currently
  // used, in this case, this mean the sorting order must be reversed

  if(sorting & this->_pkgSorting) {

    // check if current sorting is reversed, then switch order
    if(this->_pkgSorting & LS_SORT_REVERSE) {
      this->_pkgSorting &= ~LS_SORT_REVERSE; //< remove reversed flag
    } else {
      this->_pkgSorting |= LS_SORT_REVERSE; //< add reversed flag
    }

  } else {

    this->_pkgSorting = sorting;
  }

  // save the current sorting
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"library_sort")) {
      this->_config.xml().child(L"library_sort").setAttr(L"sort", static_cast<int>(this->_pkgSorting));
    } else {
      this->_config.xml().addChild(L"library_sort").setAttr(L"sort", static_cast<int>(this->_pkgSorting));
    }

    this->_config.save();
  }

  // finally sort packages
  this->_pkgSort();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setLibDevMode(bool enable)
{
  this->_libDevMode = enable;

  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"library_devmode")) {
      this->_config.xml().child(L"library_devmode").setAttr(L"enable", this->_libDevMode ? 1 : 0);
    } else {
      this->_config.xml().addChild(L"library_devmode").setAttr(L"enable", this->_libDevMode ? 1 : 0);
    }

    this->_config.save();
  }

  // refresh all library for all locations
  this->libClear();
  this->libRefresh();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnOverlaps(bool enable)
{
  this->_warnOverlaps = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_opt;

    if(this->_config.xml().hasChild(L"warn_options")) {
      xml_opt = this->_config.xml().child(L"warn_options");
    } else {
      xml_opt = this->_config.xml().addChild(L"warn_options");
    }

    if(xml_opt.hasChild(L"warn_overlaps")) {
      xml_opt.child(L"warn_overlaps").setAttr(L"enable", this->_warnOverlaps ? 1 : 0);
    } else {
      xml_opt.addChild(L"warn_overlaps").setAttr(L"enable", this->_warnOverlaps ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnExtraInst(bool enable)
{
  this->_warnExtraInst = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_opt;

    if(this->_config.xml().hasChild(L"warn_options")) {
      xml_opt = this->_config.xml().child(L"warn_options");
    } else {
      xml_opt = this->_config.xml().addChild(L"warn_options");
    }

    if(xml_opt.hasChild(L"warn_extra_inst")) {
      xml_opt.child(L"warn_extra_inst").setAttr(L"enable", this->_warnExtraInst ? 1 : 0);
    } else {
      xml_opt.addChild(L"warn_extra_inst").setAttr(L"enable", this->_warnExtraInst ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnMissDeps(bool enable)
{
  this->_warnMissDeps = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_opt;

    if(this->_config.xml().hasChild(L"warn_options")) {
      xml_opt = this->_config.xml().child(L"warn_options");
    } else {
      xml_opt = this->_config.xml().addChild(L"warn_options");
    }

    if(xml_opt.hasChild(L"warn_miss_deps")) {
      xml_opt.child(L"warn_miss_deps").setAttr(L"enable", this->_warnMissDeps ? 1 : 0);
    } else {
      xml_opt.addChild(L"warn_miss_deps").setAttr(L"enable", this->_warnMissDeps ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnExtraUnin(bool enable)
{
  this->_warnExtraUnin = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_opt;

    if(this->_config.xml().hasChild(L"warn_options")) {
      xml_opt = this->_config.xml().child(L"warn_options");
    } else {
      xml_opt = this->_config.xml().addChild(L"warn_options");
    }

    if(xml_opt.hasChild(L"warn_extra_unin")) {
      xml_opt.child(L"warn_extra_unin").setAttr(L"enable", this->_warnExtraUnin ? 1 : 0);
    } else {
      xml_opt.addChild(L"warn_extra_unin").setAttr(L"enable", this->_warnExtraUnin ? 1 : 0);
    }

    this->_config.save();
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::open(const wstring& path)
{
  this->close();

  // try to open and parse the XML file
  if(!this->_config.open(path, OMM_XMAGIC_LOC)) {
    this->_error = Om_errParse(L"Definition file", Om_getFilePart(path), this->_config.lastErrorStr());
    this->log(0, L"Location(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <uuid> entry
  if(!this->_config.xml().hasChild(L"uuid")) {
    this->_error =  L"\""+Om_getFilePart(path)+L"\" invalid definition: <uuid> node missing.";
    log(0, L"Location(<anonymous>) Open", this->_error);
    return false;
  }

  // check for the presence of <title> entry
  if(!this->_config.xml().hasChild(L"title")) {
    this->_error =  L"\""+Om_getFilePart(path)+L"\" invalid definition: <title> node missing.";
    log(0, L"Location(<anonymous>) Open", this->_error);
    return false;
  }

  // at this point the Location may be valid
  this->_path = path;
  this->_home = Om_getDirPart(this->_path);
  this->_uuid = this->_config.xml().child(L"uuid").content();
  this->_title = this->_config.xml().child(L"title").content();
  this->_index = this->_config.xml().child(L"title").attrAsInt(L"index");

  this->log(2, L"Location("+this->_title+L") Open", L"Definition parsed.");

  // check for the presence of <install> entry
  if(this->_config.xml().hasChild(L"install")) {
    // we check whether destination folder is valid
    this->_dstDir = this->_config.xml().child(L"install").content();
    if(!Om_isDir(this->_dstDir)) {
      this->log(1, L"Location("+this->_title+L") Open", Om_errIsDir(L"Destination folder", this->_dstDir));
    } else {
      this->log(2, L"Location("+this->_title+L") Open", L"Using destination folder: \""+this->_dstDir+L"\".");
    }
  } else {
    this->_error = L"Invalid definition: <install> node missing.";
    this->log(0, L"Location("+this->_title+L") Open", this->_error);
    this->close();
    return false;
  }

  // check for the presence of <library> entry for custom Library path
  if(this->_config.xml().hasChild(L"library")) {
    // get the custom Library path in config
    this->_libDir = this->_config.xml().child(L"library").content();
    // notify we use a custom Library path
    this->_libDirCust = true;
    if(!Om_isDir(this->_libDir)) {
      this->log(1, L"Location("+this->_title+L") Open", Om_errIsDir(L"Custom Library folder", this->_libDir));
    } else {
      this->log(2, L"Location("+this->_title+L") Open", L"Using custom library folder: \""+this->_libDir+L"\".");
    }
  } else {
    // no <library> node in config, use default settings
    this->_libDir = this->_home + L"\\library";
    if(!Om_isDir(this->_libDir)) {
      int result = Om_dirCreate(this->_libDir);
      if(result != 0) {
        this->_error = Om_errCreate(L"Library folder", this->_libDir, result);
        this->log(0, L"Location("+this->_title+L") Open", this->_error);
        this->close();
        return false;
      }
    }
    this->log(2, L"Location("+this->_title+L") Open", L"Using default library folder: \""+this->_libDir+L"\".");
  }

  // check for the presence of <backup> entry for custom Backup path
  if(this->_config.xml().hasChild(L"backup")) {
    // get the custom Backup path in config
    this->_bckDir = this->_config.xml().child(L"backup").content();
    // notify we use a custom Backup path
    this->_bckDirCust = true;
    if(!Om_isDir(this->_bckDir)) {
      this->log(1, L"Location("+this->_title+L") Open", Om_errIsDir(L"Custom Backup folder", this->_bckDir));
    } else {
      this->log(2, L"Location("+this->_title+L") Open", L"Using custom backup folder: \""+this->_bckDir+L"\".");
    }
  } else {
    // no <backup> node in config, use default settings
    this->_bckDir = this->_home + L"\\backup";
    if(!Om_isDir(this->_bckDir)) {
      int result = Om_dirCreate(this->_bckDir);
      if(result != 0) {
        this->_error = Om_errCreate(L"Backup folder", this->_bckDir, result);
        this->log(0, L"Location("+this->_title+L") Open", this->_error);
        this->close();
        return false;
      }
    }
    this->log(2, L"Location("+this->_title+L") Open", L"Using default backup folder: \""+this->_bckDir+L"\".");
  }

  // we check for backup compression level
  if(this->_config.xml().hasChild(L"backup_comp")) {
    this->_bckZipLevel = this->_config.xml().child(L"backup_comp").attrAsInt(L"level");

    // check whether we have a correct value
    if(this->_bckZipLevel > 3)
      this->_bckZipLevel = -1;
  } else {
    this->setBckZipLevel(this->_bckZipLevel); //< create default
  }

  // we check for saved library sorting
  if(this->_config.xml().hasChild(L"library_sort")) {
    this->_pkgSorting = this->_config.xml().child(L"library_sort").attrAsInt(L"sort");
  }

  // we check for saved library devmode
  if(this->_config.xml().hasChild(L"library_devmode")) {
    this->_libDevMode = this->_config.xml().child(L"library_devmode").attrAsInt(L"enable");
  } else {
    this->setLibDevMode(this->_libDevMode); //< create default
  }

  // we check for saved remotes sorting
  if(this->_config.xml().hasChild(L"remotes_sort")) {
    this->_pkgSorting = this->_config.xml().child(L"remotes_sort").attrAsInt(L"sort");
  }

  // Check warnings options
  if(this->_config.xml().hasChild(L"warn_options")) {
    OmXmlNode xml_wrn = this->_config.xml().child(L"warn_options");

    if(xml_wrn.hasChild(L"warn_overlaps")) {
      this->_warnOverlaps = xml_wrn.child(L"warn_overlaps").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warnOverlaps);
    }

    if(xml_wrn.hasChild(L"warn_extra_inst")) {
      this->_warnOverlaps = xml_wrn.child(L"warn_extra_inst").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warnOverlaps);
    }


    if(xml_wrn.hasChild(L"warn_miss_deps")) {
      this->_warnOverlaps = xml_wrn.child(L"warn_miss_deps").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warnOverlaps);
    }


    if(xml_wrn.hasChild(L"warn_extra_unin")) {
      this->_warnOverlaps = xml_wrn.child(L"warn_extra_unin").attrAsInt(L"enable");
    } else {
      this->setWarnOverlaps(this->_warnOverlaps);
    }

  } else {
    // create default
    this->_config.xml().addChild(L"warn_options");
    this->setWarnOverlaps(this->_warnOverlaps);
    this->setWarnExtraInst(this->_warnExtraInst);
    this->setWarnMissDeps(this->_warnMissDeps);
    this->setWarnExtraUnin(this->_warnExtraUnin);
  }

  // Get network repository list
  if(this->_config.xml().hasChild(L"network")) {

    OmXmlNode xml_net = this->_config.xml().child(L"network");

    if(xml_net.hasAttr(L"upgd_rename")) {
      this->_upgdRename = xml_net.attrAsInt(L"upgd_rename");
    } else {
      this->_upgdRename = false;
    }

    // check whether repository already exists
    vector<OmXmlNode> xml_rep_list;
    xml_net.children(xml_rep_list, L"repository");

    OmRepository* pRep;

    for(size_t i = 0; i < xml_rep_list.size(); ++i) {
      pRep = new OmRepository(this);
      if(pRep->init(xml_rep_list[i].attrAsString(L"base"), xml_rep_list[i].attrAsString(L"name"))) {
        this->_repLs.push_back(pRep);
        this->log(2, L"Location("+this->_title+L") Load", L"Configured Repository: \""+pRep->url()+L"\".");
      } else {
        delete pRep;
      }
    }

    if(xml_net.hasChild(L"warn_extra_dnld")) {
      this->_warnExtraDnld = xml_net.child(L"warn_extra_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnExtraDnld(this->_warnExtraDnld);
    }

    if(xml_net.hasChild(L"warn_miss_dnld")) {
      this->_warnMissDnld = xml_net.child(L"warn_miss_dnld").attrAsInt(L"enable");
    } else {
      this->setWarnMissDnld(this->_warnMissDnld);
    }

    if(xml_net.hasChild(L"warn_upgd_brk_deps")) {
      this->_warnUpgdBrkDeps = xml_net.child(L"warn_upgd_brk_deps").attrAsInt(L"enable");
    } else {
      this->setWarnUpgdBrkDeps(this->_warnUpgdBrkDeps);
    }

  } else {
    // create default
    this->_config.xml().addChild(L"network");
    this->setWarnExtraDnld(this->_warnExtraDnld);
    this->setWarnMissDnld(this->_warnMissDnld);
    this->setWarnUpgdBrkDeps(this->_warnUpgdBrkDeps);
  }

  // this location is OK and ready
  this->_valid = true;

  // Refresh library
  this->libRefresh();

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::close()
{
  wstring title = this->_title;

  this->libClear();
  this->_home.clear();
  this->_path.clear();
  this->_title.clear();
  this->_dstDir.clear();
  this->_libDir.clear();
  this->_libDirCust = false;
  this->_bckDir.clear();
  this->_bckDirCust = false;
  this->_config.close();
  this->_valid = false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::dstDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_dstDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_dstDir, OMM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_dstDir, OMM_ACCESS_DIR_WRITE)) {
          this->_error = Om_errWriteAccess(L"Destination folder", this->_dstDir);
          access_ok = false;
        }
      }
    } else {
      this->_error = Om_errReadAccess(L"Destination folder", this->_dstDir);
      access_ok = false;
    }
  } else {
    this->_error = Om_errIsDir(L"Destination folder", this->_dstDir);
    access_ok = false;
  }

  if(!access_ok) {
    this->log(0, L"Location("+this->_title+L") Destination access", this->_error);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::libDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_libDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_libDir, OMM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_libDir, OMM_ACCESS_DIR_WRITE)) {
          this->_error = Om_errWriteAccess(L"Library folder", this->_libDir);
          access_ok = false;
        }
      }
    } else {
      this->_error = Om_errReadAccess(L"Library folder", this->_libDir);
      access_ok = false;
    }
  } else {
    if(this->_libDirCust) {
      this->_error = Om_errIsDir(L"Custom Library folder", this->_libDir);
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_libDir);
      if(result != 0) {
        this->_error = Om_errCreate(L"Library folder", this->_libDir, result);
        access_ok = false;
      }
    }
  }

  if(!access_ok) {
    this->log(0, L"Location("+this->_title+L") Library access", this->_error);
    return false;
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckDirAccess(bool rw)
{
  bool access_ok = true;

  // checks whether folder exists
  if(Om_isDir(this->_bckDir)) {
    // checks for proper permissions on folder
    if(Om_checkAccess(this->_bckDir, OMM_ACCESS_DIR_READ)) {
      if(rw) { //< check for writing access
        if(!Om_checkAccess(this->_bckDir, OMM_ACCESS_DIR_WRITE)) {
          this->_error = Om_errWriteAccess(L"Backup folder", this->_bckDir);
          access_ok = false;
        }
      }
    } else {
      this->_error = Om_errReadAccess(L"Backup folder", this->_bckDir);
      access_ok = false;
    }
  } else {
    if(this->_bckDirCust) {
      this->_error = Om_errIsDir(L"Custom Backup folder", this->_bckDir);
      access_ok = false;
    } else {
      // try to create it
      int result = Om_dirCreate(this->_bckDir);
      if(result != 0) {
        this->_error = Om_errCreate(L"Backup folder", this->_bckDir, result);
        access_ok = false;
      }
    }
  }

  if(!access_ok) {
    this->log(0, L"Location("+this->_title+L") Backup access", this->_error);
    return false;
  }

  return true;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::libClear()
{
  if(!this->_pkgLs.empty()) {

    for(size_t i = 0; i < this->_pkgLs.size(); ++i)
      delete this->_pkgLs[i];

    this->_pkgLs.clear();

    return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::libRefresh()
{
  if(!this->libDirAccess(false)) { // check for read access
    #ifdef DEBUG
    std::cout << "DEBUG => OmLocation::libRefresh X\n";
    #endif
    return this->libClear();
  }

  // some explanation about how Packages and Backups are managed...
  //
  // One Package object can incarnate two totally different things which
  // seam to be one unique thing in the user point of view:
  // A "Backup" and a "Source".
  //
  // The "Source" refers to the Package itself, i.e, the Mod's files to be
  // installed into the Destination folder.
  // The "Backup" refer to original application files saved in a safe place
  // before they were overwritten by the Mod's modified files.
  //
  // So, the Package object is double-sided, one side is the "Backup", the
  // other is the "Source". It may "be" either one, another, or both at the
  // same time, depending which element is actually available.
  //
  //   Backup File/Dir
  //            \ ______ Package
  //            /        Object
  //   Source File/Dir
  //
  // In this function, we create the Packages list the user will manipulate and
  // Package objects will be created with one, or both side depending what is
  // found in backup and library folders.
  //
  // "Source" and "Backup" are linked together using a unsigned 64 bit integer
  // xxHash value created from the "Source" Package file (or folder) name.
  //
  // When a "Source" is installed, this Hash is stored in a Backup definition
  // file within the Backup zip file or sub-folder, allowing to matches the
  // available Backups with available Sources.
  //
  // Notice that if user renames its package "Source" file, it will no longer
  // be linkable to an existing "Backup", it will be threated as a separate
  // Package.


  // This function is called either to initializes the package list, or to
  // refresh it because of a change in the Library folder.
  //
  // We want to avoid re-parsing all Backup and Sources for a small changes in
  // the Library folder or Backup status, so, if the package list if not empty,
  // we only update only the necessary.
  //
  // Notice that Backup are tracked internally, the algorithm does not support
  // changes in Backup folder by third-party.

  vector<wstring> path_ls;
  OmPackage* pPkg;

  // track list changes
  bool changed = false;

  // our package list is not empty, we will check for added or removed item
  if(this->_pkgLs.size()) {

    // get content of the package Library folder
    Om_lsFileFiltered(&path_ls, this->_libDir, L"*.zip", true);
    Om_lsFileFiltered(&path_ls, this->_libDir, L"*." OMM_PKG_FILE_EXT, true);
    if(this->_libDevMode)
      Om_lsDir(&path_ls, this->_libDir, true);

    bool in_list;

    // search for unavailable Sources
    for(size_t p = 0; p < this->_pkgLs.size(); ++p) {

      // search this Source in Library folder item list
      in_list = false;
      for(size_t k = 0; k < path_ls.size(); ++k) {
        if(this->_pkgLs[p]->srcPath() == path_ls[k]) { //< compare Source paths
          in_list = true;

          // compare last write time
          if(this->_pkgLs[p]->srcTime() != Om_itemTime(path_ls[k])) {
            // file changed, we parse again to update

            this->_pkgLs[p]->srcClear();
            this->_pkgLs[p]->srcParse(path_ls[k]);
            changed = true;
          }
          break;
        }
      }

      // this Source is no longer in Library folder
      if(!in_list) {
        changed = true;
        // check whether this Package is installed (has backup)
        if(this->_pkgLs[p]->hasBck()) {
          // this Package is installed, in this case we don't remove it from
          // list, but we revoke its "Source" property since it is no longer
          // available
          this->_pkgLs[p]->srcClear();
        } else {
          // The Package has no Backup and Source is no longer available
          // this is a ghost, we have to remove it
          this->_pkgLs.erase(this->_pkgLs.begin()+p);
          --p;
        }
      }
    }

    uint64_t pkg_hash;

    // Search for new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {
      // search in all packages to found this file
      in_list = false;

      pkg_hash = Om_getXXHash3(Om_getFilePart(path_ls[i]));

      for(size_t p = 0; p < this->_pkgLs.size(); ++p) {
        // we first test against the Source Path
        if(path_ls[i] == this->_pkgLs[p]->srcPath()) {
          in_list = true; break;
        }
        // checks whether Hash values matches
        if(pkg_hash == this->_pkgLs[p]->hash()) {
          // this Package Source obviously matches to a currently
          // installed one, since we got a Package with the same Hash but
          // Source is missing, so we add the Source to this Package Backup
          changed = true;
          this->_pkgLs[p]->srcParse(path_ls[i]);
          in_list = true; break;
        }
      }
      // This is a new Package Source
      if(!in_list) {
        pPkg = new OmPackage(this);
        if(pPkg->srcParse(path_ls[i])) {
          changed = true;
          this->_pkgLs.push_back(pPkg);
        } else {
          delete pPkg;
        }
      }
    }

  } else {

    changed = true;

    // get Backup folder content
    Om_lsFileFiltered(&path_ls, this->_bckDir, L"*.zip", true);
    Om_lsFileFiltered(&path_ls, this->_bckDir, L"*." OMM_BCK_FILE_EXT, true);
    Om_lsDir(&path_ls, this->_bckDir, true);

    // add all available and valid Backups
    for(size_t i = 0; i < path_ls.size(); ++i) {
      pPkg = new OmPackage(this);
      if(pPkg->bckParse(path_ls[i])) {
        this->_pkgLs.push_back(pPkg);
      } else {
        delete pPkg;
      }
    }

    // get Library folder content
    path_ls.clear();
    Om_lsFileFiltered(&path_ls, this->_libDir, L"*.zip", true);
    Om_lsFileFiltered(&path_ls, this->_libDir, L"*." OMM_PKG_FILE_EXT, true);
    if(this->_libDevMode)
      Om_lsDir(&path_ls, this->_libDir, true);

    bool has_bck;

    // Link Sources to matching Backup, or add new Sources
    for(size_t i = 0; i < path_ls.size(); ++i) {
      has_bck = false;
      // check whether this Source matches an existing Backup
      for(size_t p = 0; p < this->_pkgLs.size(); p++) {
        if(this->_pkgLs[p]->isBckOf(path_ls[i])) {
          this->_pkgLs[p]->srcParse(path_ls[i]);
          has_bck = true;
          break;
        }
      }
      // no Backup found for this Source, adding new Source
      if(!has_bck) {
        pPkg = new OmPackage(this);
        if(pPkg->srcParse(path_ls[i])) {
          this->_pkgLs.push_back(pPkg);
        } else {
          delete pPkg;
        }
      }
    }
  }

  if(changed)
    this->_pkgSort();

  #ifdef DEBUG
  std::cout << "DEBUG => OmLocation::libRefresh " << (changed ? "+-" : "==") << "\n";
  #endif

  return changed;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::libClean()
{
  bool changed = false;

  // search for ghost packages
  for(size_t p = 0; p < this->_pkgLs.size(); ++p) {

    if(!this->_pkgLs[p]->hasBck() && !this->_pkgLs[p]->hasSrc()) {
      // The Package has no Backup and Source is no longer available
      // this is a ghost, we have to remove it
      changed = true;
      this->_pkgLs.erase(this->_pkgLs.begin()+p);
      --p;
    }
  }

  #ifdef DEBUG
  std::cout << "DEBUG => OmLocation::libClean " << (changed ? "+-" : "==") << "\n";
  #endif

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setTitle(const wstring& title)
{
  if(this->_config.valid()) {

    this->_title = title;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setContent(title);
    } else {
      this->_config.xml().addChild(L"title").setContent(title);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setIndex(unsigned index)
{
  if(this->_config.valid()) {

    this->_index = index;

    if(this->_config.xml().hasChild(L"title")) {
      this->_config.xml().child(L"title").setAttr(L"index", static_cast<int>(index));
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setDstDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_dstDir = path;

    if(this->_config.xml().hasChild(L"install")) {
      this->_config.xml().child(L"install").setContent(path);
    } else {
      this->_config.xml().addChild(L"install").setContent(path);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setCustLibDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_libDir = path;
    // notify we use a custom Library path
    this->_libDirCust = true;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().child(L"library").setContent(path);
    } else {
      this->_config.xml().addChild(L"library").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustLibDir()
{
  if(this->_config.valid()) {

    this->_libDir = this->_home + L"\\Library";
    // notify we use default settings
    this->_libDirCust = false;

    if(this->_config.xml().hasChild(L"library")) {
      this->_config.xml().remChild(L"library");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setCustBckDir(const wstring& path)
{
  if(this->_config.valid()) {

    this->_bckDir = path;
    // notify we use a custom Library path
    this->_bckDirCust = true;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().child(L"backup").setContent(path);
    } else {
      this->_config.xml().addChild(L"backup").setContent(path);
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::remCustBckDir()
{
  if(this->_config.valid()) {

    this->_bckDir = this->_home + L"\\Backup";
    // notify we use default settings
    this->_bckDirCust = false;

    if(this->_config.xml().hasChild(L"backup")) {
      this->_config.xml().remChild(L"backup");
    }

    this->_config.save();
  }

  // Force a full refresh for the next time
  this->libClear();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setBckZipLevel(int level)
{
  if(this->_config.valid()) {

    this->_bckZipLevel = level;

    if(this->_config.xml().hasChild(L"backup_comp")) {
      this->_config.xml().child(L"backup_comp").setAttr(L"level", (int)level);
    } else {
      this->_config.xml().addChild(L"backup_comp").setAttr(L"level", (int)level);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setUpgdRename(bool enable)
{
  if(this->_config.valid()) {

    this->_upgdRename = enable;

    OmXmlNode xml_net;

    if(!this->_config.xml().hasChild(L"network")) {
      xml_net = this->_config.xml().addChild(L"network");
    } else {
      xml_net = this->_config.xml().child(L"network");
    }

    xml_net.setAttr(L"upgd_rename", static_cast<int>(enable ? 1 : 0));

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::renameHome(const wstring& name)
{
  wstring title = this->_title;
  wstring old_path = this->_path;
  wstring old_home = this->_home;

  // Close Location to safe rename and reload it after
  this->close();

  bool has_error = false;

  // compose new Location definition file name
  wstring new_file = name;
  new_file += L".";
  new_file += OMM_LOC_DEF_FILE_EXT;

  // Rename Location definition file
  int result = Om_fileMove(old_path, old_home + L"\\" + new_file);
  if(result != 0) {
    this->_error = Om_errRename(L"Definition file", old_path, result);
    this->log(0, L"Location("+title+L") Rename", this->_error);
    // get back the old file name to restore Location properly
    new_file = Om_getFilePart(old_path);
    has_error = true;
  } else {
    this->log(2, L"Location("+title+L") Rename", L"Definition file renamed to \""+new_file+L"\"");
  }

  // compose new Location home folder
  wstring new_home = old_home.substr(0, old_home.find_last_of(L"\\") + 1);
  new_home += name;

  #ifdef DEBUG
  std::wcout << "DEBUG => OmLocation::renameHome : " << new_home << L"\n";
  #endif

  // Rename Location home folder
  result = Om_fileMove(old_home, new_home);
  if(result != 0) {
    this->_error = Om_errRename(L"Home folder", old_home, result);
    this->log(0, L"Location("+title+L") Rename", this->_error);
    // get back the old home folder to restore Location properly
    new_home = old_home;
    has_error = true;
  } else {
    this->log(2, L"Location("+title+L") Rename", L"Home folder renamed to \""+new_home+L"\"");
  }

  // Reload location
  this->open(new_home + L"\\" + new_file);

  if(!has_error) {
    this->log(2, L"Location("+title+L") Rename", L"Success");
  }

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckHasData()
{
  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(_pkgLs[i]->hasBck()) return true;
  }
  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckPurge(Om_progressCb progress_cb, void* user_ptr)
{
  // checks for access to backup folder
  if(!this->bckDirAccess(true)) { //< check for read and write
    this->_error =  L"Backup folder \""+this->_bckDir+L"\" access error.";
    this->log(0, L"Location("+this->_title+L") Purge backups", this->_error);
    return false;
  }
  // checks for access to destination folder
  if(!this->dstDirAccess(true)) { //< check for read and write
    this->_error =  L"Destination folder \""+this->_dstDir+L"\" access error.";
    this->log(0, L"Location("+this->_title+L") Purge backups", this->_error);
    return false;
  }

  // get list of all installed packages
  vector<OmPackage*> pkg_ls; //< our select list
  for(size_t i = 0; i < this->_pkgLs.size(); ++i)
    if(this->_pkgLs[i]->hasBck())
      pkg_ls.push_back(this->_pkgLs[i]);

  // if no package installed, nothing to purge
  if(pkg_ls.empty())
    return true;

  // initialize progression
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = pkg_ls.size();
    progress_cur = 0;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0))
      return true;
  }

  // even if we uninstall all packages, we need to get a sorted list
  // so we prepare with all overlaps and dependencies checking
  vector<OmPackage*> over_ls;
  vector<OmPackage*> deps_ls;
  vector<OmPackage*> unin_ls;

  // prepare packages uninstall and backups restoration
  this->bckPrepareUnin(unin_ls, over_ls, deps_ls, pkg_ls);

  bool has_error = false;

  this->log(2, L"Location("+this->_title+L") Purge backups", L"Uninstalling "+to_wstring(unin_ls.size())+L" packages.");

  unsigned n = 0;

  // here we go for uninstall all packages
  for(size_t i = 0; i < unin_ls.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(unin_ls[i]->ident().c_str())))
        break;
    }

    // uninstall package
    if(unin_ls[i]->uninst()) {
      n++; //< increase counter
    } else {
      this->_error = L"Package \""+unin_ls[i]->ident()+L"\" uninstall failed: ";
      this->_error += unin_ls[i]->lastError();
      this->log(0, L"Location("+this->_title+L") Purge backups", this->_error);
      has_error = true;
    }

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }

  // refresh library
  this->libRefresh();

  this->log(2, L"Location("+this->_title+L") Purge backups", to_wstring(n)+L" backups successfully restored.");

  return !has_error;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckMove(const wstring& path, Om_progressCb progress_cb, void* user_ptr)
{
  if(path == this->_bckDir)
    return true;

  // verify backup folder access
  if(!this->bckDirAccess(true)) { //< check for read and write
    this->_error =  L"Backup folder \""+this->_bckDir+L"\" access error.";
    this->log(0, L"Location("+this->_title+L") Move backups", this->_error);
    return false;
  }

  // get content of backup folder
  vector<wstring> path_ls;
  Om_lsAll(&path_ls, this->_bckDir, false);

  // initialize progression
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = path_ls.size();
    progress_cur = 0;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0))
      return true;
  }

  bool has_error = false;

  this->log(2, L"Location("+this->_title+L") Purge backups", L"Moving "+to_wstring(path_ls.size())+L" elements.");

  unsigned n = 0;
  int result;
  wstring src, dst;

  for(size_t i = 0; i < path_ls.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(path_ls[i].c_str())))
        break;
    }

    // compose source and destination path
    src = this->_bckDir + L"\\" + path_ls[i];
    dst = path + L"\\" + path_ls[i];

    // move file
    result = Om_fileMove(src, dst);
    if(result != 0) {
      this->_error = Om_errMove(L"Backup data", src, result);
      this->log(0, L"Location("+this->_title+L") Move backups", this->_error);
      has_error = true;
    } else {
      n++; //< increase counter
    }

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }

  // full refresh library
  this->libClear();
  this->libRefresh();

  this->log(2, L"Location("+this->_title+L") Move backups", to_wstring(n)+L" elements successfully transfered.");

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckDcard(Om_progressCb progress_cb, void* user_ptr)
{
  // verify backup folder access
  if(!this->bckDirAccess(true)) { //< check for read and write
    this->_error =  L"Backup folder \""+this->_bckDir+L"\" access error.";
    this->log(0, L"Location("+this->_title+L") Move backups", this->_error);
    return false;
  }

  // gather all installed packages
  vector<OmPackage*> pkg_ls;
  for(size_t i = 0; i < this->_pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->hasBck())
      pkg_ls.push_back(this->_pkgLs[i]);
  }

  // check whether we have something to proceed
  if(pkg_ls.empty())
    return true;

  // initialize progression
  size_t progress_tot, progress_cur;
  if(progress_cb) {
    progress_tot = pkg_ls.size();
    progress_cur = 0;
    if(!progress_cb(user_ptr, progress_tot, progress_cur, 0))
      return true;
  }

  bool has_error = false;

  this->log(2, L"Location("+this->_title+L") Purge backups", L"Discarding "+to_wstring(pkg_ls.size())+L" backups.");

  unsigned n = 0;

  // Discard backup data for all packages
  for(size_t i = 0; i < pkg_ls.size(); ++i) {

    // call progression callback
    if(progress_cb) {
      progress_cur++;
      if(!progress_cb(user_ptr, progress_tot, progress_cur, reinterpret_cast<uint64_t>(pkg_ls[i]->ident().c_str())))
        break;
    }

    // Discard backup of this package
    if(pkg_ls[i]->unbackup()) {
      n++; //< increase counter
    } else {
      this->_error =  L"Backup data \""+pkg_ls[i]->name()+L"\"";
      this->_error += L" discard failed: "+pkg_ls[i]->lastError();
      this->log(0, L"Location("+this->_title+L") Discard backups", this->_error);
      has_error = true;
    }

    #ifdef DEBUG
    Sleep(OMM_DEBUG_SLOW); //< for debug
    #endif
  }

  // refresh library
  this->libRefresh();

  this->log(2, L"Location("+this->_title+L") Move backups", to_wstring(n)+L" backup successfully discarded.");

  return !has_error;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::pkgPrepareInst(vector<OmPackage*>& ins_ls, vector<OmPackage*>& ovr_ls, vector<OmPackage*>& dps_ls, vector<wstring>& mis_ls, const vector<OmPackage*>& sel_ls) const
{
  // gather dependencies and create missing lists
  vector<wstring> idt_ls;
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    idt_ls.clear();
    __src_get_dependencies(ins_ls, idt_ls, this->_pkgLs, sel_ls[i]);

    for(size_t j = 0; j < idt_ls.size(); ++j) {
      // add uniques only
      if(std::find(mis_ls.begin(), mis_ls.end(), idt_ls[j]) == mis_ls.end()) {
        mis_ls.push_back(idt_ls[j]);
      }
    }
  }

  // create the extra install list
  for(size_t i = 0; i < ins_ls.size(); ++i) {
    // add only if not in the initial selection
    if(std::find(sel_ls.begin(), sel_ls.end(), ins_ls[i]) == sel_ls.end()) {
      dps_ls.push_back(ins_ls[i]);
    }
  }

  // compose the final install list
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    // add only if not already in install list
    if(std::find(ins_ls.begin(), ins_ls.end(), sel_ls[i]) == ins_ls.end()) {
      ins_ls.push_back(sel_ls[i]);
    }
  }

  // get installation footprint of packages to be installed
  vector<OmPkgItem> footp;
  vector<vector<OmPkgItem>> footp_ls;

  // get overlaps list including simulated installation
  for(size_t i = 0; i < ins_ls.size(); ++i) {

    // test overlapping against installed packages
    for(size_t j = 0; j < this->_pkgLs.size(); ++j) {
      if(this->_pkgLs[j]->hasBck()) {
        if(ins_ls[i]->ovrTest(this->_pkgLs[j]))
          ovr_ls.push_back(this->_pkgLs[j]);
      }
    }

    // test overlapping against packages to be installed
    for(size_t j = 0; j < footp_ls.size(); ++j) {
      if(ins_ls[i]->ovrTest(footp_ls[j]))
        ovr_ls.push_back(ins_ls[j]);
    }

    // create installation footprint of package
    footp.clear();
    ins_ls[i]->footprint(footp);
    footp_ls.push_back(footp);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::bckPrepareUnin(vector<OmPackage*>& uns_ls, vector<OmPackage*>& ovr_ls,  vector<OmPackage*>& dpt_ls, const vector<OmPackage*>& sel_ls) const
{
  // get overlapping packages list to be uninstalled before selection
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // this is the only call we do, but the function is doubly recursive and
    // can lead to huge complexity depending the actual state of package installation
    // dependencies and overlapping...
    __bck_get_relations(uns_ls, ovr_ls, dpt_ls, this->_pkgLs, sel_ls[i]);
  }

  // compose the final uninstall list
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    // add only if not already in initial list
    if(std::find(uns_ls.begin(), uns_ls.end(), sel_ls[i]) == uns_ls.end()) {
      uns_ls.push_back(sel_ls[i]);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::bckPrepareClns(vector<OmPackage*>& cln_ls, vector<OmPackage*>& ovr_ls,  vector<OmPackage*>& dpt_ls, vector<OmPackage*>& dps_ls, const vector<OmPackage*>& sel_ls) const
{
  // get list of extra dependencies to clean uninstall
  for(size_t i = 0; i < sel_ls.size(); ++i) {
    __bck_get_dependencies(dps_ls, this->_pkgLs, sel_ls[i]);
  }

  // we compose final selection with extra dependencies
  vector<OmPackage*> mix_ls(sel_ls);

  // then add the extra dependencies
  for(size_t i = 0; i < dps_ls.size(); ++i) {
    // add only if not already in initial list
    if(std::find(mix_ls.begin(), mix_ls.end(), dps_ls[i]) == mix_ls.end()) {
      mix_ls.push_back(dps_ls[i]);
    }
  }

  // get overlapping packages list to be uninstalled before selection
  for(size_t i = 0; i < mix_ls.size(); ++i) {

    // this is the only call we do, but the function is doubly recursive and
    // can lead to huge complexity depending the actual state of package installation
    // dependencies and overlapping...
    __bck_get_relations(cln_ls, ovr_ls, dpt_ls, this->_pkgLs, mix_ls[i]);
  }

  // compose the final uninstall list
  for(size_t i = 0; i < mix_ls.size(); ++i) {
    // add only if not already in initial list
    if(std::find(cln_ls.begin(), cln_ls.end(), mix_ls[i]) == cln_ls.end()) {
      cln_ls.push_back(mix_ls[i]);
    }
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgFindOverlaps(vector<OmPackage*>& pkg_list, const OmPackage* pkg) const
{
  size_t n = 0;

  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->hasBck()) {
      if(pkg->ovrTest(this->_pkgLs[i])) {
        pkg_list.push_back(this->_pkgLs[i]);
        ++n;
      }
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgFindOverlaps(vector<uint64_t>& hash_list, const OmPackage* pkg) const
{
  size_t n = 0;

  for(size_t i = 0; i < _pkgLs.size(); ++i) {
    if(this->_pkgLs[i]->hasBck()) {
      if(pkg->ovrTest(this->_pkgLs[i])) {
        hash_list.push_back(this->_pkgLs[i]->hash());
        ++n;
      }
    }
  }

  return n;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::pkgGetDepends(vector<OmPackage*>& dep_ls, vector<wstring>& mis_ls, const OmPackage* pkg) const
{
  return __src_get_dependencies(dep_ls, mis_ls, this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::pkgChkDepends(const OmPackage* pkg) const
{
  return __src_chk_dependencies(this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::bckGetOverlaps(vector<OmPackage*>& ovr_ls, const OmPackage* pkg) const
{
  return __bck_get_overlaps(ovr_ls, this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::bckGetDependents(vector<OmPackage*>& dpt_ls, const OmPackage* pkg) const
{
  return __bck_get_dependents(dpt_ls, this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckChkDependents(const OmPackage* pkg) const
{
  return __bck_chk_dependents(this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::bckGetRelations(vector<OmPackage*>& rel_ls, vector<OmPackage*>& ovr_ls, vector<OmPackage*>& dep_ls, const OmPackage* pkg) const
{
  return __bck_get_relations(rel_ls, ovr_ls, dep_ls, this->_pkgLs, pkg);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::bckItemExists(const wstring& path, OmPkgItemDest dest) const
{
  for(size_t i = 0; i < _pkgLs.size(); ++i) {

    if(_pkgLs[i]->hasBck())
      if(_pkgLs[i]->bckItemHas(path, dest))
        return true;
  }

  return false;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::repAdd(const wstring& base, const wstring& name)
{
  if(this->_config.valid()) {

    if(!this->_config.xml().hasChild(L"network")) {
      this->_config.xml().addChild(L"network");
    }

    OmXmlNode xml_net = this->_config.xml().child(L"network");

    // check whether repository already exists
    vector<OmXmlNode> xml_rep_list;
    xml_net.children(xml_rep_list, L"repository");

    for(size_t i = 0; i < xml_rep_list.size(); ++i) {
      if(base == xml_rep_list[i].attrAsString(L"base") && name == xml_rep_list[i].attrAsString(L"name")) {
        this->_error = L"Repository with same parameters already exists";
        this->log(0, L"Location("+this->_title+L") Add Repository", this->_error);
        return false;
      }
    }

    // add repository entry in definition
    OmXmlNode xml_rep = xml_net.addChild(L"repository");
    xml_rep.setAttr(L"base", base);
    xml_rep.setAttr(L"name", name);

    this->_config.save();

    // add repository in local list
    OmRepository* pRep = new OmRepository(this);

    // set repository parameters
    if(!pRep->init(base, name)) {
      this->log(0, L"Location("+this->_title+L") Add Repository", pRep->lastError());
      delete pRep;
      return false;
    }

    // add to list
    this->_repLs.push_back(pRep);
  }

  return true;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::repRem(unsigned id)
{
  if(this->_config.valid()) {

    OmRepository* pRep = this->_repLs[id];

    // remove repository from definition
    if(this->_config.xml().hasChild(L"network")) {
      OmXmlNode xml_net = this->_config.xml().child(L"network");

      // check whether repository already exists
      vector<OmXmlNode> xml_rep_list;
      xml_net.children(xml_rep_list, L"repository");

      for(size_t i = 0; i < xml_rep_list.size(); ++i) {
        if(pRep->base() == xml_rep_list[i].attrAsString(L"base") &&
           pRep->name() == xml_rep_list[i].attrAsString(L"name")) {
          xml_net.remChild(xml_rep_list[i]);
          break;
        }
      }
    }

    this->_config.save();

    // delete object
    delete pRep;

    // remove from local list
    this->_repLs.erase(this->_repLs.begin()+id);
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::repQuery(unsigned i)
{
  // check whether we have something to proceed
  if(i >= this->_repLs.size())
    return false;

  bool has_error = false;

  this->log(2, L"Location("+this->_title+L") Query repository", this->_repLs[i]->url()+L"-"+this->_repLs[i]->name());

  if(!this->_repLs[i]->query()) {
    this->_error = L"Repository query failed: "+this->_repLs[i]->lastError();
    this->log(0, L"Location("+this->_title+L") Query repository", this->_error);
    has_error = true;
  }

  // add/merge remote packages to list
  unsigned c = this->_repLs[i]->rmtMerge(this->_rmtLs);
  this->log(2, L"Location("+this->_title+L") Query repository", to_wstring(c)+L" new remote packages merged.");

  // force refresh
  this->rmtRefresh(true);

  return !has_error;
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnExtraDnld(bool enable)
{
  this->_warnExtraDnld = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_net;

    if(this->_config.xml().hasChild(L"network")) {
      xml_net = this->_config.xml().child(L"network");
    } else {
      xml_net = this->_config.xml().addChild(L"network");
    }

    if(xml_net.hasChild(L"warn_extra_dnld")) {
      xml_net.child(L"warn_extra_dnld").setAttr(L"enable", this->_warnExtraDnld ? 1 : 0);
    } else {
      xml_net.addChild(L"warn_extra_dnld").setAttr(L"enable", this->_warnExtraDnld ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnMissDnld(bool enable)
{
  this->_warnMissDnld = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_net;

    if(this->_config.xml().hasChild(L"network")) {
      xml_net = this->_config.xml().child(L"network");
    } else {
      xml_net = this->_config.xml().addChild(L"network");
    }

    if(xml_net.hasChild(L"warn_miss_dnld")) {
      xml_net.child(L"warn_miss_dnld").setAttr(L"enable", this->_warnMissDnld ? 1 : 0);
    } else {
      xml_net.addChild(L"warn_miss_dnld").setAttr(L"enable", this->_warnMissDnld ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::setWarnUpgdBrkDeps(bool enable)
{
  this->_warnUpgdBrkDeps = enable;

  if(this->_config.valid()) {

    OmXmlNode xml_net;

    if(this->_config.xml().hasChild(L"network")) {
      xml_net = this->_config.xml().child(L"network");
    } else {
      xml_net = this->_config.xml().addChild(L"network");
    }

    if(xml_net.hasChild(L"warn_upgd_brk_deps")) {
      xml_net.child(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warnUpgdBrkDeps ? 1 : 0);
    } else {
      xml_net.addChild(L"warn_upgd_brk_deps").setAttr(L"enable", this->_warnUpgdBrkDeps ? 1 : 0);
    }

    this->_config.save();
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::rmtSetSorting(OmLocLsSort sorting)
{
  // we check if the requested sorting kind is the same as the currently
  // used, in this case, this mean the sorting order must be reversed

  if(sorting & this->_rmtSorting) {

    // check if current sorting is reversed, then switch order
    if(this->_rmtSorting & LS_SORT_REVERSE) {
      this->_rmtSorting &= ~LS_SORT_REVERSE; //< remove reversed flag
    } else {
      this->_rmtSorting |= LS_SORT_REVERSE; //< add reversed flag
    }

  } else {

    this->_rmtSorting = sorting;
  }

  // save the current sorting
  if(this->_config.valid()) {

    if(this->_config.xml().hasChild(L"remotes_sort")) {
      this->_config.xml().child(L"remotes_sort").setAttr(L"sort", static_cast<int>(this->_rmtSorting));
    } else {
      this->_config.xml().addChild(L"remotes_sort").setAttr(L"sort", static_cast<int>(this->_rmtSorting));
    }

    this->_config.save();
  }

  // finally sort packages
  this->_rmtSort();
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
bool OmLocation::rmtRefresh(bool force)
{
  bool changed = false;

  // refresh library first
  if(this->libRefresh() || force) {

    OmRemote* pRmt;
    OmPackage* pPkg;

    vector<OmRemote*> dep_ls;
    vector<wstring> mis_ls;

    unsigned new_state;

    // compare remote package list to define remote status
    for(size_t r = 0; r < this->_rmtLs.size(); ++r) {

      pRmt = this->_rmtLs[r];

      new_state = pRmt->_state;

      // remove relevant states, but not all of them to
      // keep states such as downloading or error
      new_state |=  RMT_STATE_NEW;
      new_state &= ~RMT_STATE_UPG;
      new_state &= ~RMT_STATE_OLD;
      new_state &= ~RMT_STATE_DEP;

      for(size_t p = 0; p < this->_pkgLs.size(); ++p) {

        pPkg = this->_pkgLs[p];

        // search for same core but different version
        if(pRmt->core() == pPkg->core()) {
          // check whether this identity matches
          if(pRmt->ident() != pPkg->ident()) {
            // check version changes
            if(pRmt->version() > pPkg->version()) {
              new_state |= RMT_STATE_UPG;
              // add superseded package
              pRmt->_supLs.push_back(pPkg);
            } else {
              new_state |= RMT_STATE_OLD;
            }
          } else {
            // same identity, package already exists in library
            new_state &= ~RMT_STATE_NEW; //< Remove the "NEW" state
            // check for missing dependencies
            if(!this->pkgChkDepends(pPkg)) {
              new_state |= RMT_STATE_DEP;
            }
          }
          continue;
        }
      }

      if(new_state != pRmt->_state) {
        pRmt->_state = new_state;
        changed = true;
      }
    }
  }

  if(changed || force)
    this->_rmtSort();

  #ifdef DEBUG
  std::cout << "DEBUG => OmLocation::rmtRefresh " << (changed ? "+-" : "==") << "\n";
  #endif

  return changed;
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::rmtPrepareDown(vector<OmRemote*>& dnl_ls, vector<OmRemote*>& dep_ls, vector<wstring>& mis_ls, vector<OmPackage*>& old_ls, const vector<OmRemote*>& sel_ls) const
{
  // gather dependencies and create missing lists
  vector<wstring> idt_ls;
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    idt_ls.clear();
    __rmt_get_dependencies(dnl_ls, idt_ls, this->_rmtLs, this->_pkgLs, sel_ls[i]);

    for(size_t j = 0; j < idt_ls.size(); ++j) {
      // add uniques only
      if(std::find(mis_ls.begin(), mis_ls.end(), idt_ls[j]) == mis_ls.end()) {
        mis_ls.push_back(idt_ls[j]);
      }
    }
  }

  // create the extra install list
  for(size_t i = 0; i < dnl_ls.size(); ++i) {

    // check for required old packages that this one may supersedes
    __rmt_get_old_required(old_ls, this->_pkgLs, dnl_ls[i]);

    // add only if not in the initial selection
    if(std::find(sel_ls.begin(), sel_ls.end(), dnl_ls[i]) == sel_ls.end()) {
      dep_ls.push_back(dnl_ls[i]);
    }
  }

  // compose the final install list
  for(size_t i = 0; i < sel_ls.size(); ++i) {

    // add only if not already in install list
    if(std::find(dnl_ls.begin(), dnl_ls.end(), sel_ls[i]) == dnl_ls.end()) {

      // check for required old packages that this one may supersedes
      __rmt_get_old_required(old_ls, this->_pkgLs, sel_ls[i]);

      dnl_ls.push_back(sel_ls[i]);
    }
  }
}

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
size_t OmLocation::rmtGetDepends(vector<OmRemote*>& dep_ls, vector<wstring>& mis_ls, const OmRemote* rmt) const
{
  return __rmt_get_dependencies(dep_ls, mis_ls, this->_rmtLs, this->_pkgLs, rmt);
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void OmLocation::log(unsigned level, const wstring& head, const wstring& detail)
{
  wstring log_str = L"Context("; log_str.append(this->_context->title());
  log_str.append(L"):: "); log_str.append(head);

  this->_context->log(level, log_str, detail);
}



///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
inline void OmLocation::_pkgSort()
{
 if(this->_pkgSorting & LS_SORT_STAT) //< sorting by status
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __pkg_sort_stat_fn);

  if(this->_pkgSorting & LS_SORT_NAME) //< sorting by name (alphabetical order)
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __pkg_sort_name_fn);

  if(this->_pkgSorting & LS_SORT_VERS) //< sorting by version (ascending)
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __pkg_sort_vers_fn);

  if(this->_pkgSorting & LS_SORT_CATG) //< sorting by version (ascending)
    std::sort(this->_pkgLs.begin(), this->_pkgLs.end(), __pkg_sort_catg_fn);

  // check whether we need a normal or reverse sorting
  if(this->_pkgSorting & LS_SORT_REVERSE) {
    std::reverse(this->_pkgLs.begin(), this->_pkgLs.end());
  }
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
inline void OmLocation::_rmtSort()
{
 if(this->_rmtSorting & LS_SORT_STAT) //< sorting by status
    std::sort(this->_rmtLs.begin(), this->_rmtLs.end(), __rmt_sort_stat_fn);

  if(this->_rmtSorting & LS_SORT_NAME) //< sorting by name (alphabetical order)
    std::sort(this->_rmtLs.begin(), this->_rmtLs.end(), __rmt_sort_name_fn);

  if(this->_rmtSorting & LS_SORT_VERS) //< sorting by version (ascending)
    std::sort(this->_rmtLs.begin(), this->_rmtLs.end(), __rmt_sort_vers_fn);

  if(this->_rmtSorting & LS_SORT_SIZE) //< sorting by version (ascending)
    std::sort(this->_rmtLs.begin(), this->_rmtLs.end(), __rmt_sort_size_fn);

  if(this->_rmtSorting & LS_SORT_CATG) //< sorting by version (ascending)
    std::sort(this->_rmtLs.begin(), this->_rmtLs.end(), __rmt_sort_catg_fn);

  // check whether we need a normal or reverse sorting
  if(this->_rmtSorting & LS_SORT_REVERSE) {
    std::reverse(this->_rmtLs.begin(), this->_rmtLs.end());
  }
}
