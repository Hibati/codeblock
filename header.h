#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <errno.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/uuid.h>
#include <gatt/gatt.h>
#include <gatt/gattrib.h>
#include <gatt/btio.h>
#include <curl/curl.h>
#include <wiringPi.h>
#define DEFAULT_DEFER_TIMEOUT 30
#define ERROR_FAILED(gerr, str, err) \
		g_set_error(gerr, BT_IO_ERROR, err, \
				str ": %s (%d)", strerror(err), err)
static GAttrib *attrib = NULL;
double data[50];
int postflag=0;
int datalen = 0;
int times=0;
int pre_state=30;
static char *opt_dst_type = NULL;
//static char *opt_value = NULL;
static char *opt_sec_level = NULL;
//static bt_uuid_t *opt_uuid = NULL;
//static int opt_start = 0x0001;
//static int opt_end = 0xffff;
//static int opt_handle = -1;
static int opt_mtu = 23;
static int opt_psm = 0;
typedef enum {
	BT_IO_L2CAP,
	BT_IO_RFCOMM,
	BT_IO_SCO,
	BT_IO_INVALID,
} BtIOType;

struct connect {
	BtIOConnect connect;
	gpointer user_data;
	GDestroyNotify destroy;
};
struct set_opts {
	bdaddr_t src;
	bdaddr_t dst;
	BtIOType type;
	uint8_t src_type;
	uint8_t dst_type;
	int defer;
	int sec_level;
	uint8_t channel;
	uint16_t psm;
	uint16_t cid;
	uint16_t mtu;
	uint16_t imtu;
	uint16_t omtu;
	int master;
	uint8_t mode;
	int flushable;
	uint32_t priority;
	uint16_t voice;
};
