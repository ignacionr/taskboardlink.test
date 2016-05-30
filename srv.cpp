#define BLUR_MAX	20
#define X_STEP_MAX 150

#include <iostream>
#include <fstream>
#include <random>
#include <map>
#include <functional>

#include "./CImg.h"

#define __DARWIN_C_LEVEL 201605L
#include "./mongoose.h"

using namespace cimg_library;
using namespace std;

#define BIG_SRC "./world.topo.200402.3x21600x10800.jpg"

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

CImg<unsigned char> *p_base_image;
int x, y;
random_device rd;
default_random_engine g(rd());


void update_current() {
	auto window = p_base_image->get_crop
		(x, y, 0, 0, x+639, y+479,0, 0);
	window.blur(
		(float)(g() % BLUR_MAX) / 10.0,
		(float)(g() % BLUR_MAX) / 10.0,
		(float)0);
	window.save_jpeg("current.jpg");
}

static void reply_current(struct mg_connection *nc) {
	ifstream content("current.jpg");
	content.seekg(0, content.end);
	auto length = content.tellg();
	content.seekg(0, content.beg);
	mg_send_head(nc, 200, length, NULL);
	char *buff = new char[length];
	content.read(buff, length);
	mg_send(nc, buff, length);
	delete[] buff;
}

static map<string, function<void(int&,int&)>> _moves = {
	{"/left", [] (int& x, int& y) {
			x -= g() % X_STEP_MAX;
			y += (g() % 9) - 4;
		}},
	{"/right", [] (int& x, int& y) {
			x += g() % X_STEP_MAX;
			y += (g() % 9) - 4;
		}},
	{"/up", [] (int& x, int& y) {
			x += (g() % 9) - 4;
			y -= (g() % 100) + 2;
		}},
	{"/down", [] (int& x, int& y) {
			x += (g() % 9) - 4;
			y += (g() % 100) + 2;
		}}
};

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
	struct http_message *hm = (struct http_message *) p;
	if (ev == MG_EV_HTTP_REQUEST) {
		string uri(hm->uri.p, hm->uri.len);
		auto pmove = _moves.find(uri);
		if (pmove != _moves.end()) {
			(*pmove).second(x,y);
			update_current();
			reply_current(nc);
		}
		else {
			mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
		}
	}
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";  // Serve current directory
  s_http_server_opts.dav_document_root = ".";  // Allow access via WebDav
  s_http_server_opts.enable_directory_listing = "yes";

	CImg<unsigned char> big(BIG_SRC);
	p_base_image = &big;
	cout << "big image is " << big.width() << "x" << big.height() << endl;
	x = g() % big.width();
	y = g() % big.height();
	
	update_current();


  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
