#include <sstream>
#include <curl/curl.h>
#include "distsn.h"


using namespace std;


string get_id (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"id"}) == properties.end ()) {
		throw (TootException {});
	}
	auto id_object = properties.at (string {"id"});
	string id_string;
	if (id_object.is <double> ()) {
		double id_double = id_object.get <double> ();
		stringstream s;
		s << static_cast <unsigned int> (id_double);
		id_string = s.str ();
	} else if (id_object.is <string> ()) {
		id_string = id_object.get <string> ();
	} else {
		throw (TootException {});
	}
	return id_string;
}


vector <picojson::value> get_timeline (string host)
{
	return get_timeline (host, 60 * 60 * 3);
}


vector <picojson::value> get_timeline (string host, unsigned int time_depth)
{
	vector <picojson::value> timeline;
	Http http;

	{
		string reply = http.perform (string {"https://"} + host + string {"/api/v1/timelines/public?local=true&limit=40"});

		picojson::value json_value;
		string error = picojson::parse (json_value, reply);
		if (! error.empty ()) {
			throw (HostException {});
		}
		if (! json_value.is <picojson::array> ()) {
			throw (HostException {});
		}
	
		vector <picojson::value> toots = json_value.get <picojson::array> ();
		timeline.insert (timeline.end (), toots.begin (), toots.end ());
	}
	
	if (timeline.size () < 1) {
		throw (HostException {});
	}
	
	for (unsigned int cn = 0; ; cn ++) {
		if (1000 <= cn) {
			throw (HostException {__LINE__});
		}
		
		time_t top_time;
		time_t bottom_time;
		try {
			top_time = get_time (timeline.front ());
			bottom_time = get_time (timeline.back ());
		} catch (TootException e) {
			throw (HostException {});
		}
		if (static_cast <int> (time_depth) <= top_time - bottom_time && 40 <= timeline.size ()) {
			break;
		}

		string bottom_id;
		try {
			bottom_id = get_id (timeline.back ());
		} catch (TootException e) {
			throw (HostException {});
		}
		string query
			= string {"https://"}
			+ host
			+ string {"/api/v1/timelines/public?local=true&limit=40&max_id="}
			+ bottom_id;
		string reply = http.perform (query);

		picojson::value json_value;
		string error = picojson::parse (json_value, reply);
		if (! error.empty ()) {
			throw (HostException {});
		}
		if (! json_value.is <picojson::array> ()) {
			throw (HostException {});
		}
	
		vector <picojson::value> toots = json_value.get <picojson::array> ();
		if (toots.empty ()) {
			break;
		}
		timeline.insert (timeline.end (), toots.begin (), toots.end ());
	}

	return timeline;
}


static int writer (char * data, size_t size, size_t nmemb, std::string * writerData)
{
	if (writerData == nullptr) {
		return 0;
	}
	writerData->append (data, size * nmemb);
	return size * nmemb;
}


time_t get_time (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"created_at"}) == properties.end ()) {
		throw (TootException {});
	}
	auto time_object = properties.at (string {"created_at"});
	if (! time_object.is <string> ()) {
		throw (TootException {});
	}
	auto time_s = time_object.get <string> ();
	return str2time (time_s);
}


time_t str2time (string s)
{
	struct tm tm;
	strptime (s.c_str (), "%Y-%m-%dT%H:%M:%S", & tm);
	return timegm (& tm);
}


string get_host_title (string domain)
{
	Http http;
	return get_host_title (domain, http);
}


string get_host_title (string domain, Http &http)
{
	string reply = http.perform (string {"https://"} + domain + string {"/api/v1/instance"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
	}
	auto properties = json_value.get <picojson::object> ();
	if (properties.find (string {"title"}) == properties.end ()) {
		throw (HostException {__LINE__});
	}
	auto title_object = properties.at (string {"title"});
	if (! title_object.is <string> ()) {
		throw (HostException {__LINE__});
	}
	return title_object.get <string> ();
}


string get_host_thumbnail (string domain)
{
	Http http;
	return get_host_thumbnail (domain, http);
}


string get_host_thumbnail (string domain, Http &http)
{
	string reply = http.perform (string {"https://"} + domain + string {"/api/v1/instance"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {__LINE__});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {__LINE__});
	}
	auto properties = json_value.get <picojson::object> ();
	if (properties.find (string {"thumbnail"}) == properties.end ()) {
		throw (HostException {__LINE__});
	}
	auto thumbnail_object = properties.at (string {"thumbnail"});
	if (! thumbnail_object.is <string> ()) {
		throw (HostException {__LINE__});
	}
	return thumbnail_object.get <string> ();
}


set <string> get_international_hosts ()
{
	Http http;
	const string url {"http://distsn.org/cgi-bin/instances-api.cgi"};
	string reply = http.endure (url);

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply);
	if (! error.empty ()) {
		cerr << __LINE__ << " " << error << endl;
		return set <string> {};
	}
	if (! reply_value.is <picojson::array> ()) {
		cerr << __LINE__ << endl;
		return set <string> {};
	}
	auto instances_array = reply_value.get <picojson::array> ();
	
	set <string> hosts;
	
	for (auto instance_value: instances_array) {
		if (instance_value.is <string> ()) {
			string instance_string = instance_value.get <string> ();
			hosts.insert (instance_string);
		}
	}
	
	return hosts;
}


HttpGlobal::HttpGlobal ()
{
	curl_global_init (CURL_GLOBAL_ALL);
}


Http::Http ()
{
	curl = curl_easy_init ();
	if (! curl) {
		throw (HttpException {__LINE__});
	}
}


string Http::perform (string url)
{
	return perform (url, vector <string> {});
}


string Http::perform (string url, vector <string> headers)
{
	CURLcode res;

	struct curl_slist * list = nullptr;
	for (auto &header: headers) {
		list = curl_slist_append (list, header.c_str ());
	}

	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
	string reply_1;
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, & reply_1);
	curl_easy_setopt (curl,  CURLOPT_CONNECTTIMEOUT, 60);
	curl_easy_setopt (curl,  CURLOPT_TIMEOUT, 60);
	curl_easy_setopt (curl,  CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt (curl,  CURLOPT_MAXREDIRS, 4L);
	res = curl_easy_perform (curl);
	long response_code;
	if (res == CURLE_OK) {
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, & response_code);
		if (response_code != 200) {
			cerr << "HTTP response: " << response_code << endl;
			throw (HttpException {__LINE__});
		}
	} else {
		throw (HttpException {__LINE__});
	}
	return reply_1;
}


string Http::endure (string url)
{
	CURLcode res;

	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, nullptr);
	string reply_1;
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, & reply_1);
	curl_easy_setopt (curl,  CURLOPT_CONNECTTIMEOUT, 600);
	curl_easy_setopt (curl,  CURLOPT_TIMEOUT, 600);
	curl_easy_setopt (curl,  CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt (curl,  CURLOPT_MAXREDIRS, 4L);
	res = curl_easy_perform (curl);
	long response_code;
	if (res == CURLE_OK) {
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, & response_code);
		if (response_code != 200) {
			cerr << "HTTP response: " << response_code << endl;
			throw (HttpException {__LINE__});
		}
	} else {
		throw (HttpException {__LINE__});
	}
	return reply_1;
}


Http::~Http ()
{
	curl_easy_cleanup (curl);
}


