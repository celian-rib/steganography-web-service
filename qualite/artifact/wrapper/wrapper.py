#!/usr/bin/python3
import tempfile
import os
import sys
import logging
logger = logging.basicConfig(level=logging.INFO)

BINARY_NAME='steganongraphy-cli'
BINARIES_PATH='./binaries'

def get_os_name():
  with open('/etc/os-release', 'r') as f:
    data = f.read()
    if 'Ubuntu' in data:
        return 'UBUNTU'
    elif 'Alpine' in data:
        return 'ALPINE'

osname = get_os_name()

if osname == 'UBUNTU':
    BINARY_PATH='%s/ubuntu-focal-%s'%(BINARIES_PATH, BINARY_NAME)
elif osname == 'ALPINE':
    BINARY_PATH='%s/alpine-%s'%(BINARIES_PATH, BINARY_NAME)
else:
    logging.error('Unknown OS: %s'%osname)
    sys.exit(-1)

if os.path.isfile(BINARY_PATH):
    logging.info('Using binary %s'%BINARY_PATH)
else:
    logging.error('Binary not found %s'%BINARY_PATH)
    sys.exit(-1)




PORT = 9090

from subprocess import check_output, CalledProcessError

def run(cmd):
    try:
        out = check_output(cmd)
        return 0, out.decode()
    except CalledProcessError as e:
        return e.returncode, '%s'%e


from flask import Flask, request, Response, send_from_directory
app = Flask(__name__, static_url_path='/')


def get_extension(path):
    return '.%s'%path.split('.')[-1]

@app.route('/api/encode', methods=['POST'])
def encode():
    extension = get_extension(request.files['input'].filename)
    with tempfile.NamedTemporaryFile(suffix=extension) as inputfile:
        inputfile.write(request.files['input'].read())
        with tempfile.NamedTemporaryFile(suffix=extension) as outputfile:
            cmd = [
                    BINARY_PATH,
                    '--encode',
                    inputfile.name,
                    request.form["text"],
                    outputfile.name
                ]
            rvalue, out = run(cmd)
            if(rvalue != 0):
                return out, 400
            r = Response(outputfile.read(), mimetype='image/bmp')
            r.headers['Content-Disposition'] = 'attachment'
            return r


@app.route('/api/decode', methods=['POST'])
def decode():
    extension = get_extension(request.files['input'].filename)
    with tempfile.NamedTemporaryFile(suffix=extension) as inputfile:
        inputfile.write(request.files['input'].read())
        cmd = [
                BINARY_PATH,
                '--decode',
                inputfile.name,
            ]
        rvalue, out = run(cmd)
        if(rvalue != 0):
            return out, 400
        return out

@app.route('/<path:path>')
@app.route('/',defaults={'path': ''})
def send_file(path):
    if not '--test' in sys.argv:
            return 'test mode disabled, please run with --test', 403
    if path == '':
        path = 'index.html'
    content = open('./www/%s'%path).read()
    return Response(content)

app.run(host='0.0.0.0', port=PORT)

