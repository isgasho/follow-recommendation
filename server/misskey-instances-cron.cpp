#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>

#include <socialnet-1.h>

#include "picojson.h"


using namespace std;


class Host {
public:
	string host_name;
	string title;
	string description;
	string thumbnail;
public:
	string version;
	bool registration;
	bool local_timeline;
	bool elasticsearch;
	bool recaptcha;
	bool object_storage;
	bool twitter;
	bool service_worker;
public:
	Host (string a_host_name, string a_title, string a_description, string a_thumbnail):
		host_name (a_host_name),
		title (a_title),
		description (a_description),
		thumbnail (a_thumbnail),
		registration (false),
		local_timeline (false),
		elasticsearch (false),
		recaptcha (false),
		object_storage (false),
		twitter (false),
		service_worker (false)
	{ };
	Host ():
		registration (false),
		local_timeline (false),
		elasticsearch (false),
		recaptcha (false),
		object_storage (false),
		twitter (false),
		service_worker (false)
	{ };
public:
	string format () const;
};


static string escape_json (string in)
{
	string out;
	for (auto c: in) {
		if (c == '\n') {
			out += string {"\\n"};
		} else if (c == '"'){
			out += string {"\\\""};
		} else if (c == '\\'){
			out += string {"\\\\"};
		} else {
			out.push_back (c);
		}
	}
	return out;
}


string Host::format () const
{
	string out;
	out += "{";

	out += "\"hostName\":\"" + escape_json (host_name) + "\",";
	out += "\"title\":\"" + escape_json (title) + "\",";
	out += "\"description\":\"" + escape_json (description) + "\",";
	out += "\"thumbnail\":\"" + escape_json (thumbnail) + "\",";

	out += "\"version\":\"" + escape_json (version) + "\",";
	out += "\"registration\":" + string {registration? "true": "false"} + ",";
	out += "\"localTimeline\":" + string {local_timeline? "true": "false"} + ",";
	out += "\"elasticsearch\":" + string {elasticsearch? "true": "false"} + ",";
	out += "\"recaptcha\":" + string {recaptcha? "true": "false"} + ",";
	out += "\"objectStorage\":" + string {object_storage? "true": "false"} + ",";
	out += "\"twitter\":" + string {twitter? "true": "false"} + ",";
	out += "\"serviceWorker\":" + string {service_worker? "true": "false"};
	
	out += "}";
	return out;
}


bool by_host_name (const Host &left, const Host &right)
{
	return left.host_name < right.host_name;
}


static void write_storage (FILE *out, vector <Host> hosts)
{
	fprintf (out, "[");
	for (unsigned int cn = 0; cn < hosts.size (); cn ++) {
		if (0 < cn) {
			fprintf (out, ",\n");
		}
		Host host = hosts.at (cn);
		fprintf (out, host.format ().c_str ());
	}
	fprintf (out, "]");
}


