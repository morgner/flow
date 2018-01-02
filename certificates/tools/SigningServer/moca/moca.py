# all the imports
import os
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
    return render_template('index.html', entries=entries)

from OpenSSL import crypto
import os

@app.route('/sign', methods=['POST'])
def sign_csr():

    # print "[", request.form['signer'], "]"

    ca = "server-CA"
    msg = u'Server'
    if ( request.form['signer'] == u'client' ):
        ca = "client-CA"
        msg = u'Client'
    fname_cac = ca + ".crt"
    fname_cak = ca + ".key"
    fname_car = u'cert.flow.info.crt'
    flash( msg + u' certificate' )

    REQS = u''
    CERT = u'no certificate'

    CSR = request.form['csr']

    runOk = True

    if not (os.path.isfile(fname_car)):
        print "ERROR: Root certificate file ", fname_car, " does not exist"
        runOk = False

    if not (os.path.isfile(fname_cac)):
        print "ERROR: CA certificate file ", fname_cac, " does not exist"
        runOk = False

    if not (os.path.isfile(fname_cak)):
        print "ERROR: CA key file ", fname_cak, " does not exist"
        runOk = False

    if (runOk):
        with open(fname_car, "r") as ca_root_file:
            ca_root_text = ca_root_file.read()

        with open(fname_cac, "r") as ca_cert_file:
            ca_cert_text = ca_cert_file.read()
            ca_cert = crypto.load_certificate(crypto.FILETYPE_PEM, ca_cert_text)

        with open(fname_cak, "r") as ca_key_file:
            ca_key_text = ca_key_file.read()
            ca_key = crypto.load_privatekey(crypto.FILETYPE_PEM, ca_key_text)

        csr_text = CSR
        req = crypto.load_certificate_request(crypto.FILETYPE_PEM, csr_text)

        cert = crypto.X509()
        cert.set_subject(req.get_subject())
        cert.set_serial_number(1)
        cert.gmtime_adj_notBefore(0)
        cert.set_notAfter(ca_cert.get_notAfter()) # - 1d)
        cert.set_issuer(ca_cert.get_subject())
        cert.set_pubkey(req.get_pubkey())
        cert.sign(ca_key, "sha256")

        REQS = req.get_subject()
        CERT = crypto.dump_certificate(crypto.FILETYPE_PEM, cert)

        print cert.get_notAfter()
        print REQS.get_components()
        REQS = REQS.get_components()
        # print CERT

    #return redirect(url_for('show_entries'))
    return render_template('signed.html', entries=( { 'REQS':REQS, 'CERT':CERT, 'CA':ca_cert_text, 'ROOT':ca_root_text } ) )

