/* Follow recommendation */


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
	var thumbnail;
	if (instance.thumbnail && 0 < instance.thumbnail.length) {
		thumbnail = instance.thumbnail;
	} else {
		thumbnail = 'missing.svg';
	}
	var instance_html =
		'<p>' +
		'<a href="' +
		'https://' + encodeURIComponent (instance.domain) + '" target="_blank">' +
		'<img class="avatar" src="' + encodeURI (thumbnail) + '">' +
		'</a>' +
		'<a href="' +
		'https://' + encodeURIComponent (instance.domain) + '" target="_blank">' +
		escapeHtml (instance.domain) +
		'</a>' + '<br>' +
		escapeHtml (instance.title) +
		'</p>';
	html += instance_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


