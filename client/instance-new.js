
function mediaProxy (image) {
	return 'https://images.weserv.nl/?url=' +
		encodeURIComponent (image.replace (/^http(s)?\:\/\//, '')) +
		'&errorredirect=' + encodeURIComponent ('distsn.org/missing.png')
}


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-instance-first-toot-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var instances = JSON.parse (response_text);
				show_instances (instances);
			}
		}
	}
	request.send ();
}, false); /* window.addEventListener ('load', function () { */


function escapeHtml (text) {
        text = text.replace (/\&/g, '&amp;');
        text = text.replace (/\</g, '&lt;');
        text = text.replace (/\>/g, '&gt;');
        return text;
};


function show_instances (instances) {
var placeholder = document.getElementById ('placeholder-instances');
var html = '';
var cn;
for (cn = 0; cn < instances.length; cn ++) {
	var instance;
	instance = instances [cn];
	var thumbnail;
	if (instance.thumbnail && 0 < instance.thumbnail.length) {
		thumbnail = mediaProxy (instance.thumbnail)
	} else {
		thumbnail = 'missing.svg';
	}
	var instance_html =
		'<p>' +
		'<a href="' +
		'https://mstpubapi.herokuapp.com/instance?host=' + encodeURIComponent (instance.domain) + '" target="_blank">' +
		'<img class="avatar" src="' + thumbnail + '">' +
		'</a>' +
		'<a class="headline" href="' +
		'https://mstpubapi.herokuapp.com/instance?host=' + encodeURIComponent (instance.domain) + '" target="_blank">' +
		escapeHtml (instance.title? instance.title: instance.domain) +
		'</a>' +
		'<br>' +
		(instance.title && instance.title !== instance.domain? escapeHtml (instance.domain) + '<br>': '') +
		'<a href="' + encodeURI (instance.first_toot_url) + '" target="distsn-preview">' +
		(new Date (1000 * instance.first_toot_time)) +
		'</a>' +
		'</p>';
	html += instance_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


