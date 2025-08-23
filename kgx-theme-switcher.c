/* kgx-theme-switcher.c
 *
 * Copyright 2021 Purism SPC
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

#include <adwaita.h>

#include "kgx-theme-switcher.h"

#define C_ENUM(v) ((gint) v)

GType
kgx_theme_get_type (void)
{
  static gsize gtype_id = 0;
  static const GEnumValue values[] = {
    { C_ENUM(KGX_THEME_AUTO), "KGX_THEME_AUTO", "auto" },
    { C_ENUM(KGX_THEME_NIGHT), "KGX_THEME_NIGHT", "night" },
    { C_ENUM(KGX_THEME_DAY), "KGX_THEME_DAY", "day" },
    { C_ENUM(KGX_THEME_HACKER), "KGX_THEME_HACKER", "hacker" },
    { 0, NULL, NULL }
  };

  if (g_once_init_enter (&gtype_id)) {
    GType new_type = g_enum_register_static (g_intern_static_string ("KgxTheme"), values);
      g_once_init_leave (&gtype_id, new_type);
  }

  return (GType) gtype_id;
}

gboolean
theme_to_color_scheme (GBinding     *binding,
                        const GValue *from_value,
                        GValue       *to_value,
                        gpointer      user_data)
{
  switch (g_value_get_enum (from_value)) {
    case KGX_THEME_AUTO:
      g_value_set_enum (to_value, ADW_COLOR_SCHEME_PREFER_LIGHT);
      break;
    case KGX_THEME_DAY:
      g_value_set_enum (to_value, ADW_COLOR_SCHEME_FORCE_LIGHT);
      break;
    case KGX_THEME_NIGHT:
    case KGX_THEME_HACKER:
    default:
      g_value_set_enum (to_value, ADW_COLOR_SCHEME_FORCE_DARK);
      break;
  }

  return TRUE;
}

struct _KgxThemeSwitcher {
  AdwBin     parent_instance;

  KgxTheme theme;

  GtkWidget *system_selector;
  GtkWidget *light_selector;
  GtkWidget *dark_selector;
  gboolean show_system;
};


G_DEFINE_TYPE (KgxThemeSwitcher, kgx_theme_switcher, ADW_TYPE_BIN)


enum {
  PROP_0,
  PROP_SHOW_SYSTEM,
  PROP_THEME,
  LAST_PROP
};
static GParamSpec *pspecs[LAST_PROP] = { NULL, };


static void
theme_check_active_changed (KgxThemeSwitcher *self)
{
  KgxTheme theme;

  if (gtk_check_button_get_active (GTK_CHECK_BUTTON (self->system_selector))) {
      theme = KGX_THEME_AUTO;
  } else if (gtk_check_button_get_active (GTK_CHECK_BUTTON (self->light_selector))) {
      theme = KGX_THEME_DAY;
  } else {
      theme = KGX_THEME_NIGHT;
  }

  if (theme == self->theme) {
    return;
  }

  self->theme = theme;
  g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_THEME]);
}


static void
set_theme (KgxThemeSwitcher *self,
           KgxTheme          theme)
{
  if (self->theme == theme)
    return;

  switch (theme) {
    case KGX_THEME_AUTO:
      gtk_check_button_set_active (GTK_CHECK_BUTTON (self->system_selector), TRUE);
      break;
    case KGX_THEME_DAY:
      gtk_check_button_set_active (GTK_CHECK_BUTTON (self->light_selector), TRUE);
      break;
    case KGX_THEME_NIGHT:
    case KGX_THEME_HACKER:
    default:
      gtk_check_button_set_active (GTK_CHECK_BUTTON (self->dark_selector), TRUE);
      break;
  }

  self->theme = theme;
  g_object_notify_by_pspec (G_OBJECT (self), pspecs[PROP_THEME]);
}


static void
kgx_theme_switcher_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  KgxThemeSwitcher *self = KGX_THEME_SWITCHER (object);

  switch (prop_id) {
    case PROP_THEME:
      g_value_set_enum (value, self->theme);
      break;
    case PROP_SHOW_SYSTEM:
      g_value_set_boolean (value, self->show_system);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static inline gboolean
kgx_set_boolean_prop (GObject      *restrict object,
                      GParamSpec   *restrict pspec,
                      gboolean     *restrict target,
                      const GValue *restrict value)
{
  gboolean new_value = g_value_get_boolean (value);

  if (new_value == *target) {
    return FALSE;
  }

  *target = new_value;
  g_object_notify_by_pspec (object, pspec);

  return TRUE;
}

static void
kgx_theme_switcher_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  KgxThemeSwitcher *self = KGX_THEME_SWITCHER (object);

  switch (prop_id) {
    case PROP_THEME:
      set_theme (self, g_value_get_enum (value));
      break;
    case PROP_SHOW_SYSTEM:
      kgx_set_boolean_prop (object, pspec, &self->show_system, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}


static void
kgx_theme_switcher_class_init (KgxThemeSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = kgx_theme_switcher_get_property;
  object_class->set_property = kgx_theme_switcher_set_property;

  pspecs[PROP_THEME] =
    g_param_spec_enum ("theme", NULL, NULL,
                       KGX_TYPE_THEME,
                       KGX_THEME_NIGHT,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  pspecs[PROP_SHOW_SYSTEM] =
    g_param_spec_boolean ("show-system", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, pspecs);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/kgx-theme-switcher.ui");

  gtk_widget_class_bind_template_child (widget_class, KgxThemeSwitcher, system_selector);
  gtk_widget_class_bind_template_child (widget_class, KgxThemeSwitcher, light_selector);
  gtk_widget_class_bind_template_child (widget_class, KgxThemeSwitcher, dark_selector);

  gtk_widget_class_bind_template_callback (widget_class, theme_check_active_changed);

  gtk_widget_class_set_css_name (widget_class, "themeswitcher");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}


static void
kgx_theme_switcher_init (KgxThemeSwitcher *self)
{
  self->show_system = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));
}
