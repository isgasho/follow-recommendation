/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-instance-lists-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var instances = JSON.parse (request.responseText);
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

html += '<tr>';
html += '<th>インスタンス</th>';
html += '<th><a href="https://github.com/TacosTea/versionbattle/blob/master/instances.list" target="_blank">TacosTea</a></th>';
html += '<th><a href="https://github.com/distsn/follow-recommendation/blob/master/hosts.txt" target="_blank">DistSN</a></th>';
html += '</tr>';

var cn;
for (cn = 0; cn < instances.length; cn ++) {
	var instance;
	instance = instances [cn];
	var instance_html =
		'<tr>' +
		'<td>' +
		'<a href="' +
		'instance-preview.html?' + encodeURIComponent (instance.domain) + '" target="distsn-instance-preview">' +
		escapeHtml (instance.domain) +
		'</a>' +
		'</td>' +
		'<td>' + (instance.tacostea? '✔': '') + '</td>' +
		'<td>' + (instance.distsn? '✔': '') + '</td>' +
		'</tr>';
	html += instance_html;
}

placeholder.innerHTML = html;
} /* function show_users (users) { */


