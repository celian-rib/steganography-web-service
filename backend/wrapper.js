import express from 'express'
import fileupload from 'express-fileupload';
import util from 'util';
import { exec as cp_exec } from 'child_process';

const BINARY = './steganography/main'
const PORT = 3000;
const HOST = '0.0.0.0';

const app = express();
app.use(express.urlencoded({ extended: true }));
app.use(fileupload({
	useTempFiles: true,
}));

const exec = util.promisify(cp_exec);

app.get('/', (req, res) => {
	res.send('Backend test')
})

app.post('/api/decode', async (req, res) => {
	console.log('Received decode file')

	if (!req.files?.input) {
		console.log('No file found')
		res.send('Aucun fichier donné');
		return;
	}

	await exec(`mv ${req.files.input.tempFilePath} ${req.files.input.tempFilePath}.bmp`);

	console.log('Executing command')
	const { stdout } = await exec(`${BINARY} -decode ${req.files.input.tempFilePath}.bmp`)
		.catch(e => {
			res.status(500).send(`Une erreur s\'est produite : ${e}`);
		});

	res.send(stdout);
});

app.post('/api/encode', async (req, res) => {
	console.log('Received decode file')

	if (req.body?.text == undefined) {
		console.log('No text found')
		res.send('Aucun texte donné');
		return;
	}

	if (req.files?.input == undefined) {
		console.log('No file found')
		res.send('Aucun fichier donné');
		return;
	}

	await exec(`mv ${req.files.input.tempFilePath} ${req.files.input.tempFilePath}.bmp`);

	console.log('Executing command')
	const { stdout } = await exec(`${BINARY} -encode ${req.files.input.tempFilePath}.bmp ${req.body?.text} tmp/encoded.bmp`)
		.catch(e => {
			res.status(500).send(`Une erreur s\'est produite : ${e}`);
		});

	console.log(stdout);
	res.sendFile(`${process.cwd()}/tmp/encoded.bmp`);
});

app.listen(PORT, HOST);
console.log(`Backend on http://${HOST}:${PORT}`);
