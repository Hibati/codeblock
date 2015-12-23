#include "header.h"
#include "connect.h"
int bcount=0;
static pre =1;
int cur=1;
void http(char *api_key,int field,double data)

{

	CURL *curl;
	CURLcode res;
	char url[50];
	/* In windows, this will init the winsock stuff */


	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {
	/* First set the URL that is about to receive our POST. This URL can
	   just as well be a https:// URL if that is what should receive the
	   data. */
//	curl_easy_setopt(curl, CURLOPT_URL, "https://thingspeak.com/update.json");
	curl_easy_setopt(curl, CURLOPT_URL, "http://iotser.iots.com.tw:3000/update.json");
	/* Now specify the POST data */
//	sprintf(url,"api_key=8XG6IW1ZC5YH420K&field1=%d",data[0]);
	sprintf(url,"api_key=%s&field%d=%f",api_key,field,data);

	printf("url=%s\n",url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, url);

	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl);
	/* Check for errors */
	if(res != CURLE_OK)
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
	          curl_easy_strerror(res));

	/* always cleanup */
	curl_easy_cleanup(curl);
	}
//	curl_global_cleanup();
	/*
   int i;
	for(i=0;i<datalen;i++)
	{

		printf("data = %d",data[i]);


	}
	*/


}
static void write_data_to_txt()
{
	FILE *fp;
	int i=0;
	fp = fopen( "data.txt", "w" );
	fprintf(fp, "%d\n",datalen);
	while(i<10)
	{
		fprintf(fp, "%d\n",data[i]);
		i++;
	}
	fclose(fp);
}
static void events_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
	uint8_t *opdu;
	uint16_t handle, i, olen;
	size_t plen;
	GString *s;

	handle = att_get_u16(&pdu[1]);

	switch (pdu[0]) {
	case ATT_OP_HANDLE_NOTIFY:
		s = g_string_new(NULL);
		g_string_printf(s, "Notification handle = 0x%04x value: ",
									handle);
		break;
	case ATT_OP_HANDLE_IND:
		s = g_string_new(NULL);
		g_string_printf(s, "Indication   handle = 0x%04x value: ",
									handle);
		break;
	default:
		error("Invalid opcode\n");
		return;
	}

	for (i = 3; i < len; i++)
	{
			g_string_append_printf(s, "%02x ", pdu[i]);
			data[i-3] = pdu[i];

	}
	datalen = len - 3;
	//write_data_to_txt();
	curl_global_init(CURL_GLOBAL_ALL);
//	g_print("datalen=%d\n",datalen);
/*
	if(data[0]==49)
	http("8XG6IW1ZC5YH420k",1,1);
	else
	http("8XG6IW1ZC5YH420k",1,0);

	http("VD4K0RK8CNOTWAUC",1,((data[1]<<8)+data[2])/10);
	http("VD4K0RK8CNOTWAUC",2,((data[3]<<8)+data[4])/10);
	http("ZW4V20RB26PHNFV6",1,(float)((data[5]*10)/data[6]));
	http("MX2XA7YIEZCKWE14",1,((data[7]<<8)+data[8]));
*/
	rl_printf("%s\n", s->str);
	g_string_free(s, TRUE);

	if (pdu[0] == ATT_OP_HANDLE_NOTIFY)
		return;

	opdu = g_attrib_get_buffer(attrib, &plen);
	olen = enc_confirmation(opdu, plen);

	if (olen > 0)
		g_attrib_send(attrib, 0, opdu, olen, NULL, NULL, NULL);
}

