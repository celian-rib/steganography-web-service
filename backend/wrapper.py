import os
import subprocess
import tempfile
from subprocess import check_output, CalledProcessError

from flask import Flask, request, jsonify

app = Flask(__name__)
BINARY_PATH='./binaries/alpine-steganongraphy-cli'

def run(cmd):
    try:
        out = check_output(cmd)
        return 0, out.decode()
    except CalledProcessError as e:
        return e.returncode, '%s'%e

@app.route('/ping')
def ping():
    return 'pong'

@app.route('/encode', methods=['POST'])
def encode():
    if(request.method == 'POST'):
        if('file' not in request.files or 'text' not in request.form):
            return jsonify({'error': 'Missing file or text'}), 418
        request.files['file'].save(os.path.join(os.getcwd(), 'temp.png'))
        stream = os.system('./main -encode temp.png "' + request.form['text'] + '" tempoutput.png')
        output = open('tempoutput.png', 'rb').read()
        #make that the output is a download header
        return output, 200, {'Content-Type': 'image/png', 'Content-Disposition': 'attachment; filename=output.png'}
        return output, 200, {'Content-Type': 'image/png', 'Header': 'image/png'}



@app.route('/decode', methods=['POST'])
def decodeFunction():
    cmd = [
            BINARY_PATH,
            '--decode',
            request.files['input'].name,
        ]
    rvalue, out = run(cmd)
    if(rvalue != 0):
        return out, 400
    return out


# app.run(port=5000, debug=True)
app.run(host='0.0.0.0', port=5000, debug=True)
