void rl_printf(const char *fmt, ...)
{
	va_list args;
	bool save_input;
	char *saved_line;
	int saved_point;

	save_input = !RL_ISSTATE(RL_STATE_DONE);

	if (save_input) {
		saved_point = rl_point;
		saved_line = rl_copy_text(0, rl_end);
		rl_save_prompt();
		rl_replace_line("", 0);
		rl_redisplay();
	}

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	if (save_input) {
		rl_restore_prompt();
		rl_replace_line(saved_line, 0);
		rl_point = saved_point;
		rl_redisplay();
		free(saved_line);
	}
}
size_t gatt_attr_data_from_string(const char *str, uint8_t **data)
{
	char tmp[3];
	size_t size, i;

	size = strlen(str) / 2;
	*data = g_try_malloc0(size);
	if (*data == NULL)
		return 0;

	tmp[2] = '\0';
	for (i = 0; i < size; i++) {
		memcpy(tmp, str + (i * 2), 2);
		(*data)[i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	return size;
}



static void primary_all_cb(GSList *services, guint8 status, gpointer user_data)
{
	GSList *l;

	if (status) {
		error("Discover all primary services failed: %s\n",
						att_ecode2str(status));
		return;
	}

	if (services == NULL) {
		error("No primary service found\n");
		return;
	}

	for (l = services; l; l = l->next) {
		struct gatt_primary *prim = l->data;
		rl_printf("attr handle: 0x%04x, end grp handle: 0x%04x uuid: %s\n",
				prim->range.start, prim->range.end, prim->uuid);
	}
}

static void connect_remove(struct connect *conn)
{
	if (conn->destroy)
		conn->destroy(conn->user_data);
	g_free(conn);
}

static int l2cap_set_lm(int sock, int level)
{
	int lm_map[] = {
		0,
		L2CAP_LM_AUTH,
		L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT,
		L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT | L2CAP_LM_SECURE,
	}, opt = lm_map[level];

	if (setsockopt(sock, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0)
		return -errno;

	return 0;
}
static gboolean set_sec_level(int sock, BtIOType type, int level, GError **err)
{
	struct bt_security sec;
	int ret;

	if (level < BT_SECURITY_LOW || level > BT_SECURITY_HIGH) {
		g_set_error(err, BT_IO_ERROR, EINVAL,
				"Valid security level range is %d-%d",
				BT_SECURITY_LOW, BT_SECURITY_HIGH);
		return FALSE;
	}

	memset(&sec, 0, sizeof(sec));
	sec.level = level;

	if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec,
							sizeof(sec)) == 0)
		return TRUE;

	if (errno != ENOPROTOOPT) {
	//	ERROR_FAILED(err, "setsockopt(BT_SECURITY)", errno);
		return FALSE;
	}

	if (type == BT_IO_L2CAP)
		ret = l2cap_set_lm(sock, level);
	else
	//	ret = rfcomm_set_lm(sock, level);

	if (ret < 0) {
	//	ERROR_FAILED(err, "setsockopt(LM)", -ret);
		return FALSE;
	}

	return TRUE;
}


static int l2cap_bind(int sock, const bdaddr_t *src, uint8_t src_type,
				uint16_t psm, uint16_t cid, GError **err)
{
	struct sockaddr_l2 addr;

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);


	addr.l2_cid = htobs(cid);


	addr.l2_bdaddr_type = src_type;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		int error = -errno;
	//	ERROR_FAILED(err, "l2cap_bind", errno);
		return error;
	}

	return 0;
}

static int l2cap_connect(int sock, const bdaddr_t *dst, uint8_t dst_type,
						uint16_t psm, uint16_t cid)
{
	int err;
	char ad[25];
	struct sockaddr_l2 addr;
    bdaddr_t *truedst;
   // str2ba(opt_dst,&truedst);
	memset(&addr, 0, sizeof(addr));

	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	ba2str(dst,ad);
	printf("addr=%s\n",ad);

	if (cid)
	{
		addr.l2_cid = htobs(cid);
		printf("cid\n");
	}
	else
		addr.l2_psm = htobs(psm);

	addr.l2_bdaddr_type = dst_type;
	printf("dst_type=%d\n",dst_type);
	err = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
		return -errno;

	return 0;
}


static gboolean l2cap_set(int sock, int sec_level, uint16_t imtu,
				uint16_t omtu, uint8_t mode, int master,
				int flushable, uint32_t priority, GError **err)
{
	if (imtu || omtu || mode) {
		struct l2cap_options l2o;
		socklen_t len;

		memset(&l2o, 0, sizeof(l2o));
		len = sizeof(l2o);
		if (getsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o,
								&len) < 0) {
			//ERROR_FAILED(err, "getsockopt(L2CAP_OPTIONS)", errno);
			return FALSE;
		}

		if (imtu)
			l2o.imtu = imtu;
		if (omtu)
			l2o.omtu = omtu;
		if (mode)
			l2o.mode = mode;

		if (setsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o,
							sizeof(l2o)) < 0) {
		//	ERROR_FAILED(err, "setsockopt(L2CAP_OPTIONS)", errno);
			return FALSE;
		}
	}



	if (sec_level && !set_sec_level(sock, BT_IO_L2CAP, sec_level, err))
		return FALSE;

	return TRUE;
}
static gboolean check_nval(GIOChannel *io)
{
	struct pollfd fds;

	memset(&fds, 0, sizeof(fds));
	fds.fd = g_io_channel_unix_get_fd(io);
	fds.events = POLLNVAL;

	if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLNVAL))
		return TRUE;

	return FALSE;
}
