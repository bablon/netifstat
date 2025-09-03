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

#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include <inttypes.h>

#include <glib-unix.h>

#include "netif-widget.h"

#define NETIF_TYPE_LINK_STATS	(netif_link_stats_get_type())
G_DECLARE_FINAL_TYPE(NetifLinkStats, netif_link_stats, NETIF, LINK_STATS, GObject)

struct _NetifLinkStats {
	GObject base;

	guint ifindex;
	char *ifname;

	guint64 rx_packets;
	guint64 tx_packets;
	guint64 rx_bytes;
	guint64 tx_bytes;

	guint64 rx_rate;
	guint64 tx_rate;
};

G_DEFINE_FINAL_TYPE(NetifLinkStats, netif_link_stats, G_TYPE_OBJECT)

enum {
	PROP_0,
	PROP_IFINDEX,
	PROP_IFNAME,
	PROP_RX_PACKETS,
	PROP_TX_PACKETS,
	PROP_RX_BYTES,
	PROP_TX_BYTES,
	PROP_RX_RATE,
	PROP_TX_RATE,
};

static void netif_link_stats_get_property(GObject *object,
		guint prop_id, GValue *value, GParamSpec *spec)
{
	NetifLinkStats *self = NETIF_LINK_STATS(object);

	switch (prop_id) {
	case PROP_RX_BYTES:
		g_value_set_uint64(value, self->rx_bytes);
		break;
	case PROP_TX_BYTES:
		g_value_set_uint64(value, self->tx_bytes);
		break;
	case PROP_RX_PACKETS:
		g_value_set_uint64(value, self->rx_packets);
		break;
	case PROP_TX_PACKETS:
		g_value_set_uint64(value, self->tx_packets);
		break;
	case PROP_IFINDEX:
		g_value_set_uint(value, self->ifindex);
		break;
	case PROP_IFNAME:
		g_value_set_string(value, self->ifname);
		break;
	case PROP_RX_RATE:
		g_value_set_uint64(value, self->rx_rate);
		break;
	case PROP_TX_RATE:
		g_value_set_uint64(value, self->tx_rate);
		break;
	}
}

static void netif_link_stats_set_property(GObject *object,
		guint prop_id, const GValue *value, GParamSpec *spec)
{
	NetifLinkStats *self = NETIF_LINK_STATS(object);

	switch (prop_id) {
	case PROP_RX_BYTES:
		self->rx_bytes = g_value_get_uint64(value);
		break;
	case PROP_TX_BYTES:
		self->tx_bytes = g_value_get_uint64(value);
		break;
	case PROP_RX_PACKETS:
		self->rx_packets = g_value_get_uint64(value);
		break;
	case PROP_TX_PACKETS:
		self->tx_packets = g_value_get_uint64(value);
		break;
	case PROP_IFINDEX:
		self->ifindex = g_value_get_uint(value);
		break;
	case PROP_IFNAME:
		g_free(self->ifname);
		self->ifname = g_strdup(g_value_get_string(value));
		break;
	case PROP_RX_RATE:
		self->rx_rate = g_value_get_uint64(value);
		break;
	case PROP_TX_RATE:
		self->tx_rate = g_value_get_uint64(value);
		break;
	}
}

static void netif_link_stats_dispose(GObject *object)
{
	NetifLinkStats *self = NETIF_LINK_STATS(object);

	g_free(self->ifname);
}

