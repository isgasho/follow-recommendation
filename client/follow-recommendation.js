/* Follow recommendation */


window.addEventListener ('load', function () {
	var request = new XMLHttpRequest;
	request.open ('GET', '/cgi-bin/distsn-user-speed-api.cgi?200');
	request.onload = function () {
		if (request.readyState === request.DONE) {
			if (request.status === 200) {
				var response_text = request.responseText;
				var users = JSON.parse (response_text);
				show_users (users);
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


function show_users (users) {
var placeholder = document.getElementById ('placeholder');
var html = '';
var cn;
for (cn = 0; cn < users.length; cn ++) {
	var user;
	user = users [cn];
	var avatar;
	if (user.avatar && 0 < user.avatar.length) {
		avatar = user.avatar;
	} else {
		avatar = 'missing.svg';
	}
	var user_html =
		'<p>' +
		'<a href="' +
		'https://' + user.host + '/users/' + user.username +
		'" target="distsn-external-user-profile">' +
		'<img class="avatar" src="' + avatar + '">' +
		'</a> ' +
		'<a href="' +
		'https://' + user.host + '/users/' + user.username +
		'" target="distsn-external-user-profile">' +
		user.username + '@<wbr>' + user.host +
		'</a>' +
		'<br>' +
		(user.speed * 60 * 60 * 24).toFixed (1) + ' トゥート/日 (' + (user.recommendation_order + 1) + ' 位)' +
		(user.application? '<br>' + escapeHtml (user.application): '') +
		'</p>';
	html += user_html;
}
placeholder.innerHTML = html;
} /* function show_users (users) { */


