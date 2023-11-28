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
#ifndef OMUTILSYS_H
#define OMUTILSYS_H

/// \brief Get current time.
///
/// Get current time values based on system local time.
///
/// \param[out] t_sec   : Pointer to receive seconds
/// \param[out] t_min   : Pointer to receive minute
/// \param[out] t_hour  : Pointer to receive hour
///
void Om_getTime(int *t_sec, int *t_min, int *t_hour);

/// \brief Get current date.
///
/// Get current date values based on system local time.
///
/// \param[out] t_day   : Pointer to receive day of month
/// \param[out] t_mon   : Pointer to receive month
/// \param[out] t_year  : Pointer to receive year
///
void Om_getDate(int *t_day, int *t_mon, int *t_year);

#endif // OMUTILSYS_H
