#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
#include <tuple>
#include <fstream>
#include <algorithm>

#include <socialnet-1.h>


using namespace std;


class App {
public:
	string name;
	string web;
public:
	App () {};
	App (string a_name, string a_web): name (a_name), web (a_web) {};
public:
	bool operator < (const App &r) const {
		return tuple <string, string> {name, web} < tuple <string, string> {r.name, r.web};
	};
};


class AppAndShare {
public:
	App app;
	double share;
public:
	AppAndShare () {};
	AppAndShare (App a_app, double a_share): app (a_app), share (a_share) {};
};


bool by_share (const AppAndShare &a, const AppAndShare &b)
{
	return tuple <double, App> {- a.share, a.app} < tuple <double, App> {- b.share, b.app}; 
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


static void write_storage (vector <AppAndShare> apps_and_share, string filename)
{
	ofstream out {filename};
	out << "[";
	for (unsigned int cn = 0; cn < apps_and_share.size (); cn ++) {
		if (0 < cn) {
			out << "," << endl;
		}
		auto app_and_share = apps_and_share.at (cn);
		auto app = app_and_share.app;
		out << "{";
		out << "\"name\":\"" << escape_json (app.name) << "\",";
		out << "\"web\":\"" << escape_json (app.web) << "\",";
		out << "\"share\":" << app_and_share.share;
		out << "}";
	}
	out << "]" << endl;
}


static void for_implementation (socialnet::eImplementation a_implementation, string filename)
{
	auto socialnet_hosts = socialnet::get_hosts ();

	map <App, unsigned int> apps_to_occupancy;

	for (auto socialnet_host: socialnet_hosts) {
		if (socialnet_host->implementation () == a_implementation) {
			cerr << socialnet_host->host_name << endl;
			try {
				auto timeline = socialnet_host->get_local_timeline (24 * 60 * 60);
				for (auto toot: timeline) {
					string app_name = toot.app_name;
					string app_web = toot.app_web;
					if ((! app_name.empty ()) && app_name != string {"Web"}) {
						App app {app_name, app_web};
						if (apps_to_occupancy.find (app) == apps_to_occupancy.end ()) {
							apps_to_occupancy.insert (pair <App, unsigned int> {app, 1});
						} else {
							apps_to_occupancy.at (app) ++;
						}
					}
				}
			} catch (socialnet::ExceptionWithLineNumber e) {
				cerr << "Exception " << e.line << endl;
			}
		}
	}

	unsigned int total_occupancy = 0;
	for (auto app_to_occupancy: apps_to_occupancy) {
		unsigned int occupancy {app_to_occupancy.second};
		total_occupancy += occupancy;
	}

	vector <AppAndShare> apps_and_share;

	for (auto app_to_occupancy: apps_to_occupancy) {
		App app {app_to_occupancy.first};
		unsigned int occupancy {app_to_occupancy.second};
		double share {static_cast <double> (occupancy) / static_cast <double> (total_occupancy)};
		apps_and_share.push_back (AppAndShare {app, share});
	}

	sort (apps_and_share.begin (), apps_and_share.end (), by_share);

	write_storage (apps_and_share, filename);
}


int main (int argc, char **argv)
{
	const string filename = string {"/var/lib/distsn/mastodon-apps.json"};
	for_implementation (socialnet::eImplementation::MASTODON, filename);
}

