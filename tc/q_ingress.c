/*
 * q_ingress.c             INGRESS.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Authors:    J Hadi Salim
 */

#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr,
		"Usage: [ add | replace | link | delete ] ... ingress\n"
		"       change ... ingress [ clsact-on | clsact-off ]\n"
		" clsact-on\tenable clsact egress mini-Qdisc\n"
		" clsact-off\tdelete clsact egress mini-Qdisc\n");
}

static int ingress_parse_opt(struct qdisc_util *qu, int argc, char **argv,
			     struct nlmsghdr *n, const char *dev)
{
	struct nla_bitfield32 flags = {
		.selector = TC_INGRESS_SUPPORTED_FLAGS,
	};
	bool change = false;
	struct rtattr *tail;

	while (argc > 0) {
		if (strcmp(*argv, "handle") == 0) {
			NEXT_ARG();
		} else if (strcmp(*argv, "clsact-on") == 0) {
			flags.value |= TC_INGRESS_CLSACT;
			change = true;
		} else if (strcmp(*argv, "clsact-off") == 0) {
			flags.value &= ~TC_INGRESS_CLSACT;
			change = true;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}

		argc--;
		argv++;
	}

	if (change) {
		tail = addattr_nest(n, 1024, TCA_OPTIONS);
		addattr_l(n, 1024, TCA_INGRESS_FLAGS, &flags, sizeof(flags));
		addattr_nest_end(n, tail);
	}

	return 0;
}

static int ingress_print_opt(struct qdisc_util *qu, FILE *f,
			     struct rtattr *opt)
{
	struct rtattr *tb[TCA_INGRESS_MAX + 1];
	struct nla_bitfield32 *flags;

	print_string(PRINT_FP, NULL, "---------------- ", NULL);

	if (!opt)
		return 0;

	parse_rtattr_nested(tb, TCA_INGRESS_MAX, opt);

	if (!tb[TCA_INGRESS_FLAGS])
		return -1;
	if (RTA_PAYLOAD(tb[TCA_INGRESS_FLAGS]) < sizeof(*flags))
		return -1;

	flags = RTA_DATA(tb[TCA_INGRESS_FLAGS]);
	if (flags->value & TC_INGRESS_CLSACT)
		print_string(PRINT_FP, NULL, "clsact", NULL);

	return 0;
}

struct qdisc_util ingress_qdisc_util = {
	.id		= "ingress",
	.parse_qopt	= ingress_parse_opt,
	.print_qopt	= ingress_print_opt,
};
