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

static time_t __time_rtime;
static struct tm* __time_ltime;

///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getTime(int *t_sec, int *t_min, int *t_hour)
{
  time(&__time_rtime);
  __time_ltime = localtime(&__time_rtime);

  if(t_sec != nullptr) *t_sec = __time_ltime->tm_sec;
  if(t_min != nullptr) *t_min = __time_ltime->tm_min;
  if(t_hour != nullptr) *t_hour = __time_ltime->tm_hour;
}


///
///  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
///
void Om_getDate(int *t_day, int *t_mon, int *t_year)
{
  time(&__time_rtime);
  __time_ltime = localtime(&__time_rtime);

  if(t_day != nullptr) *t_day = __time_ltime->tm_mday;
  if(t_mon != nullptr) *t_mon = __time_ltime->tm_mon;
  if(t_year != nullptr) *t_year = __time_ltime->tm_year;
}
