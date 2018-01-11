/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-instance-speed-api.cgi');
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
		'instance-preview.html?' + instance.domain + '" target="distsn-instance-preview">' +
		'<img class="avatar" src="' + thumbnail + '">' +
		'</a>' +
		'<a href="' +
		'instance-preview.html?' + instance.domain + '" target="distsn-instance-preview">' +
		(instance.title? escapeHtml (instance.title): instance.domain) +
		'</a>' +
		'<br>' +
		(instance.title && instance.title !== instance.domain? instance.domain + '<br>': '') +
		instance.toots_per_week + ' トゥート/週 (' + (cn + 1) + ' 位)' +
		'</p>';
	html += instance_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


