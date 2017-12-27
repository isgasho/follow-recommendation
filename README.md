# 分散SNSフォーラム

http://distsn.org

インスタンスを追加または削除するには [hosts.txt](https://github.com/distsn/follow-recommendation/blob/master/hosts.txt) にプルリクを送ってください。ただし、主たる言語が日本語である、マストドンまたはPleromaのインスタンスに限らせていただきます。(GNU socialはAPIを取得できません。)

## 公開API

以下のURLは公開APIです。引数なしのGETメソッドを送ってください。JSON形式で、文字コードはUTF-8です。日時はUnix紀元からの通算秒数、流速はトゥート/秒です。

* http://distsn.org/cgi-bin/distsn-user-speed-api.cgi
* http://distsn.org/cgi-bin/distsn-instance-speed-api.cgi
* http://distsn.org/cgi-bin/distsn-instance-first-toot-api.cgi
