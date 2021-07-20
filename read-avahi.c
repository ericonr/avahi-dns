/*
 * read-avahi.c
 *
 * Read avahi-browse output and create a config file for a local
 * DNS server. Requires avahi-browse(1) and a running Avahi daemon.
 *
 * Output is hardcoded:
 * - dns.conf for unbound.conf(5)
 * - hosts for a hosts(5)
 *
 * Copyright (c) 2021 Érico Nogueira
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	/* unbound config file */
	FILE *fu = fopen("dns.conf", "w");
	if (!fu) return 1;
	/* hosts */
	FILE *fh = fopen("hosts", "w");
	if (!fu) return 1;

	FILE *p = popen("avahi-browse --all --resolve --no-db-lookup --parsable --terminate", "re");
	if (!p) return 1;

	char *line = 0;
	size_t n;
	while (getline(&line, &n, p) > 0) {
		/* not resolved */
		if (line[0] != '=') continue;

		int ipv6 = -1;
		char *name=0, *ip=0;
		char *sp, *s, *liner = line;
		for (int i=0; s = strtok_r(liner, ";", &sp); i++, liner=0) {
			switch (i) {
				case 2:
					if (strcmp(s, "IPv4") == 0) ipv6 = 0;
					else if (strcmp(s, "IPv6") == 0) ipv6 = 1;
					break;
				case 6:
					name = s;
					break;
				case 7:
					ip = s;
					break;
			}
		}

		/* failed to find the wanted info */
		if (!(ipv6>=0 && name && ip)) continue;

		char *type = ipv6 ? "AAAA" : "A";

		fprintf(fu, "local-data: \"%s. IN %s %s\"\n", name, type, ip);
		fprintf(fh, "%s %s\n", ip, name);
	}
	free(line);

	fclose(fu);
	fclose(fh);
	pclose(p);

	return 0;
}
