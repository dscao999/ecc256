#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <alsa/global.h>
#include <alsa/input.h>
#include <alsa/output.h>
#include <alsa/conf.h>
#include <alsa/control_external.h>
//#include <sound/asound.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char **dnames, **dn;
	int sysret;
	char *name;

	sysret =  snd_device_name_hint(-1, "pcm", (void ***)&dnames);
	if (sysret == -1) {
		fprintf(stderr, "snd_device_name_hint failed: %s\n",
				strerror(errno));
		return 1;
	}
	dn = dnames;
	while (*dn != NULL) {
		name = snd_device_name_get_hint(*dn, "NAME");
		if (name) {
		       	if (strcmp(name, "null") != 0)
				printf("Name: %s\n", name);
			free(name);
		}
		dn++;
	}
	snd_device_name_free_hint((void **)dnames);
	return 0;
}
