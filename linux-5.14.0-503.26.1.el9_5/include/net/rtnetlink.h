/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __NET_RTNETLINK_H
#define __NET_RTNETLINK_H

#include <linux/rtnetlink.h>
#include <net/netlink.h>

#include <linux/rh_kabi.h>

typedef int (*rtnl_doit_func)(struct sk_buff *, struct nlmsghdr *,
			      struct netlink_ext_ack *);
typedef int (*rtnl_dumpit_func)(struct sk_buff *, struct netlink_callback *);

enum rtnl_link_flags {
	RTNL_FLAG_DOIT_UNLOCKED		= BIT(0),
	RTNL_FLAG_BULK_DEL_SUPPORTED	= BIT(1),
};

enum rtnl_kinds {
	RTNL_KIND_NEW,
	RTNL_KIND_DEL,
	RTNL_KIND_GET,
	RTNL_KIND_SET
};
#define RTNL_KIND_MASK 0x3

static inline enum rtnl_kinds rtnl_msgtype_kind(int msgtype)
{
	return msgtype & RTNL_KIND_MASK;
}

void rtnl_register(int protocol, int msgtype,
		   rtnl_doit_func, rtnl_dumpit_func, unsigned int flags);
int rtnl_register_module(struct module *owner, int protocol, int msgtype,
			 rtnl_doit_func, rtnl_dumpit_func, unsigned int flags);
int rtnl_unregister(int protocol, int msgtype);
void rtnl_unregister_all(int protocol);

static inline int rtnl_msg_family(const struct nlmsghdr *nlh)
{
	if (nlmsg_len(nlh) >= sizeof(struct rtgenmsg))
		return ((struct rtgenmsg *) nlmsg_data(nlh))->rtgen_family;
	else
		return AF_UNSPEC;
}

/**
 *	struct rtnl_link_ops - rtnetlink link operations
 *
 *	@list: Used internally
 *	@kind: Identifier
 *	@netns_refund: Physical device, move to init_net on netns exit
 *	@maxtype: Highest device specific netlink attribute number
 *	@policy: Netlink policy for device specific attribute validation
 *	@validate: Optional validation function for netlink/changelink parameters
 *	@alloc: netdev allocation function, can be %NULL and is then used
 *		in place of alloc_netdev_mqs(), in this case @priv_size
 *		and @setup are unused. Returns a netdev or ERR_PTR().
 *	@priv_size: sizeof net_device private space
 *	@setup: net_device setup function
 *	@newlink: Function for configuring and registering a new device
 *	@changelink: Function for changing parameters of an existing device
 *	@dellink: Function to remove a device
 *	@get_size: Function to calculate required room for dumping device
 *		   specific netlink attributes
 *	@fill_info: Function to dump device specific netlink attributes
 *	@get_xstats_size: Function to calculate required room for dumping device
 *			  specific statistics
 *	@fill_xstats: Function to dump device specific statistics
 *	@get_num_tx_queues: Function to determine number of transmit queues
 *			    to create when creating a new device.
 *	@get_num_rx_queues: Function to determine number of receive queues
 *			    to create when creating a new device.
 *	@get_link_net: Function to get the i/o netns of the device
 *	@get_linkxstats_size: Function to calculate the required room for
 *			      dumping device-specific extended link stats
 *	@fill_linkxstats: Function to dump device-specific extended link stats
 */
struct rtnl_link_ops {
	struct list_head	list;

	const char		*kind;

	size_t			priv_size;
	struct net_device	*(*alloc)(struct nlattr *tb[],
					  const char *ifname,
					  unsigned char name_assign_type,
					  unsigned int num_tx_queues,
					  unsigned int num_rx_queues);
	void			(*setup)(struct net_device *dev);

	bool			netns_refund;
	unsigned int		maxtype;
	const struct nla_policy	*policy;
	int			(*validate)(struct nlattr *tb[],
					    struct nlattr *data[],
					    struct netlink_ext_ack *extack);

	int			(*newlink)(struct net *src_net,
					   struct net_device *dev,
					   struct nlattr *tb[],
					   struct nlattr *data[],
					   struct netlink_ext_ack *extack);
	int			(*changelink)(struct net_device *dev,
					      struct nlattr *tb[],
					      struct nlattr *data[],
					      struct netlink_ext_ack *extack);
	void			(*dellink)(struct net_device *dev,
					   struct list_head *head);

