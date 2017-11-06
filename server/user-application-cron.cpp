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


static bool valid_character (char c)
{
	return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || (c == '_');
}


static bool valid_username (string s)
{
	for (auto c: s) {
		if (! valid_character (c)) {
			return false;
		}
	}
	return true;
}


static string get_username (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"account"}) == properties.end ()) {
		throw (TootException {});
	}
	auto account = properties.at (string {"account"});
	if (! account.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto account_map = account.get <picojson::object> ();
	if (account_map.find (string {"username"}) == account_map.end ()) {
		throw (TootException {});
	}
	auto username = account_map.at (string {"username"});
	if (! username.is <string> ()) {
		throw (TootException {});
	}
	auto username_s = username.get <string> ();
	if (! valid_username (username_s)) {
		throw (TootException {});
	}
	return username_s;
}


static string get_application (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"application"}) == properties.end ()) {
		throw (TootException {});
	}
	auto application = properties.at (string {"application"});
	if (! application.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto application_map = application.get <picojson::object> ();
	if (application_map.find (string {"name"}) == application_map.end ()) {
		throw (TootException {});
	}
	auto name = application_map.at (string {"name"});
	if (! name.is <string> ()) {
		throw (TootException {});
	}
	return name.get <string> ();
}


static map <string, string> read_storage (FILE *in)
{
	string s;
	for (; ; ) {
		if (feof (in)) {
			break;
		}
		char b [1024];
		fgets (b, 1024, in);
		s += string {b};
	}
	picojson::value json_value;
	picojson::parse (json_value, s);
	auto object = json_value.get <picojson::object> ();
	
	map <string, string> memo;
	
	for (auto user: object) {
		string username = user.first;
		string application = user.second.get <string> ();
		memo.insert (pair <string, string> (username, application));
	}
	
	return memo;
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


static void write_storage (FILE *out, map <string, string> memo)
{
	fprintf (out, "{");
	for (auto user: memo) {
		string username = user.first;
		string application = user.second;
		fprintf (out, "\"%s\":\"%s\",", username.c_str (), escape_json (application).c_str ());
	}
	fprintf (out, "}");
}


static void for_host (string host)
{
	const string storage_filename = string {"/var/lib/distsn/user-application/"} + host;

	map <string, string> memo;

	FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
	if (storage_file_in != nullptr) {
		memo = read_storage (storage_file_in);
		fclose (storage_file_in);
	}

	/* Get timeline. */
	vector <picojson::value> toots = get_timeline (host);

	for (auto toot = toots.rbegin (); toot != toots.rend (); toot ++) {
		try {
			string username = get_username (* toot);
			string application = get_application (* toot);
			memo.insert (pair <string, string> {username, application});
		} catch (TootException e) {
			/* Do nothing. */
		}
	}
	
	/* Save memo. */
	{
		FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
		if (storage_file_out != nullptr) {
			write_storage (storage_file_out, memo);
			fclose (storage_file_out);
		}
	}
}


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


static set <string> get_domains ()
{
	string domains_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> domains;
	string domain;
	for (char c: domains_s) {
		if (c == '\n') {
			if (valid_host_name (domain)) {
				domains.insert (domain);
			}
			domain.clear ();
		} else {
			domain.push_back (c);
		}
	}
	if (valid_host_name (domain)) {
		domains.insert (domain);
	}
	return domains;
}


int main (int argc, char **argv)
{
	set <string> hosts = get_domains ();

	for (auto host: hosts) {
		try {
			for_host (string {host});
		} catch (HttpException e) {
			/* Nothing. */
		} catch (HostException e) {
			/* Nothing. */
		}
	}

	return 0;
}

