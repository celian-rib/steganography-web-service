import express from 'express'
import fileupload from 'express-fileupload';
import util from 'util';
import { exec as cp_exec } from 'child_process';

// const BINARY = './engine'
const BINARY = './binaries/alpine-steganongraphy-cli'
const PORT = 3000;
const HOST = '0.0.0.0';

const app = express()
app.use(fileupload({
	useTempFiles: true,
	tempFileDir: '/tmp/'
}));

const exec = util.promisify(cp_exec);

app.get('/', (req, res) => {
	res.send('Backend express')
})

app.post('/api/decode', (req, res) => {
	console.log("Received decode file")
	if (!req.files?.input) {
		console.log("No file found")
		res.send('Aucun fichier donné');
		return;
	}
	
	console.log("Executing command")
	exec(`${BINARY} --decode /tmp/${req.files.input.name}`)
		.then(result => {
			res.send(result);
		}).catch(e => {
			res.status(500).send(`Une erreur s\'est produite : ${e}`);
		});
});

app.listen(PORT, HOST);
console.log(`Backend on http://${HOST}:${PORT}`);
