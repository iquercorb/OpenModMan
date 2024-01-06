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
#include "OmBase.h"           //< string, vector, Om_alloc, OM_MAX_PATH, etc.
#include <ctime>

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getTime(int *t_sec, int *t_min, int *t_hour)
{
  /* this is not thread-safe, localtime() returns pointer to global object
  time_t raw_time;
  time(&raw_time);
  struct tm* local_time = localtime(&raw_time);*/

  __time64_t long_time;
  _time64(&long_time);

  struct tm local_time;
  _localtime64_s(&local_time, &long_time);

  if(t_sec != nullptr) *t_sec = local_time.tm_sec;
  if(t_min != nullptr) *t_min = local_time.tm_min;
  if(t_hour != nullptr) *t_hour = local_time.tm_hour;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getDate(int *t_day, int *t_mon, int *t_year)
{
  /* this is not thread-safe, localtime() returns pointer to global object
  time_t raw_time;
  time(&raw_time);
  struct tm* local_time = localtime(&raw_time);*/

  __time64_t long_time;
  _time64(&long_time);

  struct tm local_time;
  _localtime64_s(&local_time, &long_time);

  if(t_day != nullptr) *t_day = local_time.tm_mday;
  if(t_mon != nullptr) *t_mon = local_time.tm_mon;
  if(t_year != nullptr) *t_year = local_time.tm_year;
}
