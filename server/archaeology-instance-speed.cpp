#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <unistd.h>
#include "picojson.h"
#include "distsn.h"


using namespace std;


unsigned int get_date (time_t time)
{
	return (time + (9 * 60 * 60)) / (24 * 60 * 60);
}


static void for_host (string host, unsigned int wait)
{
	Http http;
	vector <picojson::value> timeline;

	{
		string reply = http.perform (string {"https://"} + host + string {"/api/v1/timelines/public?local=true&limit=40"});

		picojson::value json_value;
		string error = picojson::parse (json_value, reply);
		if (! error.empty ()) {
			throw (HostException {__LINE__});
		}
		if (! json_value.is <picojson::array> ()) {
			throw (HostException {__LINE__});
		}
	
		vector <picojson::value> toots = json_value.get <picojson::array> ();
		timeline.insert (timeline.end (), toots.begin (), toots.end ());
	}
	
	if (timeline.size () < 1) {
		throw (HostException {__LINE__});
	}

	unsigned int current_date = get_date (time (nullptr)); 
	unsigned int current_counter = 0;

	for (; ; ) {
		if (0 < wait) {
			sleep (wait);
		}

		string bottom_id;
		try {
			bottom_id = get_id (timeline.back ());
		} catch (TootException e) {
			throw (HostException {__LINE__});
		}
		
		for (auto toot: timeline) {
			try {
				unsigned int date = get_date (get_time (toot));
				if (current_date == date) {
					current_counter ++;
				} else {
					if (0 < current_counter) {
						cout << "\"" << current_date << "\",\"" << current_counter << "\"" << endl;
					}
					current_date = date;
					current_counter = 1;
				}
			} catch (TootException e) {
				/* Do nothing. */
			}
		}

		timeline.clear ();

		string query
			= string {"https://"}
			+ host
			+ string {"/api/v1/timelines/public?local=true&limit=40&max_id="}
			+ bottom_id;
		vector <picojson::value> toots;

		for (unsigned int cn = 0; ; cn ++) {
			if (16 <= cn) {
				throw (HostException {__LINE__});
			}
			string reply = http.perform (query);
			picojson::value json_value;
			string error = picojson::parse (json_value, reply);
			if (error.empty () && json_value.is <picojson::array> ()) {
				toots = json_value.get <picojson::array> ();
				break;
			}
			sleep (10);
		}
		
		if (toots.size () == 0) {
			break;
		}
		
		timeline.insert (timeline.end (), toots.begin (), toots.end ());
	}


	if (0 < current_counter) {
		cout << "\"" << current_date << "\",\"" << current_counter << "\"" << endl;
	}
}


int main (int argc, char **argv)
{
	if (2 <= argc) {
		unsigned int wait = 0;
		if (3 <= argc) {
			stringstream {argv [2]} >> wait;
		}
		try {
			for_host (string {argv [1]}, wait);
		} catch (ExceptionWithLineNumber e) {
			cerr << "Exception in " << e.line << endl;
		}
	} else {
		cout << "distsn-archaeology-instance-speed host [wait]" << endl;
		return 1;
	}
	return 0;
}

