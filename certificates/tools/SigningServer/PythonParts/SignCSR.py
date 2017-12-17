#! /usr/bin/env python

from OpenSSL import crypto

with open("server-CA.crt", "r") as ca_cert_file:
    ca_cert_text = ca_cert_file.read()
    ca_cert = crypto.load_certificate(crypto.FILETYPE_PEM, ca_cert_text)
#ca_cert = crypto.load_certificate(crypto.FILETYPE_PEM, "./server-CA.pem")

with open("server-CA.key", "r") as ca_key_file:
    ca_key_text = ca_key_file.read()
    ca_key = crypto.load_privatekey(crypto.FILETYPE_PEM, ca_key_text)
#ca_key  = crypto.load_privatekey (crypto.FILETYPE_PEM, "server-CA.key")

with open("localhost.csr", "r") as csr_file:
    csr_text = csr_file.read()
    req = crypto.load_certificate_request(crypto.FILETYPE_PEM, csr_text)
#req = crypto.load_certificate_request(crypto.FILETYPE_PEM, open("localhost.csr").read())

cert = crypto.X509()
#cert.set_subject(req.get_subject())
cert.set_serial_number(1)
cert.gmtime_adj_notBefore(0)
cert.set_notAfter(ca_cert.get_notAfter()) # - 1d)
cert.set_issuer(ca_cert.get_subject())
cert.set_pubkey(req.get_pubkey())
cert.sign(ca_key, "sha256")

print crypto.dump_certificate(crypto.FILETYPE_PEM, cert)

"""
== crypto

['CRL', 'CRLType', 'Error', 'FILETYPE_ASN1', 'FILETYPE_PEM', 'FILETYPE_TEXT', 'NetscapeSPKI', 'NetscapeSPKIType', 'PKCS12', 'PKCS12Type', 'PKCS7', 'PKCS7Type', 'PKey', 'PKeyType', 'Revoked', 'TYPE_DSA', 'TYPE_RSA', 'X509', 'X509Extension', 'X509ExtensionType', 'X509Name', 'X509NameType', 'X509Req', 'X509ReqType', 'X509Store', 'X509StoreContext', 'X509StoreContextError', 'X509StoreFlags', 'X509StoreType', 'X509Type', '_EllipticCurve', '_PY3', '_PassphraseHelper', '_UNSPECIFIED', '__builtins__', '__doc__', '__eq__', '__file__', '__ge__', '__gt__', '__le__', '__lt__', '__name__', '__ne__', '__package__', '_bio_to_string', '_byte_string', '_exception_from_error_queue', '_ffi', '_get_asn1_time', '_get_backend', '_integer_types', '_lib', '_make_assert', '_module', '_native', '_new_mem_buf', '_openssl_assert', '_raise_current_error', '_set_asn1_time', '_text_to_bytes_and_warn', '_text_type', '_untested_error', 'b16encode', 'datetime', 'deprecated', 'dsa', 'dump_certificate', 'dump_certificate_request', 'dump_crl', 'dump_privatekey', 'dump_publickey', 'get_elliptic_curve', 'get_elliptic_curves', 'load_certificate', 'load_certificate_request', 'load_crl', 'load_pkcs12', 'load_pkcs7_data', 'load_privatekey', 'load_publickey', 'partial', 'rsa', 'sign', 'verify', 'x509']

== crypto.X509()

['__class__', '__delattr__', '__dict__', '__doc__', '__format__', '__getattribute__', '__hash__', '__init__', '__module__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', '__weakref__', '_get_boundary_time', '_get_name', '_set_boundary_time', '_set_name', '_x509', 'add_extensions', 'digest', 'from_cryptography', 'get_extension', 'get_extension_count', 'get_issuer', 'get_notAfter', 'get_notBefore', 'get_pubkey', 'get_serial_number', 'get_signature_algorithm', 'get_subject', 'get_version', 'gmtime_adj_notAfter', 'gmtime_adj_notBefore', 'has_expired', 'set_issuer', 'set_notAfter', 'set_notBefore', 'set_pubkey', 'set_serial_number', 'set_subject', 'set_version', 'sign', 'subject_name_hash', 'to_cryptography']

"""
