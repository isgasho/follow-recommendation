# 分散SNSフォーラム推奨フォローリスト

流速と手動得点をもとに [推奨リスト](http://distsn.org) を作成しています。

インスタンスを追加または削除するには [hosts.txt](https://github.com/distsn/follow-recommendation/blob/master/hosts.txt) にプルリクを送ってください。ただし主たる言語が日本語であるインスタンスに限らせていただきます。

手動得点を更新するには [manual_score.txt](https://github.com/distsn/follow-recommendation/blob/master/manual-score.txt) にプルリクを送ってください。手動得点の基準は以下の通りです。

0: ボット、宣伝、マルチポスト (疑い含む)

1: 有害である、初心者向けでない、主たる言語が日本語でない、連投

2: 評価済み (デフォルト)

3: 良い

4: とても良い

## 公開API

以下のURLは公開APIです。引数なしのGETメソッドを送ってください。JSON形式で、文字コードはUTF-8です。日時はUnix紀元からの通算秒数、流速はトゥート/秒です。

* http://distsn.org/cgi-bin/distsn-user-recommendation-api.cgi
* http://distsn.org/cgi-bin/distsn-user-speed-api.cgi
* http://distsn.org/cgi-bin/distsn-instance-speed-api.cgi
* http://distsn.org/cgi-bin/distsn-instance-first-toot-api.cgi
