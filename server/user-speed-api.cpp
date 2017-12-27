#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>
#include <limits>
#include "picojson.h"
#include "distsn.h"


using namespace std;


static map <string, double> read_storage (FILE *in)
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
	
	map <string, double> memo;
	
	for (auto user: object) {
		string username = user.first;
		double speed = user.second.get <double> ();
		memo.insert (pair <string, double> (username, speed));
	}
	
	return memo;
}


static map <string, string> read_storage_application (FILE *in)
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


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


class User {
public:
	string host;
	string username;
	double speed;
	unsigned int speed_order;
	string application;
	string avatar;
public:
	User (string a_host, string a_username, double a_speed, string a_application) {
		host = a_host;
		username = a_username;
		speed = a_speed;
		application  = a_application;
	};
};


class bySpeed {
public:
	bool operator () (const User &left, const User &right) const {
		return right.speed < left.speed;
	};
};


static set <string> get_hosts (string hosts_s)
{
	set <string> hosts;
	string host;
	for (char c: hosts_s) {
		if (c == '\n') {
			if (valid_host_name (host)) {
				hosts.insert (host);
			}
			host.clear ();
		} else {
			host.push_back (c);
		}
	}
	if (valid_host_name (host)) {
		hosts.insert (host);
	}
	return hosts;
}


static map <string, string> get_avatars ()
{
	try {
		string reply = http_get (string {"http://vinayaka.distsn.org/cgi-bin/vinayaka-user-profiles-api.cgi"});
		map <string, string> avatars;

		picojson::value json_value;
		string error = picojson::parse (json_value, reply);
		if (! error.empty ()) {
			cerr << error << endl;
			return map <string, string> {}; /* Silent error. */
		}
		if (! json_value.is <picojson::array> ()) {
			return map <string, string> {}; /* Silent error. */
		}

		auto users = json_value.get <picojson::array> ();
	
		for (auto user: users) {
			auto user_object = user.get <picojson::object> ();
			string host = user_object.at (string {"host"}).get <string> ();
			string user_name = user_object.at (string {"user"}).get <string> ();
			string avatar = user_object.at (string {"avatar"}).get <string> ();
			string key = user_name + string {"@"} + host;
			avatars.insert (pair <string, string> {key, avatar});
		}
		return avatars;
	} catch (HttpException e) {
		/* Silent error. */
	}
	return map <string, string> {};
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


int main (int argc, char **argv)
{
	unsigned int size = numeric_limits <unsigned int>::max ();
	if (1 < argc) {
		stringstream size_s {argv [1]};
		size_s >> size;
	}

	string hosts_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> hosts = get_hosts (hosts_s);
	
	vector <User> users;

	for (auto host: hosts) {
		map <string, double> speeds;
		map <string, string> applications;
		{
			const string storage_filename = string {"/var/lib/distsn/user-speed/"} + host;
			FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
			if (storage_file_in != nullptr) {
				speeds = read_storage (storage_file_in);
				fclose (storage_file_in);
			}
		}
		{
			const string storage_filename = string {"/var/lib/distsn/user-application/"} + host;
			FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
			if (storage_file_in != nullptr) {
				applications = read_storage_application (storage_file_in);
				fclose (storage_file_in);
			}
		}
		for (auto i: speeds) {
			string username = i.first;
			double speed = i.second;
			string application;
			if (applications.find (username) != applications.end ()) {
				application = applications.at (username);
			}
			User user {host, username, speed, application};
			users.push_back (user);
		}
	}
	
	{
		vector <User> active_users;
		for (auto i: users) {
			if (0.1 <= i.speed * 60 * 60 * 24) {
				active_users.push_back (i);
			}
		}
		users = active_users;
	}

	auto avatars = get_avatars ();
	for (auto &user: users) {
		string key = user.username + string {"@"} + user.host;
		if (avatars.find (key) != avatars.end ()) {
			user.avatar = avatars.at (key);
		}
	}

	sort (users.begin (), users.end (), bySpeed {});
	
	for (unsigned int cn = 0; cn < users.size (); cn ++) {
		users.at (cn).speed_order = cn;
	}

	cout << "Content-type: application/json" << endl << endl;

	cout << "[";
	
	for (unsigned int cn = 0; cn < users.size () && cn < size; cn ++) {
		auto user = users.at (cn);
		if (cn != 0) {
			cout << ",";
		}
		cout
			<< "{"
			<< "\"host\":\"" << user.host << "\","
			<< "\"username\":\"" << escape_json (user.username) << "\","
			<< "\"speed\":" << scientific << user.speed << ","
			<< "\"speed_order\":" << user.speed_order << ","
			<< "\"application\":\"" << escape_json (user.application) << "\","
			<< "\"avatar\":\"" << escape_json (user.avatar) << "\""
			<< "}";
	}

	cout << "]";

	return 0;
}

