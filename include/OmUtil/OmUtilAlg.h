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
#ifndef OMUTILALG_H
#define OMUTILALG_H

#include "OmBase.h"
#include <algorithm>

/// \brief Check whether array contain value
///
/// Check whether the given references array contain the specified value
///
/// \param[in] references : Array to search value in
/// \param[in] value      : Value to search
///
/// \return True if value found, false otherwise
///
template<typename T>
bool Om_arrayContain(const std::vector<T>& references, const T value)
{
  return (std::find(references.begin(), references.end(), value) != references.end());
}

/// \brief Add unique value
///
/// Add value to destination array only if not already in
/// destination array
///
/// \param[in] destination : Destination vector array
/// \param[in] value       : Unique value to add
///
template<typename T>
void Om_push_backUnique(std::vector<T>& destination, const T value)
{
  if(std::find(destination.begin(), destination.end(), value) == destination.end())
    destination.push_back(value);
}

/// \brief Add unique value
///
/// Add value to destination array only if not already in
/// destination array
///
/// \param[in] destination : Destination vector array
/// \param[in] value       : Unique value to add
///
template<typename T>
void Om_push_backUnique(std::deque<T>& destination, const T value)
{
  if(std::find(destination.begin(), destination.end(), value) == destination.end())
    destination.push_back(value);
}

/// \brief Add unique value
///
/// Add value to destination array only if not already in
/// references array
///
/// \param[in] destination  : Destination vector array
/// \param[in] references   : References vector array to search value
/// \param[in] value        : Unique value to add
///
template<typename T>
void Om_push_backUnique(std::vector<T>& destination, const std::vector<T>& references, const T value)
{
  if(std::find(references.begin(), references.end(), value) == references.end())
    destination.push_back(value);
}

/// \brief Merge array of uniques values
///
/// Add all source values to destination that are not already in destination.
///
/// \param[in] destination  : Destination vector array
/// \param[in] source       : Source vector array to merge
///
template<typename T>
void Om_mergeUniques(std::vector<T>& destination, std::vector<T>& source) {

  for(size_t i = 0; i < source.size(); ++i) {
    if(std::find(destination.begin(), destination.end(), source.at(i)) == destination.end())
      destination.push_back(source.at(i));
  }
}

/// \brief Erase value in array
///
/// Erase the first occurrence of the specified value in the given array
///
/// \param[in] array  : Array to remove value from
/// \param[in] value  : Value to search and erase in array
///
template<typename T>
void Om_eraseValue(std::vector<T>& array, const T value) {

  typename std::vector<T>::iterator pos;

  pos = std::find(array.begin(), array.end(), value);

  if(pos != array.end())
    array.erase(pos);
}

/// \brief Erase value in array
///
/// Erase the first occurrence of the specified value in the given array
///
/// \param[in] array  : Array to remove value from
/// \param[in] value  : Value to search and erase in array
///
template<typename T>
void Om_eraseValue(std::deque<T>& array, const T value) {

  typename std::deque<T>::iterator pos;

  pos = std::find(array.begin(), array.end(), value);

  if(pos != array.end())
    array.erase(pos);
}

#endif // OMUTILALG_H