	size_t			(*get_size)(const struct net_device *dev);
	int			(*fill_info)(struct sk_buff *skb,
					     const struct net_device *dev);

	size_t			(*get_xstats_size)(const struct net_device *dev);
	int			(*fill_xstats)(struct sk_buff *skb,
					       const struct net_device *dev);
	unsigned int		(*get_num_tx_queues)(void);
	unsigned int		(*get_num_rx_queues)(void);

	unsigned int		slave_maxtype;
	const struct nla_policy	*slave_policy;
	int			(*slave_changelink)(struct net_device *dev,
						    struct net_device *slave_dev,
						    struct nlattr *tb[],
						    struct nlattr *data[],
						    struct netlink_ext_ack *extack);
	size_t			(*get_slave_size)(const struct net_device *dev,
						  const struct net_device *slave_dev);
	int			(*fill_slave_info)(struct sk_buff *skb,
						   const struct net_device *dev,
						   const struct net_device *slave_dev);
	struct net		*(*get_link_net)(const struct net_device *dev);
	size_t			(*get_linkxstats_size)(const struct net_device *dev,
						       int attr);
	int			(*fill_linkxstats)(struct sk_buff *skb,
						   const struct net_device *dev,
						   int *prividx, int attr);

	RH_KABI_RESERVE(1)
	RH_KABI_RESERVE(2)
	RH_KABI_RESERVE(3)
	RH_KABI_RESERVE(4)
	RH_KABI_RESERVE(5)
	RH_KABI_RESERVE(6)
	RH_KABI_RESERVE(7)
	RH_KABI_RESERVE(8)
};

int __rtnl_link_register(struct rtnl_link_ops *ops);
void __rtnl_link_unregister(struct rtnl_link_ops *ops);

int rtnl_link_register(struct rtnl_link_ops *ops);
void rtnl_link_unregister(struct rtnl_link_ops *ops);

/**
 * 	struct rtnl_af_ops - rtnetlink address family operations
 *
 *	@list: Used internally
 * 	@family: Address family
 * 	@fill_link_af: Function to fill IFLA_AF_SPEC with address family
 * 		       specific netlink attributes.
 * 	@get_link_af_size: Function to calculate size of address family specific
 * 			   netlink attributes.
 *	@validate_link_af: Validate a IFLA_AF_SPEC attribute, must check attr
 *			   for invalid configuration settings.
 * 	@set_link_af: Function to parse a IFLA_AF_SPEC attribute and modify
 *		      net_device accordingly.
 */
struct rtnl_af_ops {
	struct list_head	list;
	int			family;

	int			(*fill_link_af)(struct sk_buff *skb,
						const struct net_device *dev,
						u32 ext_filter_mask);
	size_t			(*get_link_af_size)(const struct net_device *dev,
						    u32 ext_filter_mask);

	int			(*validate_link_af)(const struct net_device *dev,
						    const struct nlattr *attr,
						    struct netlink_ext_ack *extack);
	int			(*set_link_af)(struct net_device *dev,
					       const struct nlattr *attr,
					       struct netlink_ext_ack *extack);
	int			(*fill_stats_af)(struct sk_buff *skb,
						 const struct net_device *dev);
	size_t			(*get_stats_af_size)(const struct net_device *dev);
};

void rtnl_af_register(struct rtnl_af_ops *ops);
void rtnl_af_unregister(struct rtnl_af_ops *ops);

struct net *rtnl_link_get_net(struct net *src_net, struct nlattr *tb[]);
struct net_device *rtnl_create_link(struct net *net, const char *ifname,
				    unsigned char name_assign_type,
				    const struct rtnl_link_ops *ops,
				    struct nlattr *tb[],
				    struct netlink_ext_ack *extack);
int rtnl_delete_link(struct net_device *dev, u32 portid, const struct nlmsghdr *nlh);
int rtnl_configure_link(struct net_device *dev, const struct ifinfomsg *ifm,
			u32 portid, const struct nlmsghdr *nlh);

int rtnl_nla_parse_ifinfomsg(struct nlattr **tb, const struct nlattr *nla_peer,
			     struct netlink_ext_ack *exterr);
struct net *rtnl_get_net_ns_capable(struct sock *sk, int netnsid);

#define MODULE_ALIAS_RTNL_LINK(kind) MODULE_ALIAS("rtnl-link-" kind)

#endif
