
function mediaProxy (image) {
	return 'https://images.weserv.nl/?url=' +
		encodeURIComponent (image) +
		'&errorredirect=' + encodeURIComponent ('https://distsn.org/missing.png')
}


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-pleroma-instances-api.cgi');
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
var placeholder = document.getElementById ('placeholder');
var html = '';
var cn;
for (cn = 0; cn < instances.length; cn ++) {
	var instance;
	instance = instances [cn];
	if (instance.registration) {
		var thumbnail;
		if (instance.thumbnail && 0 < instance.thumbnail.length) {
			thumbnail = mediaProxy (instance.thumbnail)
		} else {
			thumbnail = 'missing.png';
		}
		var instance_html
		instance_html = ''
		instance_html +=
			'<p>' +
			'<a href="' +
			'https://' + encodeURIComponent (instance.domain) + '" target="_blank">' +
			'<img class="avatar" src="' + thumbnail + '">' +
			'</a>' +
			'<a href="' +
			'https://' + encodeURIComponent (instance.domain) + '" target="_blank">' +
			escapeHtml (instance.domain) +
			'</a>' + '<br>'
		instance_html +=
			(instance.registration? '📛': '🚫') + ' ' +
			(instance.chat? '💬': '🚫') + ' ' +
			(instance.gopher? '📟': '🚫') + ' ' +
			(instance.who_to_follow? '👥': '🚫') + ' ' +
			(instance.media_proxy? '🕵️': '🚫') + ' ' +
			(instance.scope_options? '🔏': '🚫') + ' ' +
			'🖊️=' + instance.text_limit +
			'<br>'
		instance_html +=
			escapeHtml (instance.title) +
			'</p>';
		html += instance_html
	}
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