static void netif_link_stats_class_init(NetifLinkStatsClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->get_property = netif_link_stats_get_property;
	object_class->set_property = netif_link_stats_set_property;
	object_class->dispose = netif_link_stats_dispose;

	g_object_class_install_property(object_class, PROP_IFINDEX,
			g_param_spec_uint("ifindex", "ifindex", "interface index",
				0, G_MAXUINT, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_IFNAME,
			g_param_spec_string("ifname", "ifname", "interface name",
				NULL,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_RX_BYTES,
			g_param_spec_uint64("rx-bytes", "rx bytes", "rx bytes",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_TX_BYTES,
			g_param_spec_uint64("tx-bytes", "tx bytes", "tx bytes",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_RX_PACKETS,
			g_param_spec_uint64("rx-packets", "rx packets", "rx packets",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_TX_PACKETS,
			g_param_spec_uint64("tx-packets", "tx packets", "tx packets",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_RX_RATE,
			g_param_spec_uint64("rx-rate", "rx rate", "rx rate",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_TX_RATE,
			g_param_spec_uint64("tx-rate", "tx rate", "tx rate",
				0, G_MAXUINT64, 0,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void netif_link_stats_init(NetifLinkStats *self)
{

}

struct _NetifWidget {
	AdwBin base;

	GListStore *netif_store;
	GHashTable *netif_ht;

	struct nl_sock *nlsock;
	struct nl_msg *nlmsg;
	struct nl_cb *nlcb;

	int nl_timeout_id;

	struct nl_sock *rtnl_sock;
	int rtnl_id;

	bool raw_bytes;
};

enum {
	PROP_RAW_BYTES = 1,
};

G_DEFINE_FINAL_TYPE(NetifWidget, netif_widget, ADW_TYPE_BIN)

static gboolean netlink_send_func(gpointer data)
{
	NetifWidget *self = data;
	struct nlmsghdr *nlmsghdr = nlmsg_hdr(self->nlmsg);

	nlmsghdr->nlmsg_seq = NL_AUTO_SEQ;
	int err = nl_send_auto(self->nlsock, self->nlmsg);
	if (err < 0)
		g_warning("nl_send_auto error %d\n", err);

	nl_recvmsgs(self->nlsock, self->nlcb);

	return G_SOURCE_CONTINUE;
}

static int netlink_msg_handler(struct nl_msg *msg, void *arg)
{
	struct rtattr *tb[IFLA_STATS_MAX + 1];
	struct nlmsghdr *nlmsghdr = nlmsg_hdr(msg);
	struct rtattr *rta;
	int rta_len;
	struct rtnl_link_stats64 *stats;
	struct if_stats_msg *stats_msg = nlmsg_data(nlmsghdr);
	char ifname[IF_NAMESIZE];
	NetifWidget *self = arg;

	if (nlmsghdr->nlmsg_type != RTM_NEWSTATS)
		g_warning("%s: received type %d, not %d", __func__,
				nlmsghdr->nlmsg_type, RTM_NEWSTATS);

	memset(tb, 0, sizeof(tb));
	rta = (void *)nlmsghdr + NLMSG_SPACE(sizeof(struct if_stats_msg));
	rta_len = NLMSG_PAYLOAD(nlmsghdr, sizeof(struct if_stats_msg));

	while (RTA_OK(rta, rta_len)) {
		unsigned short type = rta->rta_type;

		if (type <= IFLA_STATS_MAX && !tb[type])
			tb[type] = rta;

		rta = RTA_NEXT(rta, rta_len);
	}

	g_assert(tb[IFLA_STATS_LINK_64]);
	g_assert(tb[IFLA_STATS_LINK_64]->rta_len == RTA_LENGTH(sizeof(*stats)));
	stats = RTA_DATA(tb[IFLA_STATS_LINK_64]);

	NetifLinkStats *netif = g_hash_table_lookup(self->netif_ht,
			GUINT_TO_POINTER(stats_msg->ifindex));

	if (!netif) {
		netif = g_object_new(NETIF_TYPE_LINK_STATS,
				"ifindex", stats_msg->ifindex,
				"ifname", if_indextoname(stats_msg->ifindex, ifname),
				"rx-bytes", stats->rx_bytes,
				"tx-bytes", stats->tx_bytes,
				"rx-packets", stats->rx_packets,
				"tx-packets", stats->tx_packets,
				NULL);
		g_hash_table_insert(self->netif_ht, GUINT_TO_POINTER(stats_msg->ifindex), netif);
		g_list_store_append(self->netif_store, netif);
	} else {
		g_object_set(G_OBJECT(netif),
				"ifindex", stats_msg->ifindex,
				"ifname", if_indextoname(stats_msg->ifindex, ifname),
				"rx-bytes", stats->rx_bytes,
				"tx-bytes", stats->tx_bytes,
				"rx-packets", stats->rx_packets,
				"tx-packets", stats->tx_packets,
				"rx-rate", stats->rx_bytes - netif->rx_bytes,
				"tx-rate", stats->tx_bytes - netif->tx_bytes,
				NULL);
	}

	return NL_OK;
}

static int rtnl_recv(struct nl_msg *msg, void *arg)
{
	NetifWidget *self = arg;

	struct nlmsghdr *hdr = nlmsg_hdr(msg);

	if (hdr->nlmsg_type == RTM_DELLINK) {
		struct ifinfomsg *ifmsg = nlmsg_data(hdr);

		NetifLinkStats *link = g_hash_table_lookup(self->netif_ht,
				GUINT_TO_POINTER(ifmsg->ifi_index));

		if (link) {
			guint pos;

			if (g_list_store_find(self->netif_store, link, &pos))
				g_list_store_remove(self->netif_store, pos);
		}
	}

	return NL_OK;
}

static int rtnl_recv_func(gint fd, GIOCondition cond, gpointer data)
{
	NetifWidget *self = data;

	nl_recvmsgs_default(self->rtnl_sock);

	return G_SOURCE_CONTINUE;
}

static int netif_widget_netlink_init(NetifWidget *self)
{
	struct nlmsghdr *nlmsghdr;
	struct if_stats_msg *stats_msg;

	self->nlcb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!self->nlcb)
		return -ENOMEM;

	nl_cb_set(self->nlcb, NL_CB_VALID, NL_CB_CUSTOM, netlink_msg_handler, self);

	self->nlsock = nl_socket_alloc_cb(self->nlcb);
	if (!self->nlsock) {
		nl_cb_put(self->nlcb);
		return -ENOMEM;
	}

	g_assert(nl_connect(self->nlsock, NETLINK_ROUTE) == 0);
	nl_socket_set_nonblocking(self->nlsock);
	nl_socket_set_peer_port(self->nlsock, 0);
	nl_socket_set_peer_groups(self->nlsock, 0);

	self->nlmsg = nlmsg_alloc();
	if (!self->nlmsg) {
		nl_close(self->nlsock);
		nl_socket_free(self->nlsock);
		nl_cb_put(self->nlcb);
		return -ENOMEM;
	}

	nlmsghdr = nlmsg_put(self->nlmsg, NL_AUTO_PID, NL_AUTO_SEQ, RTM_GETSTATS,
			sizeof(struct if_stats_msg), NLM_F_REQUEST | NLM_F_DUMP);
	stats_msg = nlmsg_data(nlmsghdr);

	memset(stats_msg, 0, sizeof(*stats_msg));
	stats_msg->family = AF_INET;
	stats_msg->filter_mask = IFLA_STATS_FILTER_BIT(IFLA_STATS_LINK_64);

	int err = nl_send_auto(self->nlsock, self->nlmsg);
	if (err < 0)
		g_warning("nl_send_auto error %d\n", err);

	nl_recvmsgs(self->nlsock, self->nlcb);

	self->nl_timeout_id = g_timeout_add_seconds(1, netlink_send_func, self);

	self->rtnl_sock = nl_socket_alloc();
	if (self->rtnl_sock) {
		g_assert(nl_connect(self->rtnl_sock, NETLINK_ROUTE) == 0);

		g_assert(nl_socket_add_membership(self->rtnl_sock, RTNLGRP_LINK) == 0);

		nl_socket_disable_seq_check(self->rtnl_sock);
		struct nl_cb *rtnl_cb = nl_socket_get_cb(self->rtnl_sock);
		nl_cb_set(rtnl_cb, NL_CB_VALID, NL_CB_CUSTOM, rtnl_recv, self);
		nl_cb_put(rtnl_cb);

		self->rtnl_id = g_unix_fd_add(nl_socket_get_fd(self->rtnl_sock),
					G_IO_IN, rtnl_recv_func, self);
	}

	return 0;
}

static void netif_widget_netlink_exit(NetifWidget *self)
{
	g_source_remove(self->rtnl_id);
	nl_close(self->rtnl_sock);
	nl_socket_free(self->rtnl_sock);

	g_source_remove(self->nl_timeout_id);

	nlmsg_free(self->nlmsg);
	nl_close(self->nlsock);
	nl_socket_free(self->nlsock);
	nl_cb_put(self->nlcb);
}

static void netif_widget_dispose(GObject *object)
{
	NetifWidget *self = NETIF_WIDGET(object);

	netif_widget_netlink_exit(self);
	g_hash_table_destroy(self->netif_ht);
	g_object_unref(self->netif_store);

	G_OBJECT_CLASS(netif_widget_parent_class)->dispose(object);
}

static void index_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_list_item_set_child(list_item, label);

	gtk_expression_bind(
			gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
				gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
					NULL, "item"),
				"ifindex"),
			label, "label", list_item);
}

static void name_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_list_item_set_child(list_item, label);

	gtk_expression_bind(
			gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
				gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
					NULL, "item"),
				"ifname"),
			label, "label", list_item);
}

static char *bytes_calc_func(GtkListItem *item, guint64 rate, NetifWidget *netif)
{
	char buf[128];

	if (rate == 0) {
		strcpy(buf, "0");
	} else if (netif->raw_bytes) {
		snprintf(buf, sizeof(buf), "%"PRIu64, rate);
	} else if (rate < 1024) {
		snprintf(buf, sizeof(buf), "%"PRIu64, rate);
	} else if (rate < 1024 * 1024) {
		snprintf(buf, sizeof(buf), "%.1lf KiB", (double)rate / 1024.0);
	} else if (rate < 1024 * 1024 * 1024) {
		snprintf(buf, sizeof(buf), "%.2lf MiB",
				(double)rate / (1024.0 * 1024.0));
	} else {
		snprintf(buf, sizeof(buf), "%.2lf GiB",
				(double)rate / (1024.0 * 1024.0 * 1024.0));
	}

	return g_strdup(buf);
}


static void rx_bytes_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 90, 0);
	gtk_list_item_set_child(list_item, label);

	GtkExpression *expr[2] = {
		gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
			gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
				NULL, "item"),
			"rx-bytes"),
		gtk_object_expression_new(G_OBJECT(data))
	};

	gtk_expression_bind(
			gtk_cclosure_expression_new(G_TYPE_STRING,
				NULL, 2, expr, G_CALLBACK(bytes_calc_func), NULL, NULL),
			label, "label", list_item);
}

