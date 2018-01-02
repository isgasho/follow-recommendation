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

	{
		string reply = http_get (string {"https://"} + host + string {"/api/v1/timelines/public?local=true&limit=40"});

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
		string reply = http_get (query);

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


string http_get (string url)
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


string http_get_quick (string url)
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
	curl_easy_setopt (curl,  CURLOPT_CONNECTTIMEOUT, 60);
	curl_easy_setopt (curl,  CURLOPT_TIMEOUT, 60);
	res = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	if (res != CURLE_OK) {
		throw (HttpException {__LINE__});
	}
	return reply_1;
}


string http_get (string url, vector <string> headers)
{
	CURL *curl;
	CURLcode res;
	curl_global_init (CURL_GLOBAL_ALL);

	curl = curl_easy_init ();
	if (! curl) {
		throw (HttpException {__LINE__});
	}
	
	struct curl_slist * list = nullptr;
	for (auto &header: headers) {
		list = curl_slist_append (list, header.c_str ());
	}
	
	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
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
	string reply = http_get (string {"https://"} + domain + string {"/api/v1/instance"});

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
	string reply = http_get (string {"https://"} + domain + string {"/api/v1/instance"});

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


/* Application name: vinayaka */
/* Application ID: 743317923 */
/* GadjL7MFZLb7h9zTdafxvEBRRsfbyGDyeLaExGF4abwTTRKzBY0mHShCUDwUV09qs03SJ3z9EYJvkDq82sBWll5wn8GBr37nRYDCOVE6K6Hcro2VRTDIeFLFVhlNsTQ5 */


static set <string> get_international_hosts_impl ()
{
	const string resource {"https://instances.social/api/1.0/instances/list"};
	const string parameters {"count=0&include_dead=false&include_down=true&include_closed=true"};
	const string url {resource + string {"?"} + parameters};
	vector <string> headers;
	const string token {"GadjL7MFZLb7h9zTdafxvEBRRsfbyGDyeLaExGF4abwTTRKzBY0mHShCUDwUV09qs03SJ3z9EYJvkDq82sBWll5wn8GBr37nRYDCOVE6K6Hcro2VRTDIeFLFVhlNsTQ5"};
	headers.push_back (string {"Authorization: Bearer "} + token);
	string reply = http_get (url, headers);

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply);
	if (! error.empty ()) {
		cerr << __LINE__ << " " << error << endl;
		return set <string> {};
	}
	if (! reply_value.is <picojson::object> ()) {
		cerr << __LINE__ << endl;
		return set <string> {};
	}
	auto reply_object = reply_value.get <picojson::object> ();
	if (reply_object.find (string {"instances"}) == reply_object.end ()) {
		cerr << __LINE__ << endl;
		return set <string> {};
	}
	auto instances_value = reply_object.at (string {"instances"});
	if (! instances_value.is <picojson::array> ()) {
		cerr << __LINE__ << endl;
		return set <string> {};
	}
	auto instances_array = instances_value.get <picojson::array> ();
	
	set <string> hosts;
	
	for (auto instance_value: instances_array) {
		if (instance_value.is <picojson::object> ()) {
			auto instance_object = instance_value.get <picojson::object> ();
			if (instance_object.find (string {"name"}) != instance_object.end ()) {
				auto name_value = instance_object.at (string {"name"});
				if (name_value.is <string> ()) {
					string name_string = name_value.get <string> ();
					hosts.insert (name_string);
				}
			}
		}
	}
	
	return hosts;
}


set <string> get_international_hosts ()
{
	set <string> hosts = get_international_hosts_impl ();
	
	/* Pleroma */
	hosts.insert (string {"pleroma.soykaf.com"});
	hosts.insert (string {"pleroma.knzk.me"});
	hosts.insert (string {"ketsuben.red"});
	hosts.insert (string {"plrm.ht164.jp"});
	hosts.insert (string {"pleroma.vocalodon.net"});
	hosts.insert (string {"plero.ma"});
	hosts.insert (string {"pleroma.taketodon.com"});
	
	/* Mastodon */
	hosts.insert (string {"2.distsn.org"});

	return hosts;
}


