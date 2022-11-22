/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <env.h>


static int rollback_part(const char *partition, const char *partition_alt)
{
	char *p;
	int ret;
	int tmp;

	p = env_get(partition_alt);
	if (p == NULL) {
		return 0;
	}
	tmp = 1;
	printf("Rollback partition %s to %s\n", partition, p);
	ret = env_set(partition, p);
	if (ret) {
		printf("Failed to set env %s %s: ret = %d\n", partition, p, ret);
		tmp = -1;
	}
	ret = env_set(partition_alt, NULL);
	if (ret) {
		printf("Failed to del env %s: ret = %d\n", partition_alt, ret);
		tmp = -1;
	}
	return tmp;
}

static int upgrade_rollback_check(void)
{
	unsigned long bootlimit;
	unsigned long bootcount;
	char *p;
	char buf[20];
	int ret;
	int save;

	p = env_get("bootlimit");
	if (p == NULL) {
		return -1;
	}
	if (!strcmp(p, "0")) {
		return 0;
	} else {
		if (strict_strtoul(p, 16, &bootlimit) < 0) {
			printf("Failed to strict_strtoul bootlimit\n");
			return -1;
		}
	}
	p = env_get("bootcount");
	if (p == NULL) {
		bootcount = 0;
	} else if (strict_strtoul(p, 16, &bootcount) < 0) {
		bootcount = 0;
	}
	save = 0;
	bootcount++;
	if (bootcount == bootlimit + 1) {
		save = 1;
		printf("Failed to start for %lu times, will rollback!\n", bootlimit);
		rollback_part("boot_partition", "boot_partition_alt");
		rollback_part("root_partition", "root_partition_alt");
	} else if (bootcount < bootlimit + 1) {
		save = 1;
	}
	if (save) {
		snprintf(buf, sizeof(buf), "%lu", bootcount);
		ret = env_set("bootcount", buf);
		if (ret) {
			printf("Failed to set env bootcount %s: ret = %d\n", buf, ret);
		}
		ret = env_save();
		if (ret) {
			printf("Failed to env_save: ret = %d\n", ret);
		}
	}
	
	return 0;
}

static int do_rollback(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
    upgrade_rollback_check();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	rollback, 1, 1,	do_rollback,
	"Automatic rollback if upgrade fails",
	NULL
);