static void tx_bytes_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 90, 0);
	gtk_list_item_set_child(list_item, label);

	GtkExpression *expr[2] = {
		gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
			gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
				NULL, "item"),
			"tx-bytes"),
		gtk_object_expression_new(G_OBJECT(data))
	};

	gtk_expression_bind(
			gtk_cclosure_expression_new(G_TYPE_STRING,
				NULL, 2, expr, G_CALLBACK(bytes_calc_func), NULL, NULL),
			label, "label", list_item);
}

static void rx_packets_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 70, 0);
	gtk_list_item_set_child(list_item, label);

	gtk_expression_bind(
			gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
				gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
					NULL, "item"),
				"rx-packets"),
			label, "label", list_item);
}

static void tx_packets_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 70, 0);
	gtk_list_item_set_child(list_item, label);

	gtk_expression_bind(
			gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
				gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
					NULL, "item"),
				"tx-packets"),
			label, "label", list_item);
}

static char *rate_calc_func(GtkListItem *item, guint64 rate, NetifWidget *netif)
{
	char buf[128];

	if (rate == 0) {
		strcpy(buf, "0");
	} else if (netif->raw_bytes) {
		snprintf(buf, sizeof(buf), "%"PRIu64, rate);
	} else if (rate < 1024) {
		snprintf(buf, sizeof(buf), "%"PRIu64" Bytes/s", rate);
	} else if (rate < 1024 * 1024) {
		snprintf(buf, sizeof(buf), "%.1lf KiB/s", (double)rate / 1024.0);
	} else if (rate < 1024 * 1024 * 1024) {
		snprintf(buf, sizeof(buf), "%.2lf MiB/s",
				(double)rate / (1024.0 * 1024.0));
	} else {
		snprintf(buf, sizeof(buf), "%.2lf GiB/s",
				(double)rate / (1024.0 * 1024.0 * 1024.0));
	}

	return g_strdup(buf);
}

