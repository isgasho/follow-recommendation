
var g_distsn_domain = '';


function get_random_instance (continuation) {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-instance-speed-api.cgi');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var instances = JSON.parse (response_text);
				var good_instances = [];
				for (var cn = 0; cn < instances.length; cn ++) {
					var instance = instances[cn];
					if (0.1 <= instance.speed * 60 * 60) {
						good_instances.push (instance);
					}
				}
				var random_number = Math.floor (Math.random () * good_instances.length);
				var domain_name = good_instances[random_number].domain;
				continuation (domain_name);
			}
		}
	}
	request.send ();
}


window.addEventListener ('load', function () {
var domain = window.location.search.replace (/^\?/, '');
	if (domain) {
		g_distsn_domain = domain;
		get_instance (domain);
		get_timeline (domain, 1);
	} else {
		get_random_instance (function (domain_name) {
			history.replaceState (null, '', window.location.href + '?' + domain_name);
			g_distsn_domain = domain_name;
			get_instance (domain_name);
			get_timeline (domain_name, 1);
		});
	}
}, false); /* window.addEventListener ('load', function () { */


window.addEventListener ('load', function () {
document.getElementById ('random-button').addEventListener ('click', function () {
	get_random_instance (function (domain_name) {
		window.location.search = '?' + domain_name;
	});
}, false);
}, false); /* window.addEventListener ('load', function () { */


function get_instance (domain) {
	var url ='https://' + domain + '/api/v1/instance';
	var request = new XMLHttpRequest ();
	request.onreadystatechange = function () {
		if (request.readyState == 4) {
			if (request.status == 200) {
				show_instance (request.response);
			} else {
				document.getElementById ('placeholder-instance').innerHTML =
					'<h1>' +
					'<a href="' + 'https://' + domain +'">' +
					escapeHtml (domain) +
					'</a>' +
					'</h1>' +
					'<p><strong>インスタンスの情報を取得できませんでした。</strong></p>';
			}
		}
	};
	request.responseType = 'json';
	request.open ('GET', url, true);
	request.send ();
}


function show_instance (response) {
	var html = '';
	html += '<h1>';
	if (response.uri.startsWith ('https://')) {
		html += '<a href="' + response.uri + '" target="_blank">';
	} else {
		html += '<a href="https://' + response.uri + '" target="_blank">';
	}
	html += escapeHtml (response.title);
	html += '</a>';
	html += '</h1>';
	html += response.description;
	var placeholder = document.getElementById ('placeholder-instance');
	placeholder.innerHTML = html;
};


function get_timeline (domain, depth) {
	document.getElementById ('loading-message').removeAttribute ('style');
	get_timeline_impl (domain, depth, [], 0);
};


function get_timeline_impl (domain, depth, toots, a_bottom_id) {
	if (depth <= 0) {
		show (toots);
		document.getElementById ('loading-message').setAttribute ('style', 'display:none;');
		return;
	}
	var url ='https://' + domain + '/api/v1/timelines/public?local=true&limit=40';
	if (0 < a_bottom_id) {
		url += '&max_id=' + a_bottom_id;
	}
	var request = new XMLHttpRequest ();
	request.onreadystatechange = function () {
		if (request.status == 200 && request.readyState == 4) {
			if (0 < request.response.length) {
				var bottom_id = request.response [request.response.length - 1].id;
				get_timeline_impl (domain, depth - 1, toots.concat (request.response), bottom_id);
			} else {
				show (toots);
				document.getElementById ('loading-message').setAttribute ('style', 'display:none;');
			}
		}
	};
	request.responseType = 'json';
	request.open ('GET', url, true);
	request.send ();
}


function show (response) {
	show_applications (response);
	show_local_timeline (response);
};


function show_applications (response) {
	var occupancy = {};
	for (cn = 0; cn < response.length; cn ++) {
		var toot = response [cn];
		if (toot.application && toot.application.name) {
			var application = toot.application.name;
			if (occupancy [application]) {
				occupancy [application] = occupancy [application] + 1;
			} else {
				occupancy [application] = 1;
			}
		}
	}
	var table = [];
	for (var application in occupancy) {
		table.push ({'application': application, 'occupancy': occupancy [application]});
	};
	table.sort (function (a, b) {return b.occupancy - a.occupancy; });
	var total = response.length;
	var html = '';
	for (var row in table) {
		html +=
			'<p>' +
			escapeHtml (table [row].application) +
			'<br>' +
			table [row].occupancy + ' ' +'トゥート' + ' ' +
			'(' + ((table [row].occupancy / total) * 100).toFixed (1) + ' %)' +
			'</p>';
	};
	var placeholder = document.getElementById ('placeholder-applications');
	placeholder.innerHTML = html;
};


function show_local_timeline (response) {
	var html;
	var cn;
	html = '';
	for (cn = 0; cn < response.length; cn ++) {
		var toot = response [cn];
		html += show_toot (toot);
	}
	var placeholder = document.getElementById ('placeholder-toots');
	placeholder.innerHTML = html;
};


function show_toot (toot) {
	var html = '';
	html += '<div class="toot">';
	html += '<a href="' + toot.account.url + '" target="distsn-preview">';
	html += '<img src="' + toot.account.avatar + '" width="40" height="40">';
	html += escapeHtml (toot.account.display_name);
	html += '</a>';
	html += '&emsp;';
	html += '<small>';
	html += '<a href="' + toot.url + '" target="distsn-preview">';
	html += (new Date (toot.created_at));
	html += '</a>';
	if (toot.application && toot.application.name) {
		html += '&emsp;';
		html += escapeHtml (toot.application.name);
	}
	html += '</small>';
	if (toot.content.startsWith ('<p>')) {
		html += toot.content;
	} else {
		html += '<p>' + toot.content + '</p>';
	}
	var attachments = toot.media_attachments;
	if (attachments && 0 < attachments.length) {
		var cn;
		for (cn = 0; cn < attachments.length; cn ++) {
			var attachment = attachments [cn];
			if (attachment.type === 'image') {
				html += '<a href="' + (attachment.remote_url? attachment.remote_url: attachment.url) + '" target="distsn-preview">';
				html += '<img class="preview" src="' + attachment.preview_url + '">';
				html += '</a>';
				html += ' ';
			}
		}
	}
	html += '</div>';
	return html;
};


function escapeHtml (text) {
		text = text.replace (/\&/g, '&amp;');
		text = text.replace (/\</g, '&lt;');
		text = text.replace (/\>/g, '&gt;');
		return text;
};


window.show400 = function () {
	document.getElementById ('a-400').removeAttribute ('href');
	window.open ('https://enty.jp/distsn', 'distsn-donation');
	get_timeline (g_distsn_domain, 10);
};


window.show1000 = function () {
	document.getElementById ('a-400').removeAttribute ('href');
	document.getElementById ('a-1000').removeAttribute ('href');
	window.open ('https://enty.jp/distsn', 'distsn-donation');
	get_timeline (g_distsn_domain, 25);
};


window.addEventListener ('load', function () {
document.getElementById ('move-button').addEventListener ('click', function () {
	var responce = prompt ('ドメイン名を入力してください。(例: mstdn.jp)');
	if (responce) {
		window.location.search = '?' + responce;
	}
}, false);
}, false);


