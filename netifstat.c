/*
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
#include "netif-widget.h"

GtkWidget *adw_win_new(GtkApplication *app, GtkWidget *content)
{
	GtkWidget *win = adw_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(win), "Network Interface Stats");

	GFile *file = g_file_new_for_uri("resource:///style.css");
	GtkStyleProvider *css_provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
	gtk_css_provider_load_from_file(GTK_CSS_PROVIDER(css_provider), file);
	gtk_style_context_add_provider_for_display(gdk_display_get_default(),
			css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	GtkIconTheme *theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
	gtk_icon_theme_add_resource_path(theme, "/icons/scalable");
	g_assert(gtk_icon_theme_has_icon(theme, "open-menu-symbolic"));
	g_assert(gtk_icon_theme_has_icon(theme, "theme-check-symbolic"));

	GtkWidget *headerbar = adw_header_bar_new();
	GtkWidget *menubutton = gtk_menu_button_new();
	adw_header_bar_pack_end(ADW_HEADER_BAR(headerbar), menubutton);
	gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menubutton), "open-menu-symbolic");

	GMenu *menu = g_menu_new();
	GMenuItem *item = g_menu_item_new(NULL, NULL);
	g_menu_item_set_attribute(item, "custom", "s", "theme-switcher");
	g_menu_append_item(menu, item);

	GMenu *section = g_menu_new();
	item = g_menu_item_new("Raw Bytes", "app.raw-bytes");
	g_menu_append_item(section, item);
	item = g_menu_item_new("Simple Mode", "app.simple-mode");
	g_menu_append_item(section, item);

	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));

	GtkWidget *popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
	GtkWidget *theme_switcher = g_object_new(KGX_TYPE_THEME_SWITCHER, NULL);
	gtk_popover_menu_add_child(GTK_POPOVER_MENU(popover), theme_switcher, "theme-switcher");

	AdwStyleManager *style_manager = adw_style_manager_get_default();
	g_object_bind_property(style_manager, "system-supports-color-schemes",
			theme_switcher, "show-system", G_BINDING_SYNC_CREATE);
	g_object_bind_property_full(theme_switcher, "theme",
			style_manager, "color-scheme", G_BINDING_SYNC_CREATE,
			theme_to_color_scheme, NULL, NULL, NULL);

	gtk_menu_button_set_popover(GTK_MENU_BUTTON(menubutton), popover);

	GtkWidget *toolbarview = adw_toolbar_view_new();

	adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(toolbarview), headerbar);
	adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(toolbarview), content);

	adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), toolbarview);

	return win;
}

static void on_activate(GtkApplication *app)
{
	GtkWidget *netif = g_object_new(NETIF_TYPE_WIDGET, NULL);

	GPropertyAction *action = g_property_action_new("raw-bytes", netif, "raw-bytes");
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(action));

	action = g_property_action_new("simple-mode", netif, "simple-mode");
	g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(action));

	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), netif);
	GtkWidget *window = adw_win_new(app, scrolled);
	gtk_window_set_default_size(GTK_WINDOW(window), 0, 400);

	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
	g_autoptr(AdwApplication) app = NULL;

	app = adw_application_new("cc.call.netifstat", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	return g_application_run(G_APPLICATION(app), argc, argv);
}