static void rx_rate_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 100, 0);
	gtk_list_item_set_child(list_item, label);

	GtkExpression *expr[2] = {
		gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
			gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
				NULL, "item"),
			"rx-rate"),
		gtk_object_expression_new(G_OBJECT(data))
	};

	gtk_expression_bind(
			gtk_cclosure_expression_new(G_TYPE_STRING,
				NULL, 2, expr, G_CALLBACK(rate_calc_func), NULL, NULL),
			label, "label", list_item);
}

static void tx_rate_setup_func(GtkSignalListItemFactory *self,
		GtkListItem *list_item, gpointer data)
{
	GtkWidget *label = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_widget_set_size_request(GTK_WIDGET(label), 100, 0);
	gtk_list_item_set_child(list_item, label);

	GtkExpression *expr[2] = {
		gtk_property_expression_new(NETIF_TYPE_LINK_STATS,
			gtk_property_expression_new(GTK_TYPE_LIST_ITEM,
				NULL, "item"),
			"tx-rate"),
		gtk_object_expression_new(G_OBJECT(data))
	};

	gtk_expression_bind(
			gtk_cclosure_expression_new(G_TYPE_STRING,
				NULL, 2, expr, G_CALLBACK(rate_calc_func), NULL, NULL),
			label, "label", list_item);
}