static gboolean connect_cb2(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{

	struct connect *conn = user_data;
	GError *gerr = NULL;
	int err, sk_err, sock;
	socklen_t len = sizeof(sk_err);

	/* If the user aborted this connect attempt */
	if ((cond & G_IO_NVAL) || check_nval(io))
		return FALSE;

	sock = g_io_channel_unix_get_fd(io);

	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
	{
			err = -errno;
				 g_print ("error1\n");
	}
	else
	{
		err = -sk_err;
			 g_print ("error2\n");
	}


	if (err < 0)
	{
			g_print ("err = %d\n",err);
			ERROR_FAILED(&gerr, "connect error", -err);
	}


	conn->connect(io, gerr, conn->user_data);
	 g_print ("timeout_callback called 1 times\n");
	g_clear_error(&gerr);

	return FALSE;
}


static void test_cb()
{
printf("cur=%d\n",cur);
    if(digitalRead(7)==0)
    cur=0;

}

static void test_cb2()
{
printf("cur=%d\n",cur);
//delay(30);
if(digitalRead(7)==0)
{
printf("sdsdf\n");
//delay(100);
if(pre  ==31)
{
active_the_lamp(30);
pre =30;
}
else
{
active_the_lamp(31);
pre=31;
}


}

//printf("cur =%d, pre =%d\n",cur,pre);



}
void  active_the_lamp(int a)
{
printf("fgfgfg\n");
    size_t plen;
    uint8_t *value;
    if(a==30)
    {

        plen = gatt_attr_data_from_string("30", &value);
        gatt_write_cmd(attrib, 14, value, plen, NULL, NULL);
    }
    else
    {

        plen = gatt_attr_data_from_string("31", &value);
        gatt_write_cmd(attrib, 14, value, plen, NULL, NULL);
    }


    g_free(value);



}
static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	g_print ("timeout_callback called 2 times\n");


	if (err) {
		//set_state(STATE_DISCONNECTED);


		//error("%s\n", err->message);
		return;
	}
	attrib = g_attrib_new(io);
	g_attrib_register(attrib, ATT_OP_HANDLE_NOTIFY, GATTRIB_ALL_HANDLES,
					events_handler, attrib, NULL);
	g_attrib_register(attrib, ATT_OP_HANDLE_IND, GATTRIB_ALL_HANDLES,
					events_handler, attrib, NULL);
	g_print("Connection successful\n");
	//gatt_discover_primary(attrib, NULL, primary_all_cb, NULL);
	/*
	size_t plen;
	uint8_t *value;
	plen = gatt_attr_data_from_string("0100", &value);
	gatt_write_cmd(attrib, 12, value, plen, NULL, NULL);
	g_free(value);
	*/
	//g_timeout_add_seconds(0,test_cb,NULL);
	//g_timeout_add_seconds(0,test_cb2,NULL);
	g_timeout_add(150,test_cb2,NULL);


//	test_cb2();










}
static void connect_add(GIOChannel *io, BtIOConnect connect,
				gpointer user_data, GDestroyNotify destroy)
{
	struct connect *conn;
	GIOCondition cond;

	conn = g_new0(struct connect, 1);
	conn->connect = connect;
	conn->user_data = user_data;
	conn->destroy = destroy;

	cond = G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL ;
	g_io_add_watch_full(io, G_PRIORITY_DEFAULT, cond, connect_cb2, conn,
					(GDestroyNotify) connect_remove);
}
void myinterupt()
{

	static int it =0, on=0,stop=0,ms=0;

	it++;

	printf("it= %d\n  , on=%d  , stop = %d\n",it,on,stop);


	if(it ==1 && on==0 && stop ==0)
	{

		stop =1;
		size_t plen;
		uint8_t *value;
		plen = gatt_attr_data_from_string("30", &value);
		gatt_write_cmd(attrib, 14, value, plen, NULL, NULL);
		g_free(value);
		delay(500);
		on = 1;
		//it=0;
		//stop=0;


	}
	else if (it==1 && on==1 && stop ==0)
	{
		stop=1;
		size_t plen;
		uint8_t *value;
		plen = gatt_attr_data_from_string("31", &value);
		gatt_write_cmd(attrib, 14, value, plen, NULL, NULL);
		g_free(value);
		delay(500);
		on =0;
		//it=0;
		//stop=0;
	}


		ms = 500;


}
int main(int argc, char *argv[])
{
		FILE *fPtr;


		fPtr = fopen("pid.txt", "w");
		if (!fPtr) {
		printf("檔案建立失敗...\n");

		}

		fprintf(fPtr, "%d", getpid());

		fclose(fPtr);
        if(wiringPiSetup()==-1)
        exit(1);

        pinMode(7,INPUT);


        //wiringPiISR(7,INT_EDGE_FALLING,&myinterupt);
		GError **err;
		bdaddr_t *sba, *dba;
		struct set_opts *opts;
		opts = (struct set_opts*)malloc(sizeof(struct set_opts));
		BtIOSecLevel sec = BT_IO_SEC_LOW;
		opts->defer = DEFAULT_DEFER_TIMEOUT;
		opts->master = -1;
		opts->mode = L2CAP_MODE_BASIC;
		opts->flushable = -1;
		opts->priority = 0;
		opts->src_type = BDADDR_LE_PUBLIC;
		//str2ba("D8:AD:4A:AA:42:B5",&opts->dst);
		//str2ba("EE:2A:81:DE:72:6A",&opts->dst);
		//str2ba("00:1A:7D:DA:71:13",&opts->src);
		str2ba("F6:97:28:12:42:66",&opts->dst);
		opts->dst_type = BDADDR_LE_RANDOM;
		opts->type = BT_IO_L2CAP;
		opts->cid = ATT_CID;
		opts->sec_level = sec;

	 	GMainLoop *event_loop = g_main_loop_new(NULL, FALSE);


		int sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
		GIOChannel *io = g_io_channel_unix_new(sock);
		g_io_channel_set_close_on_unref(io, TRUE);
		g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);

		printf("sock=%d\n",sock);
		printf("binding...\n");
		int bind = l2cap_bind(sock, &opts->src, opts->src_type,
			0, opts->cid, err);
		printf("bind=%d\n",bind);
		printf("setting...\n");

		int set = l2cap_set(sock, opts->sec_level, opts->imtu, opts->omtu,
			opts->mode, opts->master, opts->flushable,
			opts->priority, err);
		printf("set=%d\n",set);
		printf("connecting....\n");
		int con = l2cap_connect( g_io_channel_unix_get_fd(io), &opts->dst, opts->dst_type,
						opts->psm, opts->cid);

		printf("connection=%d\n",con);

		connect_add(io, connect_cb, NULL, NULL);
	//	ERROR_FAILED(err, "connect", -con);

		g_main_loop_run(event_loop);
		g_main_loop_unref(event_loop);




		scanf("%d",&con);
		return 0;
}
