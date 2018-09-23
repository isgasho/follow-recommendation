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
#include "distsn.h"


using namespace std;


class Host {
public:
	string domain;
	unsigned int toots_per_week;
	string title;
	string thumbnail;
public:
	Host (string a_domain, unsigned int a_toots_per_week, string a_title, string a_thumbnail) {
		domain = a_domain;
		toots_per_week = a_toots_per_week;
		title = a_title;
		thumbnail = a_thumbnail;
	};
};


class bySpeed {
public:
	bool operator () (const Host &left, const Host &right) const {
		return right.toots_per_week < left.toots_per_week;
	};
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


static void write_storage (FILE *out, vector <Host> hosts)
{
	fprintf (out, "[");
	for (unsigned int cn = 0; cn < hosts.size (); cn ++) {
		if (0 < cn) {
			fprintf (out, ",");
		}
		Host host = hosts.at (cn);
		double speed = static_cast <double> (host.toots_per_week) / static_cast <double> (7 * 24 * 60 * 60);
		fprintf
			(out,
			"{\"domain\":\"%s\",\"speed\":%e,\"title\":\"%s\",\"thumbnail\":\"%s\",\"toots_per_week\":%u}",
			host.domain.c_str (),
			speed,
			escape_json (host.title).c_str (),
			escape_json (host.thumbnail).c_str (),
			host.toots_per_week);
	}
	fprintf (out, "]");
}


static Host for_host (shared_ptr <socialnet::Host> socialnet_host)
{
	/* Get weekly number of toots. */
	string reply_string = socialnet_host->http->perform
		(string {"https://"} + socialnet_host->host_name + string {"/api/v1/instance/activity"});

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply_string);
	if (! error.empty ()) {
		throw (socialnet::HostException {__LINE__});
	}
	if (! reply_value.is <picojson::array> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto reply_array = reply_value.get <picojson::array> ();
	if (reply_array.size () < 2) {
		throw (socialnet::HostException {__LINE__});
	}
	auto last_week_value = reply_array.at (1);
	if (! last_week_value.is <picojson::object> ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto last_week_object = last_week_value.get <picojson::object> ();
	if (last_week_object.find (string {"statuses"}) == last_week_object.end ()) {
		throw (socialnet::HostException {__LINE__});
	}
	auto statuses_value = last_week_object.at (string {"statuses"});
	unsigned int statuses_uint = 0;
	if (statuses_value.is <string> ()) {
		string statuses_string = statuses_value.get <string> ();
		stringstream {statuses_string} >> statuses_uint;
	} else if (statuses_value.is <double> ()) {
		double statuses_double = statuses_value.get <double> ();
		statuses_uint = static_cast <unsigned int> (statuses_double);
	} else {
		throw (socialnet::HostException {__LINE__});
	}

	string title;
	string description;
	string thumbnail;

	try {
		socialnet_host->get_profile (title, description, thumbnail);
	} catch (socialnet::HostException e) {
		/* Do nothing. */
	}

	return Host {socialnet_host->host_name, statuses_uint, title, thumbnail};
}


int main (int argc, char **argv)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	vector <Host> hosts;
	for (auto socialnet_host: socialnet_hosts) {
		if (socialnet_host->implementation () == socialnet::eImplementation::MASTODON) {
			cerr << socialnet_host->host_name << endl;
			try {
				Host host = for_host (socialnet_host);
				hosts.push_back (host);
				cerr << "TPW: " << host.toots_per_week << endl;
			} catch (socialnet::HttpException e) {
				/* Nothing. */
				cerr << "HttpException " << e.line << endl;
			} catch (socialnet::HostException e) {
				/* Nothing. */
				cerr << "socialnet::HostException " << e.line << endl;
			}
		} else {
			cerr << socialnet_host->host_name << " is not Mastodon." << endl;
		}
	}

	sort (hosts.begin (), hosts.end (), bySpeed {});

	const string storage_filename = string {"/var/lib/distsn/instance-speed/instance-speed.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