static void netif_widget_constructed(GObject *object)
{
	NetifWidget *self = NETIF_WIDGET(object);

	GtkWidget *columnview = gtk_column_view_new(NULL);
	GtkNoSelection *selection = gtk_no_selection_new(G_LIST_MODEL(self->netif_store));
	gtk_column_view_set_model(GTK_COLUMN_VIEW(columnview), GTK_SELECTION_MODEL(selection));

	GtkListItemFactory *name_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *name_column = gtk_column_view_column_new("Name", name_factory);

	GtkListItemFactory *index_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *index_column = gtk_column_view_column_new("Index", index_factory);

	GtkListItemFactory *rx_bytes_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *rx_bytes_column = gtk_column_view_column_new("RxBytes", rx_bytes_factory);

	GtkListItemFactory *tx_bytes_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *tx_bytes_column = gtk_column_view_column_new("TxBytes", tx_bytes_factory);

	GtkListItemFactory *rx_packets_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *rx_packets_column = gtk_column_view_column_new("RxPackets", rx_packets_factory);

	GtkListItemFactory *tx_packets_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *tx_packets_column = gtk_column_view_column_new("TxPackets", tx_packets_factory);

	GtkListItemFactory *rx_rate_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *rx_rate_column = gtk_column_view_column_new("RxRate", rx_rate_factory);

	GtkListItemFactory *tx_rate_factory = gtk_signal_list_item_factory_new();
	GtkColumnViewColumn *tx_rate_column = gtk_column_view_column_new("TxRate", tx_rate_factory);

	g_signal_connect(name_factory, "setup", G_CALLBACK(name_setup_func), NULL);
	g_signal_connect(index_factory, "setup", G_CALLBACK(index_setup_func), NULL);
	g_signal_connect(rx_bytes_factory, "setup", G_CALLBACK(rx_bytes_setup_func), self);
	g_signal_connect(tx_bytes_factory, "setup", G_CALLBACK(tx_bytes_setup_func), self);
	g_signal_connect(rx_packets_factory, "setup", G_CALLBACK(rx_packets_setup_func), NULL);
	g_signal_connect(tx_packets_factory, "setup", G_CALLBACK(tx_packets_setup_func), NULL);
	g_signal_connect(rx_rate_factory, "setup", G_CALLBACK(rx_rate_setup_func), self);
	g_signal_connect(tx_rate_factory, "setup", G_CALLBACK(tx_rate_setup_func), self);

	gtk_column_view_column_set_expand(name_column, TRUE);
	gtk_column_view_column_set_expand(index_column, TRUE);
	gtk_column_view_column_set_expand(rx_bytes_column, TRUE);
	gtk_column_view_column_set_expand(tx_bytes_column, TRUE);
	gtk_column_view_column_set_expand(rx_packets_column, TRUE);
	gtk_column_view_column_set_expand(tx_packets_column, TRUE);
	gtk_column_view_column_set_expand(rx_rate_column, TRUE);
	gtk_column_view_column_set_expand(tx_rate_column, TRUE);

#if 0
	gtk_column_view_column_set_visible(index_column, FALSE);
	gtk_column_view_column_set_visible(rx_packets_column, FALSE);
	gtk_column_view_column_set_visible(tx_packets_column, FALSE);
#endif
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), name_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), index_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), rx_bytes_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), tx_bytes_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), rx_packets_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), tx_packets_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), rx_rate_column);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(columnview), tx_rate_column);

	adw_bin_set_child(ADW_BIN(self), columnview);
}

static void netif_widget_get_property(GObject *object,
		guint prop_id, GValue *value, GParamSpec *spec)
{
	NetifWidget *self = NETIF_WIDGET(object);

	switch (prop_id) {
	case PROP_RAW_BYTES:
		g_value_set_boolean(value, self->raw_bytes);
		break;
	}
}

static void netif_widget_set_property(GObject *object,
		guint prop_id, const GValue *value, GParamSpec *spec)
{
	NetifWidget *self = NETIF_WIDGET(object);

	switch (prop_id) {
	case PROP_RAW_BYTES:
		self->raw_bytes = g_value_get_boolean(value);
		break;
	}
}

static void netif_widget_class_init(NetifWidgetClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS(class);

	object_class->dispose = netif_widget_dispose;
	object_class->constructed= netif_widget_constructed;

	object_class->get_property = netif_widget_get_property;
	object_class->set_property = netif_widget_set_property;

	g_object_class_install_property(object_class, PROP_RAW_BYTES,
			g_param_spec_boolean("raw-bytes", "raw bytes", "raw bytes",
				FALSE,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void netif_widget_init(NetifWidget *self)
{
	self->netif_ht = g_hash_table_new(g_direct_hash, g_direct_equal);
	self->netif_store = g_list_store_new(NETIF_TYPE_LINK_STATS);
	g_object_ref(self->netif_store);

	g_assert(netif_widget_netlink_init(self) == 0);
}
