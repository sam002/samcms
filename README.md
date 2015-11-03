#samcms

[![Join the chat at https://gitter.im/sam002/samcms](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/sam002/samcms?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
Web-CMS for postgresql. Written in pgsql and C.

##Preparations of code in progress...

Please wait some time!

##How run?
Now debugging code... Step by step

###Prepare

Add and edit to nginx config form ./samcms/tools/samcms-nginx
```bash
sudo cp ./samcms/tools/samcms-nginx /etc/nginx/sites-avalible/
sudo ln -s /etc/nginx/sites-available/samcms-nginx /etc/nginx/sites-enabled/
#nano /etc/nginx/sites-available/samcms-nginx
sudo nginx -t
service nginx restart
```
Install dependency packages (example tested only for kubuntu 14.10)
```bash
sudo apt-get install postgresql-9.4 postgresql-client-9.4 postgresql-contrib-9.4\
  libpq-dev. libfcgi0ldbl make gcc
```

###Run it!
Goto repo root and compiling with Make
```bash
cd ./samcms
#make uninstall ; make clean ; make
make
```
For last step run in debug mode
```bash
./develop/usr/bin/samcms -n -a 127.0.0.1 -p 7777 -u www-data -g www-data \
-- ./develop/usr/bin/samcms-index -d samcms -u samcms -w samcms -a 127.0.0.1
```

And test with curl))
```bash
curl -v "http://127.0.0.1:7770/test?123"
```
###Not sure I remembered all the details - you can trying.

====
