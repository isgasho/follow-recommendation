
window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-mastodon-apps-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var apps = JSON.parse (response_text);
				show_apps (apps);
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


function show_apps (apps) {
var placeholder = document.getElementById ('placeholder')
var html = ''
var cn
for (cn = 0; cn < apps.length && cn < 400; cn ++) {
	var app
	app = apps [cn]
	var app_html
	app_html = ''
	if (app.web) {
		app_html +=
			'<p>' +
			'<a class="headline" href="' +
			encodeURI (app.web) + '" target="_blank">' +
			escapeHtml (app.name) +
			'</a>' + ' ' +
			'(' + (app.share * 100).toFixed (3) + ' %)' +
			'</p>'
	} else {
		app_html +=
			'<p>' +
			'<span class="headline">' +
			escapeHtml (app.name) + ' ' +
			'<a href="https://www.google.com/search?q=' +
			encodeURIComponent (app.name) +
			'" target="_blank">🔍</a>' +
			'</span>' + ' ' +
			'(' + (app.share * 100).toFixed (3) + ' %)' +
			'</p>'
	}
	html += app_html
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