static void get_features (
	shared_ptr <socialnet::Http> http,
	string host_name,
	string & a_version,
	bool & a_registration,
	bool & a_local_timeline,
	bool & a_elasticsearch,
	bool & a_recaptcha,
	bool &a_object_storage,
	bool & a_twitter,
	bool & a_service_worker)
{
	string version;
	bool registration = false;
	bool local_timeline = false;
	bool elasticsearch = false;
	bool recaptcha = false;
	bool object_storage = false;
	bool twitter = false;
	bool service_worker = false;

	string url = string {"https://"} + host_name + string {"/api/meta"};
	
	string reply;
	
	vector <string> headers;
	string payload;
	reply = http->post (url, headers, payload);
	
	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply);
	if (! error.empty ()) {
		throw (socialnet::HostException {__LINE__});
	}
	if (! reply_value.is <picojson::object> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto reply_object = reply_value.get <picojson::object> ();

	if (reply_object.find (string {"version"}) != reply_object.end ()) {
		auto version_value = reply_object.at (string {"version"});
		if (version_value.is <string> ()) {
			string version_string = version_value.get <string> ();
			version = version_string;
		}
	}

	if (reply_object.find (string ("features")) != reply_object.end ()) {
		auto features_value = reply_object.at (string {"features"});
		if (! features_value.is <picojson::object> ()) {
			throw (socialnet::HostException {__LINE__});
		}
		auto features_object = features_value.get <picojson::object> ();

		if (features_object.find (string {"registration"}) != features_object.end ()
			&& features_object.at (string {"registration"}).is <bool> ()
		) {
			registration = features_object.at (string {"registration"}).get <bool> ();
		}

		if (features_object.find (string {"localTimeLine"}) != features_object.end ()
			&& features_object.at (string {"localTimeLine"}).is <bool> ()
		) {
			local_timeline = features_object.at (string {"localTimeLine"}).get <bool> ();
		}

		if (features_object.find (string {"elasticsearch"}) != features_object.end ()
			&& features_object.at (string {"elasticsearch"}).is <bool> ()
		) {
			elasticsearch = features_object.at (string {"elasticsearch"}).get <bool> ();
		}

		if (features_object.find (string {"recaptcha"}) != features_object.end ()
			&& features_object.at (string {"recaptcha"}).is <bool> ()
		) {
			recaptcha = features_object.at (string {"recaptcha"}).get <bool> ();
		}

		if (features_object.find (string {"objectStorage"}) != features_object.end ()
			&& features_object.at (string {"objectStorage"}).is <bool> ()
		) {
			object_storage= features_object.at (string {"objectStorage"}).get <bool> ();
		}

		if (features_object.find (string {"twitter"}) != features_object.end ()
			&& features_object.at (string {"twitter"}).is <bool> ()
		) {
			twitter = features_object.at (string {"twitter"}).get <bool> ();
		}

		if (features_object.find (string {"serviceWorker"}) != features_object.end ()
			&& features_object.at (string {"serviceWorker"}).is <bool> ()
		) {
			service_worker = features_object.at (string {"serviceWorker"}).get <bool> ();
		}
	}

	a_version = version;
	a_registration = registration;
	a_local_timeline = local_timeline;
	a_elasticsearch = elasticsearch;
	a_recaptcha = recaptcha;
	a_object_storage = object_storage;
	a_twitter = twitter;
	a_service_worker = service_worker;
}


static Host for_host (shared_ptr <socialnet::Host> socialnet_host)
{
	string title;
	string description;
	string thumbnail;
	try {
		socialnet_host->get_profile (title, description, thumbnail);
	} catch (socialnet::ExceptionWithLineNumber e) {
		/* Do nothing. */
	}

	Host host {socialnet_host->host_name, title, description, thumbnail};

	string version;
	bool registration = false;
	bool local_timeline = false;
	bool elasticsearch = false;
	bool recaptcha = false;
	bool object_storage = false;
	bool twitter = false;
	bool service_worker = false;
	try {
		get_features (
			socialnet_host->http,
			socialnet_host->host_name,
			version,
			registration,
			local_timeline,
			elasticsearch,
			recaptcha,
			object_storage,
			twitter,
			service_worker);
	} catch (socialnet::ExceptionWithLineNumber e) {
		/* Do nothing. */
	}
	host.version = version;
	host.registration = registration;
	host.local_timeline = local_timeline;
	host.elasticsearch = elasticsearch;
	host.recaptcha = recaptcha;
	host.object_storage = object_storage;
	host.twitter = twitter;
	host.service_worker = service_worker;

	return host;
}


int main (int argc, char **argv)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	vector <Host> hosts;
	for (auto socialnet_host: socialnet_hosts) {
		if (socialnet_host->implementation () == socialnet::eImplementation::MISSKEY) {
			cerr << socialnet_host->host_name << endl;
			try {
				Host host = for_host (socialnet_host);
				hosts.push_back (host);
			} catch (socialnet::HostException e) {
				cerr << "socialnet::HostException " << e.line << endl;
			}
		}
	}
	
	sort (hosts.begin (), hosts.end (), by_host_name);

	const string storage_filename = string {"/var/lib/distsn/misskey-instances.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

