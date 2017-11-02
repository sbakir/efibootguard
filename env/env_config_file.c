/*
 * EFI Boot Guard
 *
 * Copyright (c) Siemens AG, 2017
 *
 * Authors:
 *  Andreas Reichel <andreas.reichel.ext@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include "env_api.h"
#include "env_disk_utils.h"
#include "env_config_file.h"

FILE *open_config_file(CONFIG_PART *cfgpart, char *mode)
{
	char *configfilepath;
	configfilepath = (char *)malloc(strlen(FAT_ENV_FILENAME) +
					strlen(cfgpart->mountpoint) + 2);
	if (!configfilepath) {
		return NULL;
	}
	strncpy(configfilepath, cfgpart->mountpoint,
		strlen(cfgpart->mountpoint) + 1);
	strncat(configfilepath, "/", 1);
	strncat(configfilepath, FAT_ENV_FILENAME, strlen(FAT_ENV_FILENAME));
	VERBOSE(stdout, "Probing config file at %s.\n", configfilepath);
	FILE *config = fopen(configfilepath, mode);
	free(configfilepath);
	return config;
}

int close_config_file(FILE *config_file_handle)
{
	if (config_file_handle)
	{
		return fclose(config_file_handle);
	}
}

bool probe_config_file(CONFIG_PART *cfgpart)
{
	bool do_unmount = false;
	if (!cfgpart) {
		return false;
	}
	printf_debug("Checking device: %s\n", cfgpart->devpath);
	if (!(cfgpart->mountpoint = get_mountpoint(cfgpart->devpath))) {
		/* partition is not mounted */
		cfgpart->not_mounted = true;
		VERBOSE(stdout, "Partition %s is not mounted.\n",
			cfgpart->devpath);
		if (!mount_partition(cfgpart)) {
			return false;
		}
		do_unmount = true;
	} else {
		cfgpart->not_mounted = false;
	}

	if (cfgpart->mountpoint) {
		/* partition is mounted to mountpoint, either before or by this
		 * program */
		VERBOSE(stdout, "Partition %s is mounted to %s.\n",
			cfgpart->devpath, cfgpart->mountpoint);
		bool result = false;
		FILE *config;
		if (!(config = open_config_file(cfgpart, "rb"))) {
			printf_debug(
			    "Could not open config file on partition %s.\n",
			    FAT_ENV_FILENAME);
		} else {
			result = true;
			if (fclose(config)) {
				VERBOSE(stderr, "Error closing config file on "
						"partition %s.\n",
						cfgpart->devpath);
			}
		}
		if (do_unmount) {
			unmount_partition(cfgpart);
		}
		return result;
	}
	return false;
}