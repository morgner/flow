# Setup DB
flask initdb

#!usermod -a -G uswgi-user-name nginx

# nginx
## nginx.conf (partial)

user nginx;

    server {
        listen       127.0.0.1:80;
        server_name  localhost;

	location / {
            try_files $uri @flaskr;
	}
	location @flaskr {
	    include uwsgi_params;
#           ensure the dir is present, writable for 
#           the flask user and rw for nginx
	    uwsgi_pass unix:/var/run/flaskr/flaskr.sock;
	}
    }

## SE Linux (like fedora)
setenforce 0
grep nginx /var/log/audit/audit.log | audit2allow -m nginx > nginx.te
grep nginx /var/log/audit/audit.log | audit2allow -M nginx
semodule -i nginx.pp
setenforce 1

# virtualenv (optional)

pip install --editable .

# Run
## application

./run.sh

## uwsgi

./urun.sh

#uwsgi flaskr/flaskr.ini

