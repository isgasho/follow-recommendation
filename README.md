# 分散SNSフォーラム

https://distsn.org

## 公開API

以下のURLは公開APIです。引数なしのGETメソッドを送ってください。JSON形式で、文字コードはUTF-8です。日時はUnix紀元からの通算秒数です。

* https://distsn.org/cgi-bin/distsn-gnusocial-instances-api.cgi
* https://distsn.org/cgi-bin/distsn-pleroma-instances-api.cgi
* https://distsn.org/cgi-bin/distsn-misskey-instances-api.cgi
* https://distsn.org/cgi-bin/distsn-instance-first-toot-api.cgi
* https://distsn.org/cgi-bin/distsn-instance-speed-api.cgi
* https://distsn.org/cgi-bin/distsn-mastodon-apps-api.cgi

## Install

Depends on: https://gitlab.com/distsn/libsocialnet

    sudo apt install build-essential libcurl4-openssl-dev apache2
    make clean
    make
    sudo make initialize
    sudo make install
    sudo make activate-cgi

Write following code in crontab:

```
6  */4 * * * /usr/local/bin/distsn-gnusocial-instances-cron
12 */4 * * * /usr/local/bin/distsn-pleroma-instances-cron
18 */4 * * * /usr/local/bin/distsn-misskey-instances-cron
24 4   * * * /usr/local/bin/distsn-instance-first-toot-cron
30 */4 * * * /usr/local/bin/distsn-instance-speed-cron
```

Write following code in crontab for the root:

    40 1 * * 2 /usr/local/bin/distsn-https-renew-cron

## Update

    make clean
    make
    sudo make refresh
    sudo make install
