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


static int writer (char * data, size_t size, size_t nmemb, std::string * writerData)
{
	if (writerData == nullptr) {
		return 0;
	}
	writerData->append (data, size * nmemb);
	return size * nmemb;
}


static string http_get (string url)
{
	CURL *curl;
	CURLcode res;
	curl_global_init (CURL_GLOBAL_ALL);

	curl = curl_easy_init ();
	if (! curl) {
		throw (HttpException {__LINE__});
	}
	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	string reply_1;
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, & reply_1);
	res = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	if (res != CURLE_OK) {
		throw (HttpException {__LINE__});
	}
	return reply_1;
}


static time_t str2time (string s)
{
	struct tm tm;
	strptime (s.c_str (), "%Y-%m-%dT%H:%M:%S", & tm);
	return timegm (& tm);
}


static time_t get_time (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {__LINE__});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"created_at"}) == properties.end ()) {
		throw (TootException {__LINE__});
	}
	auto time_object = properties.at (string {"created_at"});
	if (! time_object.is <string> ()) {
		throw (TootException {__LINE__});
	}
	auto time_s = time_object.get <string> ();
	return str2time (time_s);
}


unsigned int get_date (time_t time)
{
	return (time + (9 * 60 * 60)) / (24 * 60 * 60);
}


static void for_host (string host, unsigned int wait)
{
	vector <picojson::value> timeline;

	{
		string reply = http_get (string {"https://"} + host + string {"/api/v1/timelines/public?local=true&limit=40"});

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
			string reply = http_get (query);
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

