function showInstances (implementation, pleceholder) {
	var request = new XMLHttpRequest
	request.open ('GET', '/cgi-bin/distsn-instances-implementation-api.cgi?' + encodeURIComponent (implementation))
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText
				var instances = JSON.parse (response_text)
				showInstancesImpl (instances, placeholder)
			}
		}
	}
	request.send ()
}


function showInstancesImpl (instances, placeholder) {
	var html = ''
	var cn
	for (cn = 0; cn < instances.length; cn ++) {
		var instance;
		instance = instances [cn]
		var instance_html =
			'<p>' +
			'<a href="' +
			'https://' + encodeURIComponent (instance) +
			'" target="_blank">' +
			encodeURIComponent (instance) +
			'</a>' +
			'</p>'
		html += instance_html
	}
	placeholder.innerHTML = html;
}


