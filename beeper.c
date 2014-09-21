/*
 * Beeper.c
 *
 * Registers a netfilter hook in NF_INET_LOCAL_IN that beeps through
 * the PC speaker on every ICMP Echo packet it receives.
 *
 * PC speaker functionality borrowed from the pcspkr_event() handling
 * under drivers/input/misc/pcspkr.c.
 *
 * Copyright (c) 2014 Ragnar B. Johannsson
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i8253.h>
#include <linux/timex.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <asm/io.h>

MODULE_AUTHOR("Ragnar B. Johannsson <ragnar@igo.is>");
MODULE_DESCRIPTION("PC speaker beeper for incoming ICMP Echo requests");
MODULE_LICENSE("Dual BSD/GPL");

static void beep(void)
{
	unsigned int count = PIT_TICK_RATE / 1000;
	unsigned long flags;

	raw_spin_lock_irqsave(&i8253_lock, flags);

	outb_p(0xB6, 0x43);
	outb_p(count & 0xff, 0x42);
	outb((count >> 8) & 0xff, 0x42);
	outb_p(inb_p(0x61) | 3, 0x61);

	raw_spin_unlock_irqrestore(&i8253_lock, flags);
}

static unsigned int
icmp_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
	  const struct net_device *in, const struct net_device *out,
	  int (*okfn)(struct sk_buff *))
{
	if (ip_hdr(skb)->protocol == IPPROTO_ICMP &&
	    icmp_hdr(skb)->type == ICMP_ECHO)
		beep();

	return NF_ACCEPT;
}

static struct nf_hook_ops icmp_hook_ops = {
	.hook      = icmp_hook,
	.owner     = THIS_MODULE,
	.pf        = NFPROTO_IPV4,
	.hooknum   = NF_INET_LOCAL_IN,
	.priority  = NF_IP_PRI_FILTER,
};

static int __init beeper_init(void)
{
	return nf_register_hook(&icmp_hook_ops);
}

static void __exit beeper_exit(void)
{
	unsigned long flags;

	nf_unregister_hook(&icmp_hook_ops);
	raw_spin_lock_irqsave(&i8253_lock, flags);
	outb(inb_p(0x61) & 0xFC, 0x61);
	raw_spin_unlock_irqrestore(&i8253_lock, flags);
}

module_init(beeper_init);
module_exit(beeper_exit);
