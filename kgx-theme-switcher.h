/* kgx-theme-switcher.h
 *
 * Copyright 2021 Purism SPC
 * Copyright 2023 Zander Brown
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

typedef enum /*< enum,prefix=KGX >*/ {
  KGX_THEME_AUTO = 0,   /*< nick=auto >*/
  KGX_THEME_NIGHT = 1,  /*< nick=night >*/
  KGX_THEME_DAY = 2,    /*< nick=day >*/
  KGX_THEME_HACKER = 3, /*< nick=hacker >*/
} KgxTheme;

gboolean
theme_to_color_scheme (GBinding     *binding,
                        const GValue *from_value,
                        GValue       *to_value,
                        gpointer      user_data);

GType kgx_theme_get_type (void);
#define KGX_TYPE_THEME (kgx_theme_get_type())

#define KGX_TYPE_THEME_SWITCHER (kgx_theme_switcher_get_type ())

G_DECLARE_FINAL_TYPE (KgxThemeSwitcher, kgx_theme_switcher, KGX, THEME_SWITCHER, AdwBin)


G_END_DECLS
