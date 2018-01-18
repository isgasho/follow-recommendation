#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include "picojson.h"
#include "distsn.h"


using namespace std;


class Host {
public:
	string domain;
	string title;
	string thumbnail;
public:
	Host (string a_domain, string a_title, string a_thumbnail) {
		domain = a_domain;
		title = a_title;
		thumbnail = a_thumbnail;
	};
};


bool by_domain (const Host &left, const Host &right)
{
	return left.domain < right.domain;
}


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
		fprintf
			(out,
			"{\"domain\":\"%s\",\"title\":\"%s\",\"thumbnail\":\"%s\"}",
			host.domain.c_str (),
			escape_json (host.title).c_str (),
			escape_json (host.thumbnail).c_str ());
	}
	fprintf (out, "]");
}


static Host for_host (string domain)
{
	string reply_string;

	try {
		reply_string = http_get_quick (string {"https://"} + domain + string {"/api/pleroma/emoji"});
	} catch (HttpException e) {
		cerr << domain << " has no /api/pleroma/emoji" << endl;
		throw (HostException {__LINE__});
	}

	picojson::value reply_value;
	string error = picojson::parse (reply_value, reply_string);
	if (! error.empty ()) {
		cerr << error << endl;
		throw (HostException {__LINE__});
	}

	string title;
	try {
		title = get_host_title (domain);
	} catch (HostException e) {
		/* Do nothing. */
	}

	string thumbnail;
	try {
		thumbnail = get_host_thumbnail (domain);
	} catch (HostException e) {
		/* Do nothing. */
	}

	return Host {domain, title, thumbnail};
}


int main (int argc, char **argv)
{
	set <string> domains;
	domains = get_international_hosts ();

	vector <Host> hosts;
	for (auto domain: domains) {
		cerr << domain << endl;
		try {
			Host host = for_host (string {domain});
			hosts.push_back (host);
		} catch (HttpException e) {
			/* Nothing. */
			cerr << "HttpException " << e.line << endl;
		} catch (HostException e) {
			/* Nothing. */
			cerr << "HostException " << e.line << endl;
		}
	}
	
	sort (hosts.begin (), hosts.end (), by_domain);

	const string storage_filename = string {"/var/lib/distsn/pleroma-instances.json"};
	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

