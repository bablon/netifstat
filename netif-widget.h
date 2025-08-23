#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define NETIF_TYPE_WIDGET	(netif_widget_get_type())

G_DECLARE_FINAL_TYPE(NetifWidget, netif_widget, NETIF, WIDGET, AdwBin)

G_END_DECLS
