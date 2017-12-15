#! /usr/bin/env bash

#export PYTHONPATH=/usr/bin/:/usr/lib64/python2.7/:/usr/lib64/python2.7/sqlite3:$PYTHONPATH
#export LD_LIBRARY_PATH=/usr/lib64/python2.7/:/usr/lib64/python2.7/sqlite3:$LD_LIBRARY_PATH
#export PYTHONPATH=/usr/bin
#export FLASKR_SETTINGS=setup.py
#export FLASK_APP=flaskr

#export PYTHONPATH=/usr/bin:$PYTHONPATH
#export PYTHONHOME=/usr/lib64/python2.7:$PYTHONPATH

export PYTHONHOME=/usr/bin
export PYTHONPATH=/usr/lib/python2.7

# uwsgi -s /tmp/flaskr.sock --mount /flaskr=flaskr:app --processes 4 --threads 2 --stats localhost:9191
# uwsgi -s /tmp/flaskr.sock --manage-script-name --mount /=flaskr.flaskr:app --processes 4 --threads 2 
# uwsgi -s /tmp/flaskr.sock --master --manage-script-name --mount / flaskr:app --processes 4 --threads 2
# uwsgi -s /tmp/yourapplication.sock --manage-script-name --mount /yourapplication=myapp:app

uwsgi flaskr/flaskr.ini 
