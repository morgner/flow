= Setup DB
flask initdb

= SE Linux (like fedora)
# setenforce 0
# grep nginx /var/log/audit/audit.log | audit2allow -m nginx > nginx.te
# grep nginx /var/log/audit/audit.log | audit2allow -M nginx
# semodule -i nginx.pp
# setenforce 1

#!usermod -a -G uswgi-user-name nginx

= nginx
== nginx.conf (partial)

user nginx;

    server {
        listen       127.0.0.1:80;
        server_name  lenovum;

	location / {
       		try_files $uri @flaskr;
	}
	location @flaskr {
	    include uwsgi_params;
	    uwsgi_pass unix:/var/run/flaskr/flaskr.sock;
	}
    }

= virtual env ( "pip install --editable ." )
== Install Requirements

 * click
 * Flask
 * itsdangerous
 * Jinja2
 * MarkupSafe
 * Werkzeug

== Run uwsgi

uwsgi flaskr/flaskr.ini

