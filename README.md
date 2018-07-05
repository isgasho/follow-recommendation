# 分散SNSフォーラム

http://distsn.org

## 公開API

以下のURLは公開APIです。引数なしのGETメソッドを送ってください。JSON形式で、文字コードはUTF-8です。日時はUnix紀元からの通算秒数です。

* http://distsn.org/cgi-bin/distsn-instance-speed-api.cgi
* http://distsn.org/cgi-bin/distsn-instance-first-toot-api.cgi
* http://distsn.org/cgi-bin/distsn-pleroma-instances-api.cgi

## Install

    sudo apt install build-essential libcurl4-openssl-dev apache2
    make clean
    make
    sudo make initialize
    sudo make install
    sudo make activate-cgi

Write following code in crontab:

    10 */6  * * * /home/ubuntu/git/follow-recommendation/server/distsn-instance-speed-cron
    20 4    * * * /home/ubuntu/git/follow-recommendation/server/distsn-instance-first-toot-cron
    30 */3  * * * /home/ubuntu/git/follow-recommendation/server/distsn-pleroma-instances-cron

Write following code in crontab for the root:

    40 1 * * 2 /usr/local/bin/distsn-https-renew-cron

## Update

    make clean
    make
    sudo make refresh
    sudo make install
