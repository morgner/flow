# all the imports
import os
import sqlite3
from flask import Flask, request, session, g, redirect, url_for, abort, render_template, flash

app = Flask(__name__) #, instance_relative_config=True) # create the application instance :)
app.config.from_object(__name__) # load config from this file , moca.py

# Load default config and override config from an environment variable
app.config.update(dict(
#    DATABASE=os.path.join(app.root_path, 'moca.db'),
    SECRET_KEY='development key',
    USERNAME='admin',
    PASSWORD='default'
))
app.config.from_envvar('FLASKR_SETTINGS', silent=True)

@app.route('/')
def show_entries():
    entries = () #( [title,2], [text,"wir"] )
    return render_template('show_entries.html', entries=entries)

@app.route('/sign', methods=['POST'])
def sign_csr():
    flash('New CSR was successfully signed')
    return redirect(url_for('show_entries'))

