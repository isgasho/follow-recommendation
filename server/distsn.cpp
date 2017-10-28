#include <sstream>
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
	if (! id_object.is <double> ()) {
		throw (TootException {});
	}
	double id_double = id_object.get <double> ();
	stringstream s;
	s << static_cast <unsigned int> (id_double);
	return s.str ();
}


